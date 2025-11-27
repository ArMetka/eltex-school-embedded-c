#include "./ipc.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int sem_init(key_t key, int sem_num, int *sem_id) {
    *sem_id = semget(key, sem_num, IPC_CREAT | 0644);
    if (*sem_id == -1) {
        perror("semget");
        return IPC_FAIL;
    }

    return IPC_SUCCESS;
}

int sem_set(int sem_id, int sem_num, int val) {
    union semun arg;
    arg.val = val;

    if (semctl(sem_id, sem_num, SETVAL, arg) == -1) {
        perror("semctl");
        return IPC_FAIL;
    }

    return IPC_SUCCESS;
}

int sem_wait(int sem_id, int sem_num) {
    struct sembuf sb = {sem_num, -1, 0};

    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop");
        return IPC_FAIL;
    }

    return IPC_SUCCESS;
}

int sem_post(int sem_id, int sem_num) {
    struct sembuf sb = {sem_num, 1, 0};

    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop");
        return IPC_FAIL;
    }

    return IPC_SUCCESS;
}

int sem_destroy(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl");
        return IPC_FAIL;
    }

    return IPC_SUCCESS;
}
