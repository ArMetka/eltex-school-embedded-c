#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_FILENAME "out.txt"
#define MAX_FILENAME_LEN 256
#define FILE_WRITE_MOD (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void sigHandler(int signal);

int fd = -1;
sig_atomic_t sigint_count = 0;

int main(int argc, char **argv) {
    char filename[MAX_FILENAME_LEN];

    if (argc > 1) {
        strncpy(filename, argv[1], MAX_FILENAME_LEN - 1);
    } else {
        strncpy(filename, DEFAULT_FILENAME, MAX_FILENAME_LEN - 1);
    }

    fd = open(filename, O_CREAT | O_WRONLY, FILE_WRITE_MOD);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = sigHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int count = 1;
    char buf[16] = "";
    while (sigint_count < 3) {
        sprintf(buf, "%d\n", count++);
        write(fd, buf, strlen(buf));
        sleep(1);
    }

    close(fd);

    return 0;
}

void sigHandler(int signal) {
    if (signal == SIGINT) {
        sigint_count++;
        if (fd != -1) {
            write(fd, "SIGINT recieved\n", 16);
        }
    } else if (signal == SIGQUIT) {
        if (fd != -1) {
            write(fd, "SIGQUIT recieved\n", 17);
        }
    }
}
