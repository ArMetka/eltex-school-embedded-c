#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SEM_0_NAME "/m3_11_sem0"
#define SEM_1_NAME "/m3_11_sem1"
#define FTOK_PATH "/m3_11_shm0"

#define SHARED_MEM_SIZE 4096
#define MAX_NUMBERS 16

pid_t pid;
sem_t *sem0, *sem1;
int shm_id;
int processed_count = 0;

void generate_line(char *buf);
void analyze_line(char *buf);
void print_line(char *buf);
void sigint_handler(int sig);

int main() {
    void *shm;
    key_t key;
    srand(0);

    key = ftok(FTOK_PATH, 0);

    sem0 = sem_open(SEM_0_NAME, O_CREAT, 0644, 0);
    if (sem0 == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem1 = sem_open(SEM_1_NAME, O_CREAT, 0644, 0);
    if (sem1 == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(key, SHARED_MEM_SIZE, IPC_CREAT | 0644);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shm = shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sigint_handler);
    if (pid == 0) {  // child
        while (1) {
            if (sem_wait(sem0) == -1) {
                perror("sem_wait");
                _exit(EXIT_FAILURE);
            }

            analyze_line(shm);

            sem_post(sem1);

            sleep(1);
        }
    } else {  // parent
        while (1) {
            generate_line(shm);

            sem_post(sem0);

            if (sem_wait(sem1) == -1) {
                perror("sem_wait");
                exit(EXIT_FAILURE);
            }

            print_line(shm);
            processed_count++;

            sleep(1);
        }
    }

    if (pid != 0) {
        sem_close(sem0);
        sem_close(sem1);
        sem_unlink(SEM_0_NAME);
        sem_unlink(SEM_1_NAME);
        shmdt(shm);
        shmctl(shm_id, IPC_RMID, NULL);
    }
}

void generate_line(char *buf) {
    int numbers = rand() % MAX_NUMBERS + 1;
    int offset = 0;

    for (int i = 0; i < numbers; i++) {
        int rand_num = rand() % 100;
        // printf("%d ", rand_num);
        offset += snprintf(buf + offset, SHARED_MEM_SIZE - offset - 1, "%d", rand_num);
        if (i != numbers - 1) {
            offset += snprintf(buf + offset, SHARED_MEM_SIZE - offset - 1, " ");
        }
    }
    // printf("\n");
    buf[offset] = '\n';
}

void analyze_line(char *buf) {
    int max = INT_MIN;
    int min = INT_MAX;
    int num;

    char *buf_ptr = buf;
    while (sscanf(buf_ptr, "%d", &num)) {
        if (num < min) {
            min = num;
        }

        if (num > max) {
            max = num;
        }

        while (*buf_ptr != ' ') {
            if (*buf_ptr == '\n') {
                sprintf(buf, "%d %d\n", min, max);
                return;
            }
            buf_ptr++;
        }
        buf_ptr++;
    }
}

void print_line(char *buf) {
    int min, max;

    sscanf(buf, "%d %d", &min, &max);
    printf("min = %d, max = %d\n", min, max);
}

void sigint_handler(int sig) {
    if (pid == 0) {
        sem_post(sem1);
        _exit(EXIT_SUCCESS);
    } else {
        printf("total processed: %d\n", processed_count);

        sem_close(sem0);
        sem_close(sem1);
        sem_unlink(SEM_0_NAME);
        sem_unlink(SEM_1_NAME);
        shmctl(shm_id, IPC_RMID, NULL);

        exit(EXIT_SUCCESS);
    }
}
