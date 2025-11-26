#include <fcntl.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#include "./ipc.h"

int fd = -1;
sem_t *sem = NULL;

int init_ipc(char *filename) {
    fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        return IPC_FAIL;
    }

    sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 1);
    if (!sem) {
        perror("sem_open");
        return IPC_FAIL;
    }
}

int write_line(char *buf);

int read_line(char *buf);

int read_lines(void (*callback)(char *));
