#ifndef M3_08_IPC_H
#define M3_08_IPC_H

#include <sys/ipc.h>
#include <sys/sem.h>

#define IPC_SUCCESS 1
#define IPC_FAIL 0

#define DEFAULT_FILENAME "out.txt"
#define LINE_MAX_LEN 128
#define MAX_NUMBERS 10

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int sem_init(key_t key, int sem_num, int *sem_id);
int sem_set(int sem_id, int sem_num, int val);
int sem_wait(int sem_id, int sem_num);
int sem_post(int sem_id, int sem_num);
int sem_destroy(int sem_id);

#endif