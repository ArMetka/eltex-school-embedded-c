#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SEMAPHORE_NAME "/m3_09_sem0"
#define MAX_NUMBERS 12
#define MAX_LINE_LEN 128
#define MAX_FILENAME 128

void generate_line(char *buf);
void analyze_line(char *buf);

int main(int argc, char **argv) {
    char filename[MAX_FILENAME];
    if (argc < 2 || !strncpy(filename, argv[1], MAX_FILENAME)) {
        fputs("failed to get filename from argv\n", stderr);
        exit(EXIT_FAILURE);
    }

    sem_t *sem;
    sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    srand(0);
    if (pid == 0) {  // child
        int offset = 0;
        char buf[MAX_LINE_LEN];
        while (1) {
            if (sem_wait(sem) == -1) {
                perror("sem_wait");
                exit(EXIT_FAILURE);
            }

            FILE *fd = fopen(filename, "r");
            if (!fd) {
                sem_post(sem);
                perror("open");
                exit(EXIT_FAILURE);
            }

            fseek(fd, offset, SEEK_SET);

            while (fgets(buf, MAX_LINE_LEN - 1, fd)) {
                analyze_line(buf);
            }

            offset = ftell(fd);

            fclose(fd);

            if (sem_post(sem) == -1) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }

            sleep(1);
        }
    } else {  // parent
        char buf[MAX_LINE_LEN];
        while (1) {
            generate_line(buf);

            if (sem_wait(sem) == -1) {
                perror("sem_wait");
                exit(EXIT_FAILURE);
            }

            int fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, 0644);
            if (fd == -1) {
                sem_post(sem);
                perror("open");
                exit(EXIT_FAILURE);
            }

            write(fd, buf, (strchr(buf, '\n') - buf) + 1);

            close(fd);

            if (sem_post(sem) == -1) {
                perror("sem_post");
                exit(EXIT_FAILURE);
            }

            sleep(1);
        }

        sem_close(sem);
        sem_unlink(SEMAPHORE_NAME);
    }
}

void generate_line(char *buf) {
    int numbers = rand() % MAX_NUMBERS + 1;
    int offset = 0;

    for (int i = 0; i < numbers; i++) {
        offset += snprintf(buf + offset, MAX_LINE_LEN - offset - 1, "%d", rand() % 100);
        if (i != numbers - 1) {
            offset += snprintf(buf + offset, MAX_LINE_LEN - offset - 1, " ");
        }
    }
    buf[offset] = '\n';
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
