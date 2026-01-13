#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./ipc.h"

#define BUF_SIZE 2048
#define DRIVERS_MAX 128
#define TIMEOUT_S 1

#define DRIVER_EXEC_PATH "./driver"
#define ARG_DELIMS " "
#define ERROR_PROMPT "Invalid command, use \"help\" for help\n"

struct driver {
    int fd;
    pid_t pid;
    int status;
    int task_timer;
};

int create_driver();
int send_task(pid_t pid, int task_timer);
int get_status(pid_t pid);
int get_drivers();
void handle_msg(struct driver_msg *msg);

void clear_and_exit(int exit_status);
void sig_handler(int signal);

sig_atomic_t should_exit = 0;
struct driver drivers[DRIVERS_MAX];
struct pollfd poll_fds[2];
int driver_count = 0;
int nfds = 0;
int fifo_fd = -1;

int main() {
    char buf[BUF_SIZE];

    poll_fds[0].fd = 0;  // stdin
    poll_fds[0].events = POLLIN;  // data to read
    nfds++;

    struct sigaction sa;
    sa.sa_handler = sig_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("failed to bind signal handler");
        clear_and_exit(EXIT_FAILURE);
    }

    unlink(CLI_FIFO_PATH);
    if (mkfifo(CLI_FIFO_PATH, 0644) < 0) {
        perror("failed to create fifo");
        clear_and_exit(EXIT_FAILURE);
    }

    if ((fifo_fd = open(CLI_FIFO_PATH, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("failed to open fifo for reading");
        clear_and_exit(EXIT_FAILURE);
    }
    fcntl(fifo_fd, F_SETFL, 0);  // switch back to blocking read

    poll_fds[1].fd = fifo_fd;
    poll_fds[1].events = POLLIN;
    nfds++;

    while (!should_exit) {
        int ret = poll(poll_fds, nfds, TIMEOUT_S);
        if (!ret) {  // timeout
            continue;
        } else if (ret < 0) {  // error
            if (should_exit) {
                continue;
            }
            perror("failed to poll");
            clear_and_exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++) {
            if (!(poll_fds[i].revents & POLLIN)) {
                continue;
            }
            poll_fds[i].revents = 0;

            if (poll_fds[i].fd == 0) {  // stdin
                if (!fgets(buf, BUF_SIZE - 1, stdin)) {
                    perror("failed to get input from stdin");
                    continue;
                }
                *strchr(buf, '\n') = 0;

                if (buf[0] == 0) {  // empty command
                    continue;
                }

                char *tok = strtok(buf, ARG_DELIMS);
                if (!tok) {
                    fputs("invalid input\n", stderr);
                    continue;
                } else if (strcmp(tok, "help") == 0) {
                    fputs("create_driver - creates a new driver\n", stdout);
                    fputs("send_task <pid> <task_timer> - send a new task to an existing driver\n", stdout);
                    fputs("get_status <pid> - get status of the driver (either available or busy)\n", stdout);
                    fputs("get_drivers - get all existing drivers\n", stdout);
                    fputs("help - get this text\n", stdout);
                } else if (strcmp(tok, "create_driver") == 0) {
                    if (create_driver() == FAIL) {
                        fputs("failed to create driver\n", stderr);
                        continue;
                    }
                } else if (strcmp(tok, "send_task") == 0) {
                    int pid;
                    tok = strtok(NULL, ARG_DELIMS);
                    if (!tok || (sscanf(tok, "%d", &pid) != 1)) {
                        fputs(ERROR_PROMPT, stderr);
                        continue;
                    }

                    int task_timer;
                    tok = strtok(NULL, ARG_DELIMS);
                    if (!tok || (sscanf(tok, "%d", &task_timer) != 1)) {
                        fputs(ERROR_PROMPT, stderr);
                        continue;
                    }

                    if (send_task(pid, task_timer) == FAIL) {
                        fputs("failed to get status\n", stderr);
                        continue;
                    }
                } else if (strcmp(tok, "get_status") == 0) {
                    int pid;
                    tok = strtok(NULL, ARG_DELIMS);
                    if (!tok || (sscanf(tok, "%d", &pid) != 1)) {
                        fputs(ERROR_PROMPT, stderr);
                        continue;
                    }

                    if (get_status(pid) == FAIL) {
                        fputs("failed to get status\n", stderr);
                        continue;
                    }
                } else if (strcmp(tok, "get_drivers") == 0) {
                    if (get_drivers() == FAIL) {
                        fputs("failed to get drivers\n", stderr);
                        continue;
                    }
                } else {
                    fputs(ERROR_PROMPT, stderr);
                }
            } else if (poll_fds[i].fd == fifo_fd) {  // fifo message
                if (read(fifo_fd, buf, sizeof(struct driver_msg)) != sizeof(struct driver_msg)) {
                    perror("failed to read from pipe");
                    continue;
                }

                handle_msg((struct driver_msg *)buf);
            }
        }
    }

    clear_and_exit(EXIT_SUCCESS);
    return 0;
}

int create_driver() {
    if (driver_count >= DRIVERS_MAX - 1) {
        fputs("maximum number of drivers reached\n", stderr);
        return FAIL;
    }

    struct driver *new_driver = &drivers[driver_count++];
    new_driver->status = DRIVER_FREE;
    new_driver->task_timer = 0;

    new_driver->pid = fork();
    if (new_driver->pid < 0) {  // error
        perror("failed to fork");
        driver_count--;
        return FAIL;
    } else if (new_driver->pid == 0) {  // child
        if (execl(DRIVER_EXEC_PATH, DRIVER_EXEC_PATH, NULL) < 0) {
            perror("failed to exec");
            _exit(EXIT_FAILURE);
        }
    } else {  // parent
        char fifo_path_buf[BUF_SIZE];
        sprintf(fifo_path_buf, DRIVER_FIFO_PATH, new_driver->pid);
        mkfifo(fifo_path_buf, 0644);
        new_driver->fd = open(fifo_path_buf, O_WRONLY);
        if (new_driver->fd < 0) {
            perror("failed to open fifo for writing");
            kill(new_driver->pid, SIGINT);
            driver_count--;
            return FAIL;
        }
    }

    return SUCCESS;
}

int send_task(pid_t pid, int task_timer) {
    struct driver *driver = NULL;

    for (int i = 0; i < driver_count; i++) {
        if (drivers[i].pid == pid) {
            driver = &drivers[i];
            break;
        }
    }
    if (!driver) {
        fputs("driver not found!\n", stderr);
        return FAIL;
    }

    struct driver_msg msg = {
        .pid = pid,
        .msg_type = MSG_TYPE_TASK,
        .status = 0,
        .task_timer = task_timer,
    };
    if (write(driver->fd, &msg, sizeof(struct driver_msg)) <= 0) {
        perror("failed to send message");
        return FAIL;
    }

    driver->status = DRIVER_BUSY;
    driver->task_timer = task_timer;
    kill(driver->pid, SIGUSR1);  // force driver to read

    return SUCCESS;
}

int get_status(pid_t pid) {
    struct driver *driver = NULL;

    for (int i = 0; i < driver_count; i++) {
        if (drivers[i].pid == pid) {
            driver = &drivers[i];
            break;
        }
    }
    if (!driver) {
        fputs("driver not found!\n", stderr);
        return FAIL;
    }

    if (driver->status == DRIVER_FREE) {
        printf("Available\n");
    } else {
        printf("Busy %d\n", driver->task_timer);
    }

    return SUCCESS;
}

int get_drivers() {
    printf("Drivers active - %d\n", driver_count);
    for (int i = 0; i < driver_count; i++) {
        printf("\tDriver %d - ", drivers[i].pid);
        if (drivers[i].status == DRIVER_FREE) {
            printf("Available\n");
        } else {
            printf("Busy %d\n", drivers[i].task_timer);
        }
    }

    return SUCCESS;
}

void handle_msg(struct driver_msg *msg) {
    if (msg->msg_type == MSG_TYPE_STATUS) {  // update driver status
        for (int i = 0; i < driver_count; i++) {
            if (drivers[i].pid == msg->pid) {
                drivers[i].status = msg->status;
                drivers[i].task_timer = msg->task_timer;
                break;
            }
        }
    } else if (msg->msg_type == MSG_TYPE_ERROR) {  // driver error
        for (int i = 0; i < driver_count; i++) {
            if (drivers[i].pid == msg->pid) {
                close(drivers[i].fd);
                for (int j = i; j < driver_count - 1; j++) {
                    drivers[j] = drivers[j + 1];
                }
                break;
            }
        }
        driver_count--;
    } else if (msg->msg_type == MSG_TYPE_TASK_FAIL) {  // scheduled task to a busy driver
        fprintf(stderr, "failed to schedule task to a driver %d - driver is busy\n", msg->pid);
        for (int i = 0; i < driver_count; i++) {
            if (drivers[i].pid == msg->pid) {
                drivers[i].status = msg->status;
                drivers[i].task_timer = msg->task_timer;
                break;
            }
        }
    }
}

void clear_and_exit(int exit_status) {
    for (int i = 0; i < nfds; i++) {
        close(poll_fds[i].fd);
    }
    for (int i = 0; i < driver_count; i++) {
        kill(drivers[i].pid, SIGINT);
    }
    unlink(CLI_FIFO_PATH);
    exit(exit_status);
}

void sig_handler(int signal) {
    should_exit = 1;
}
