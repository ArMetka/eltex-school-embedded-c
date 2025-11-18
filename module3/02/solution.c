#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_LEN 2048
#define MAX_ARG_LEN 256

int parseArgs(char *args, char ***result);
void reallocArgs(int arg_cur, int arg_max, char ***args);
void freeArgs(int args_count, char **args);

/**
 * "exit" or ^D to exit
 * "?" to get last return value
 */
int main(int argc, char **argv) {
    char buf[BUF_LEN];
    char shell_prompt[128] = "";
    int args_count = 0;
    char **args;
    int rv;

    // printf("program args (%d):\n", argc);
    // for (int i = 0; i < argc; i++) {
    //     printf("%s\n", argv[i]);
    // }

    sprintf(shell_prompt, "%s> ", getlogin());

    while (1) {
        printf("%s", shell_prompt);

        buf[BUF_LEN - 1] = '\n';
        if (!fgets(buf, BUF_LEN, stdin)) {  // EOF
            printf("exit");
            break;
        }
        if (buf[BUF_LEN - 1] != '\n') {
            fputs("Arguments buffer size exceeded", stderr);
            exit(EXIT_FAILURE);
        }
        args_count = parseArgs(buf, &args);

        if (args_count == 0) {
            freeArgs(args_count, args);
            continue;
        } else if (args_count == 1 && strcmp(args[0], "exit") == 0) {
            freeArgs(args_count, args);
            break;
        } else if (args_count == 1 && strcmp(args[0], "?") == 0) {
            printf("%d\n", rv);
            freeArgs(args_count, args);
            continue;
        }

        // printf("cmd: %s\n", args[0]);
        // printf("args (%d):\n", args_count);
        // char **tmp = args;
        // while (*tmp) {
        //     printf("%s\n", *tmp++);
        // }

        pid_t pid;
        switch (pid = fork()) {
            case -1:  // error
                perror("fork");
                exit(EXIT_FAILURE);
            case 0:  // child
                execvp(args[0], args);
                perror("exec");
                exit(EXIT_FAILURE);
            default:  // parent
                wait(&rv);
                // printf("\nProcess exited with %d\n", rv);
        }
        freeArgs(args_count, args);
    }

    return 0;
}

/**
 * arguments must be separated with whitespaces
 *
 * in order to input argument containing whitespaces, argument
 * must be either in double quotes ("") or have all its
 * whitespaces escaped (\ )
 *
 * following escape sequences are supported:
 * [\][nt\'"vr0 ]
 *
 * returns number of arguments (array elements)
 *
 * result array is null-terminated
 */
int parseArgs(char *args, char ***result) {
    int arg_cur = -1;
    int arg_max = 4;

    *result = (char **)calloc(arg_max + 1, sizeof(char *));
    for (int i = 0; i < arg_max; i++) {
        (*result)[i] = (char *)calloc(MAX_ARG_LEN, sizeof(char));
    }

    int arg_start = 0;
    int is_started = 0;
    int quotes_flag = 0;
    int escape_flag = 0;
    for (int i = 0; args[i] != '\n'; i++) {
        // printf("arg=%c, as=%d, is=%d, qf=%d, ef=%d\n", args[i], arg_start, is_started, quotes_flag, escape_flag);

        if (!is_started && args[i] == ' ') {  // space between args, skip
            arg_start = i + 1;
            continue;
        } else if (!is_started) {  // new arg
            is_started = 1;
            arg_cur++;
            if (arg_cur >= arg_max) {  // out of space, realloc
                arg_max <<= 1;
                reallocArgs(arg_cur, arg_max, result);
            }
        }

        if (args[i] == ' ') {
            if (escape_flag) {  // replace backslash with space
                escape_flag = 0;
                arg_start++;
                (*result)[arg_cur][i - arg_start] = args[i];
            } else if (quotes_flag) {  // part of arg, just read
                (*result)[arg_cur][i - arg_start] = args[i];
            } else {  // end of arg
                (*result)[arg_cur][i - arg_start] = '\0';
                arg_start = i + 1;
                is_started = 0;
            }
        } else if (args[i] == '\"') {
            if (escape_flag) {  // replace backslash with quote
                escape_flag = 0;
                arg_start--;
                (*result)[arg_cur][i - arg_start] = args[i];
            } else if (quotes_flag) {  // end of arg
                quotes_flag = 0;
                (*result)[arg_cur][i - arg_start] = '\0';
                arg_start = i + 1;
                is_started = 0;
            } else {  // start of arg
                quotes_flag = 1;
                arg_start++;
            }
        } else if (args[i] == '\\') {
            if (escape_flag) {  // prev char was backslash, leave it as is and skip 1 char
                escape_flag = 0;
                arg_start++;
            } else {  // if escape sequence is correct, next char will replace this backslash
                escape_flag = 1;
                (*result)[arg_cur][i - arg_start] = args[i];
            }
        } else if (escape_flag) {
            escape_flag = 0;
            arg_start++;
            switch (args[i]) {
                case 'n':
                    (*result)[arg_cur][i - arg_start] = '\n';
                    break;
                case 't':
                    (*result)[arg_cur][i - arg_start] = '\t';
                    break;
                case '\'':
                    (*result)[arg_cur][i - arg_start] = '\'';
                    break;
                case 'v':
                    (*result)[arg_cur][i - arg_start] = '\v';
                    break;
                case 'r':
                    (*result)[arg_cur][i - arg_start] = '\r';
                    break;
                case '0':
                    (*result)[arg_cur][i - arg_start] = '\0';
                    break;
                default:  // unsupported sequence, leave backslash and read current char
                    arg_start--;
                    (*result)[arg_cur][i - arg_start] = args[i];
            }
        } else {  // just read
            (*result)[arg_cur][i - arg_start] = args[i];
        }
    }

    for (int i = arg_cur + 1; i < arg_max + 1; i++) {
        free((*result)[i]);
        (*result)[i] = NULL;
    }

    return arg_cur + 1;
}

void reallocArgs(int arg_cur, int arg_max, char ***args) {
    *args = realloc(*args, (arg_max + 1) * sizeof(char *));
    // *args = reallocarray(*args, arg_max + 1, sizeof(char*));
    for (int i = arg_cur; i < arg_max; i++) {
        (*args)[i] = (char *)calloc(MAX_ARG_LEN, sizeof(char));
    }
}

void freeArgs(int args_count, char **args) {
    for (int i = 0; i < args_count; i++) {
        free(args[i]);
    }
    free(args);
}
