#include <stdio.h>

#include "./calc.h"

#define BINARY_OPS_NUM 4
binary_op binary_ops[BINARY_OPS_NUM] = {
    {
        "addition",
        add
    },
    {
        "multiplication",
        multiply},
    {
        "subtraction",
        subtract
    },
    {
        "division",
        divide
    }
};

char charp;  // char for scanf

int printMenu() {
    int result = 0;

    printf("\n----Calculator CLI----\n");

    for (int i = 0; i < BINARY_OPS_NUM; i++) {
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
            printf("value is required!\n");
        } else if ((buf[0] == '\n') && allow_blank) {
            return 0;
        }

        printf("%s", prompt);
        fgets(buf, 128, stdin);
    } while ((!allow_blank && (buf[0] == '\n')) || (sscanf(buf, "%lf", result) != 1));

    return 1;
}

int main() {
    double buf1, buf2;

    while (1 != 0) {
        int op = printMenu();
        if (op == 0) {  // exit
            break;
        } else if (op < 0 || op > BINARY_OPS_NUM) {
            printf("Invalid input!\n");
        } else {
            getUserArg("arg1 = ", &buf1, 0);
            getUserArg("arg2 = ", &buf2, 0);
            printf("answer = %.2lf\n", binary_ops[op - 1].op(buf1, buf2));
        }
    }
    return 0;
}
