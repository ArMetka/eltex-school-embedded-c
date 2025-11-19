#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_LEN 2048
#define MAX_ARG_LEN 256

#define FILE_WRITE_MOD (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int parseArgs(char *args, char ***result);
void reallocArgs(int arg_cur, int arg_max, char ***args);
void freeArgs(char **args);

/**
 * "exit" or ^D to exit
 * "?" to get last return value
 *
 * pipes & input redirection (bash variant implemented):
 *  case 1: ls -l
 *  case 2: ls -l | head -n 3
 *  case 3: head -n 3 > test1.txt | tail -n 1 < test2.txt
 *  1) stdin --> ls -l --> stdout
 *  2) stdin --> ls -l --> head -n 3 --> stdout
 *  3-zsh) head -n 3 --> test1.txt -(+test2.txt)> tail -n 1 --> stdout
 *  3-bash) head -n 3 --> test1.txt -X> test2.txt --> tail -n 1 --> stdout
 */
int main(int argc, char **argv) {
    char buf[BUF_LEN];
    char shell_prompt[128] = "";
    int fork_max = 4;
    char ***args;
    pid_t pid;
    int rv = 0;

    if (argc > 1) {  // run from argv (non-interactive mode)
        execvp(argv[1], &(argv[1]));
        perror("exec");
        exit(EXIT_FAILURE);
    }

    sprintf(shell_prompt, "%s> ", getlogin());

    while (1) {
        printf("%s", shell_prompt);

        buf[BUF_LEN - 1] = '\n';
        if (!fgets(buf, BUF_LEN, stdin)) {  // EOF
            printf("exit\n");
            break;
        }
        if (buf[BUF_LEN - 1] != '\n') {
            fputs("Arguments buffer size exceeded", stderr);
            exit(EXIT_FAILURE);
        }

        int fork_count = 0;
        int offset = 0;
        int offset_max = strchr(buf, '\n') - buf;
        args = (char ***)calloc(fork_max, sizeof(char **));
        while ((offset += parseArgs(buf + offset, &(args[fork_count])))) {
            if (++fork_count >= fork_max) {
                fork_max <<= 1;
                args = (char ***)realloc(args, fork_max * sizeof(char **));
            }
            if (++offset >= offset_max) {
                break;
            }
        }

        // printf("fork_count = %d\n", fork_count);
        // for (int i = 0; i < fork_count; i++) {
        //     printf("args[%d]:\n", i);
        //     char **args_ptr = args[i];
        //     while (*args_ptr) {
        //         printf("\t%s\n", *args_ptr);
        //         args_ptr++;
        //     }
        // }

        int prev_read_pipe_fd = -1;
        for (int i = 0; i < fork_count; i++) {
            int pipe_fd[2];
            if (i != fork_count - 1) {  // if not last
                if (pipe(pipe_fd)) {  // create pipe between current and next process
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }

            pid = fork();
            if (pid == -1) {  // error
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {  // child
                if (prev_read_pipe_fd != -1) {  // if not first, read from pipe
                    dup2(prev_read_pipe_fd, 0);  // stdin -> read from pipe
                }
                if (i != fork_count - 1) {  // if not last, write to pipe
                    dup2(pipe_fd[1], 1);  // stdout -> write to pipe
                }
                close(pipe_fd[0]);  // close read pipe for next process

                int args_count = 0;
                char **args_ptr = args[i];
                while (*args_ptr) {
                    args_ptr++;
                    args_count++;
                }

                if (args_count >= 3 && args[i][args_count - 2][0] == '>') {  // write to file
                    int fd = -1;
                    if (args[i][args_count - 2][1] == '>') {  // append, no create (>>)
                        fd = open(args[i][args_count - 1], O_APPEND | O_WRONLY);
                    } else {  // override / create (>)
                        fd = open(args[i][args_count - 1], O_CREAT | O_WRONLY, FILE_WRITE_MOD);
                    }
                    if (fd == -1) {
                        perror("open");
                        _exit(EXIT_FAILURE);
                    }
                    dup2(fd, 1);  // override stdout
                    free(args[i][args_count - 2]);
                    free(args[i][args_count - 1]);
                    args[i][args_count - 2] = NULL;
                    args[i][args_count - 1] = NULL;
                } else if (args_count >= 3 && args[i][args_count - 2][0] == '<') {  // read file
                    int fd = open(args[i][args_count - 1], O_RDONLY);
                    if (fd == -1) {
                        perror("open");
                        _exit(EXIT_FAILURE);
                    }
                    dup2(fd, 0);  // override stdin
                    free(args[i][args_count - 2]);
                    free(args[i][args_count - 1]);
                    args[i][args_count - 2] = NULL;
                    args[i][args_count - 1] = NULL;
                }

                if (strcmp(args[i][0], "?") == 0) {
                    printf("%d\n", rv);
                    _exit(EXIT_SUCCESS);
                } else if (strcmp(args[i][0], "exit") == 0) {
                    _exit(EXIT_SUCCESS);
                }

                execvp(args[i][0], args[i]);
                perror("exec");
                exit(EXIT_FAILURE);
            } else {  // parent
                if (prev_read_pipe_fd != -1) {  // close read pipe for pre-last process
                    close(prev_read_pipe_fd);
                }
                prev_read_pipe_fd = pipe_fd[0];
                close(pipe_fd[1]);  // close write pipe for last process
                wait(&rv);
                // printf("\nProcess exited with %d\n", rv);
            }
        }

        for (int i = 0; i < fork_count; i++) {
            freeArgs(args[i]);
        }
        free(args);
    }

    return 0;
}

/**
 * parse args string to array of args, pipe ("|") is treated
 * as end of args string
 *
 * arguments must be separated with whitespaces
 *
 * in order to input argument containing whitespaces, argument
 * must be either in double quotes ("") or have all its
 * whitespaces escaped (\ )
 *
 * following escape sequences are supported:
 * [\][nt\'"vr0 ]
 *
 * returns offset to last processed char in args (either "|" or "\n")
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
    int offset = 0;
    for (int i = 0; args[i] != '\n' && args[i] != '|'; i++) {
        offset = i + 1;
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

    return offset;
}

void reallocArgs(int arg_cur, int arg_max, char ***args) {
    *args = realloc(*args, (arg_max + 1) * sizeof(char *));
    // *args = reallocarray(*args, arg_max + 1, sizeof(char*));
    for (int i = arg_cur; i < arg_max; i++) {
        (*args)[i] = (char *)calloc(MAX_ARG_LEN, sizeof(char));
    }
}

void freeArgs(char **args) {
    if (!args) {
        return;
    }

    char **args_ptr = args;
    while (*args_ptr) {
        free(*args_ptr);
        args_ptr++;
    }
    free(args);
}
