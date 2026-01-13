#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./ipc.h"

#define BUF_SIZE 2048

void clear_and_exit(int exit_status, int send_error);
void sig_handler(int signal);

sig_atomic_t should_exit = 0;
sig_atomic_t need_to_read = 0;
char fifo_path_buf[128];
int cli_fd = -1;
pid_t pid;

int main() {
    char buf[BUF_SIZE];
    int self_fd = -1;
    int status = DRIVER_FREE;
    int task_timer = 0;
    int task_timer_left = 0;
    pid = getpid();

    struct sigaction sa;
    sa.sa_handler = sig_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0 || sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("failed to bind signal handler");
        clear_and_exit(EXIT_FAILURE, 1);
    }

    sprintf(fifo_path_buf, DRIVER_FIFO_PATH, pid);
    self_fd = open(fifo_path_buf, O_RDONLY | O_NONBLOCK);
    if (self_fd < 0) {
        perror("failed to open fifo for reading");
        clear_and_exit(EXIT_FAILURE, 1);
    }
    fcntl(self_fd, F_SETFL, 0);  // switch back to blocking read

    cli_fd = open(CLI_FIFO_PATH, O_WRONLY);
    if (cli_fd < 0) {
        perror("failed to open fifo for writing");
        clear_and_exit(EXIT_FAILURE, 1);
    }

    while (!should_exit) {
        if (status == DRIVER_FREE || need_to_read) {  // wait for cli commands
            need_to_read = 0;
            if (read(self_fd, buf, sizeof(struct driver_msg)) != sizeof(struct driver_msg)) {
                continue;
            }

            struct driver_msg *msg = (struct driver_msg *)buf;
            if (msg->msg_type == MSG_TYPE_EXIT) {
                clear_and_exit(EXIT_SUCCESS, 0);
            } else if (msg->msg_type == MSG_TYPE_STATUS) {
                struct driver_msg new_msg = {
                    .pid = pid,
                    .msg_type = MSG_TYPE_STATUS,
                    .status = status,
                    .task_timer = task_timer,
                };
                write(cli_fd, &new_msg, sizeof(struct driver_msg));
            } else if (msg->msg_type == MSG_TYPE_TASK) {
                if (status == DRIVER_BUSY) {  // already have active task
                    struct driver_msg new_msg = {
                        .pid = pid,
                        .msg_type = MSG_TYPE_TASK_FAIL,
                        .status = status,
                        .task_timer = task_timer,
                    };
                    write(cli_fd, &new_msg, sizeof(struct driver_msg));
                    need_to_read = 0;
                    continue;
                }

                status = DRIVER_BUSY;
                task_timer_left = task_timer = msg->task_timer;
                need_to_read = 0;
            }
        } else {  // do the job
            if (task_timer_left == 0) {
                status = DRIVER_FREE;
                task_timer = 0;
                struct driver_msg new_msg = {
                    .pid = pid,
                    .msg_type = MSG_TYPE_STATUS,
                    .status = status,
                    .task_timer = task_timer,
                };
                write(cli_fd, &new_msg, sizeof(struct driver_msg));
                continue;
            }

            task_timer_left = sleep(task_timer_left);
        }
    }

    clear_and_exit(EXIT_SUCCESS, 0);
}

void clear_and_exit(int exit_status, int send_error) {
    if (send_error && cli_fd > 0) {
        struct driver_msg msg = {
            .pid = pid,
            .msg_type = MSG_TYPE_ERROR,
            .status = 0,
            .task_timer = 0,
        };
        write(cli_fd, &msg, sizeof(struct driver_msg));
    }
    unlink(fifo_path_buf);
    exit(exit_status);
}

void sig_handler(int signal) {
    if (signal == SIGINT) {
        should_exit = 1;
    } else if (signal == SIGUSR1) {
        need_to_read = 1;
    }
}
