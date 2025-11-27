#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "./ipc.h"

#define FILES_MAX 8
#define FILENAME_MAX_LEN 128

void analyze_line(char *buf);

int main(int argc, char **argv) {
    int files_count = 0;
    char files[FILES_MAX][FILENAME_MAX_LEN];
    int semaphores[FILES_MAX];
    char buf[LINE_MAX_LEN];
    int offset[FILES_MAX];

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

        offset[i] = 0;
    }

    while (1) {
        for (int i = 0; i < files_count; i++) {
            if (sem_wait(semaphores[i], 0) == IPC_FAIL) {
                exit(EXIT_FAILURE);
            }

            FILE *fd = fopen(files[i], "r");
            if (!fd) {
                sem_post(semaphores[i], 0);
                continue;
            }

            fseek(fd, offset[i], SEEK_SET);

            while (fgets(buf, LINE_MAX_LEN - 1, fd)) {
                analyze_line(buf);
            }

            offset[i] = ftell(fd);

            fclose(fd);

            if (sem_post(semaphores[i], 0) == IPC_FAIL) {
                exit(EXIT_FAILURE);
            }
        }

        sleep(1);
    }
}

void analyze_line(char *buf) {
    int max = INT_MIN;
    int min = INT_MAX;
    int num;

    while (sscanf(buf, "%d", &num)) {
        if (num < min) {
            min = num;
        }

        if (num > max) {
            max = num;
        }

        while (*buf != ' ') {
            if (*buf == '\n') {
                printf("min = %d, max = %d\n", min, max);
                return;
            }
            buf++;
        }
        buf++;
    }
}
