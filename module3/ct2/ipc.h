#ifndef M3_CT2_IPC_H
#define M3_CT2_IPC_H

#define SUCCESS 1
#define FAIL 0
#define DRIVER_FREE 1
#define DRIVER_BUSY 2

#define CLI_FIFO_PATH "/tmp/fifo_taxi_cli.0"
#define DRIVER_FIFO_PATH "/tmp/fifo_taxi_cli.%d"

#define MSG_TYPE_ERROR 0
#define MSG_TYPE_STATUS 1
#define MSG_TYPE_TASK 2
#define MSG_TYPE_TASK_FAIL 3
#define MSG_TYPE_EXIT 4

struct driver_msg {
    int pid;
    int msg_type;
    int status;
    int task_timer;
};

#endif