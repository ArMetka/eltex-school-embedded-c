#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>

#include "./ipc.h"

#define FILES_MAX 8
#define FILENAME_MAX_LEN 128

void generate_line(char *buf);

int main(int argc, char **argv) {
    int files_count = 0;
    char files[FILES_MAX][FILENAME_MAX_LEN];
    int semaphores[FILES_MAX];
    char buf[LINE_MAX_LEN];

    for (int i = 1; (i < argc) && (i < FILES_MAX + 1); i++) {
        strncpy(files[files_count++], argv[i], FILENAME_MAX_LEN);
    }

    if (files_count == 0) {
        files_count = 1;
        strcpy(files[0], DEFAULT_FILENAME);
    }

    for (int i = 0; i < files_count; i++) {
        if (sem_init(ftok(files[i], 0), 1, semaphores + i) == IPC_FAIL) {
            exit(EXIT_FAILURE);
        }

        if (sem_set(semaphores[i], 0, 1) == IPC_FAIL) {
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        for (int i = 0; i < files_count; i++) {
            generate_line(buf);

            if (sem_wait(semaphores[i], 0) == IPC_FAIL) {
                exit(EXIT_FAILURE);
            }

            int fd = open(files[i], O_CREAT | O_APPEND | O_WRONLY, 0644);
            if (fd == -1) {
                perror("open");
                sem_post(semaphores[i], 0);
                exit(EXIT_FAILURE);
            }

            write(fd, buf, (strchr(buf, '\n') - buf) + 1);

            close(fd);

            if (sem_post(semaphores[i], 0) == IPC_FAIL) {
                exit(EXIT_FAILURE);
            }
        }

        sleep(1);
    }
}

void generate_line(char *buf) {
    int numbers = rand() % MAX_NUMBERS + 1;
    int offset = 0;

    for (int i = 0; i < numbers; i++) {
        offset += snprintf(buf + offset, LINE_MAX_LEN - offset - 1, "%d", rand() % 100);
        if (i != numbers - 1) {
            offset += snprintf(buf + offset, LINE_MAX_LEN - offset - 1, " ");
        }
    }
    buf[offset] = '\n';
}
