#include <linux/limits.h>
#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>

#define BINARY_OPS_MAX 16
#define BINARY_OPS_DIR "./lib"

typedef struct {
    char name[NAME_MAX];
    double (*op)(double, double);
    void *handle;
} binary_op;

binary_op binary_ops[BINARY_OPS_MAX];
int binary_ops_count = 0;
char charp;  // char for scanf

int printMenu() {
    int result = 0;

    printf("\n----Calculator CLI----\n");

    for (int i = 0; i < binary_ops_count; i++) {
        printf("%d) %s\n", i + 1, binary_ops[i].name);
    }
    printf("0) Exit\n");

    printf("\nselect op: ");
    scanf("%d%c", &result, &charp);

    return result;
}

int getUserArg(const char *prompt, double *result, int allow_blank) {
    char buf[128];
    buf[0] = '\0';

    do {
        if ((buf[0] == '\n') && !allow_blank) {
            fputs("value is required!\n", stderr);
        } else if ((buf[0] == '\n') && allow_blank) {
            return 0;
        }

        printf("%s", prompt);
        fgets(buf, 128, stdin);
    } while ((!allow_blank && (buf[0] == '\n')) || (sscanf(buf, "%lf", result) != 1));

    return 1;
}

int loadSharedObjects() {
    struct dirent *de;
    DIR *dr;

    dr = opendir(BINARY_OPS_DIR);
    if (!dr) {
        fputs("Failed to open binary ops directory!\n", stderr);
        return 0;
    }
    
    while ((de = readdir(dr))) {
        int len = strlen(de->d_name);
        if (len >= 4 && strstr(de->d_name, ".so") == de->d_name + (len - 3)) {
            char full_path[NAME_MAX] = "";
            strcat(full_path, BINARY_OPS_DIR);
            strcat(full_path, "/");
            strcat(full_path, de->d_name);

            binary_ops[binary_ops_count].handle = dlopen(full_path, RTLD_LAZY);
            if (!binary_ops[binary_ops_count].handle) {
                fprintf(stderr, "Failed to open lib: %s\n", full_path);
                continue;
            }
            strncpy(binary_ops[binary_ops_count].name, de->d_name, len - 3);
            binary_ops[binary_ops_count].op = NULL;
            binary_ops_count++;
        }
    }

    printf("Successfully loaded %d shared objects\n", binary_ops_count);

    return 1;
}

int main() {
    double buf1, buf2;

    if (!loadSharedObjects()) {
        fputs("Failed to load shared objects\n", stderr);
        return 1;
    }

    while (1 != 0) {
        int op = printMenu();
        if (op == 0) {  // exit
            break;
        } else if (op < 0 || op > binary_ops_count) {
            fputs("Invalid input!\n", stderr);
        } else {
            if (!binary_ops[op - 1].op) {
                binary_ops[op - 1].op = dlsym(binary_ops[op - 1].handle, binary_ops[op - 1].name);
                if (!binary_ops[op - 1].op) {
                    fprintf(stderr, "Failed to load function %s from %s/%s.so!\n", binary_ops[op - 1].name, BINARY_OPS_DIR, binary_ops[op - 1].name);
                    return 1;
                }
            }
            getUserArg("arg1 = ", &buf1, 0);
            getUserArg("arg2 = ", &buf2, 0);
            printf("answer = %.2lf\n", binary_ops[op - 1].op(buf1, buf2));
        }
    }
    return 0;
}
