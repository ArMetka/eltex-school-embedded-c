#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void processArg(const char *arg, FILE *out);

int main(int argc, char **argv) {
    pid_t pid;
    int child_rv;

    if (argc <= 1) {
        return 0;
    }

    printf("Main PID=%d\n", getpid());

    switch (pid = fork()) {
        case -1:  // error
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:  // child
            for (int i = 0; i < argc / 2; i++) {
                processArg(argv[i], stdout);
            }
            _exit(EXIT_SUCCESS);
        default:  // parent
            for (int i = argc / 2; i < argc; i++) {
                processArg(argv[i], stdout);
            }
            wait(&child_rv);
            printf("Child returned with %d\n", child_rv);
            exit(EXIT_SUCCESS);
    }

    return 0;
}

void processArg(const char *arg, FILE *out) {
    long long int input_int64;
    double input_double;
    char end = '\0';

    fprintf(out, "PID=%d; ", getpid());
    if (sscanf(arg, "%lld%c", &input_int64, &end) == 1) {  // integer
        fprintf(out, "%lld %lld\n", input_int64, input_int64 * 2);
    } else if (sscanf(arg, "%lf%c", &input_double, &end) == 1) {  // float
        fprintf(out, "%.2lf %.2lf\n", input_double, input_double * 2);
    } else {  // string
        fprintf(out, "%s\n", arg);
    }
}
