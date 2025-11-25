#ifndef M3_07_IPC_H
#define M3_07_IPC_H

#include <mqueue.h>

#define MQ_SUCCESS 1
#define MQ_FAIL 0

#define QUEUE_READ "/m3_07_mq0"
#define QUEUE_WRITE "/m3_07_mq1"

#define MSG_MAX_SIZE 128

int queue_open(mqd_t *mqd0, mqd_t *mqd1, int reverse);
int queue_recv(mqd_t mqd, char *msg, unsigned int *prio);
int queue_send(mqd_t mqd, char *msg, unsigned int prio);
int queue_close(mqd_t mqd);
int queue_destroy();

#endif