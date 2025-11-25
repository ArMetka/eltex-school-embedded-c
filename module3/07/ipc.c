#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include "./ipc.h"

int queue_open(mqd_t *mqd0, mqd_t *mqd1, int reverse) {
    struct mq_attr queue_attr;
    queue_attr.mq_curmsgs = 0;
    queue_attr.mq_flags = 0;
    queue_attr.mq_maxmsg = 8;
    queue_attr.mq_msgsize = MSG_MAX_SIZE;

    if (!reverse) {
        *mqd0 = mq_open(QUEUE_READ, O_CREAT | O_RDONLY, 0644, &queue_attr);
    } else {
        *mqd0 = mq_open(QUEUE_WRITE, O_CREAT | O_RDONLY, 0644, &queue_attr);
    }
    if (*mqd0 == (mqd_t) -1) {
        perror("mqopen");
        return MQ_FAIL;
    }
    
    if (!reverse) {
        *mqd1 = mq_open(QUEUE_WRITE, O_CREAT | O_WRONLY, 0644, &queue_attr);
    } else {
        *mqd1 = mq_open(QUEUE_READ, O_CREAT | O_WRONLY, 0644, &queue_attr);
    }
    if (*mqd1 == (mqd_t) -1) {
        perror("mqopen");
        return MQ_FAIL;
    }

    return MQ_SUCCESS;
}

int queue_recv(mqd_t mqd, char *msg, unsigned int *prio) {
    if (mq_receive(mqd, msg, MSG_MAX_SIZE, prio) == -1) {
        perror("mq_receive");
        return MQ_FAIL;
    }

    return MQ_SUCCESS;
}

int queue_send(mqd_t mqd, char *msg, unsigned int prio) {
    if (mq_send(mqd, msg, strlen(msg), prio) == -1) {
        perror("mq_send");
        return MQ_FAIL;
    }

    return MQ_SUCCESS;
}

int queue_close(mqd_t mqd) {
    if (mq_close(mqd) == -1) {
        perror("mq_close");
        return MQ_FAIL;
    }

    return MQ_SUCCESS;
}

int queue_destroy() {
    if (mq_unlink(QUEUE_READ) == -1) {
        perror("mq_unlink");
        return MQ_FAIL;
    }

    if (mq_unlink(QUEUE_WRITE) == -1) {
        perror("mq_unlink");
        return MQ_FAIL;
    }

    return MQ_SUCCESS;
}
