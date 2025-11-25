#include "./ipc.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>

int msq_init(int *msqid, int create) {
    key_t key = ftok(FTOK_PATH, 0);

    *msqid = msgget(key, ((create) ? (IPC_CREAT) : 0) | 0644);
    
    if (*msqid == -1) {
        perror("msgget");
        return MSQ_FAIL;
    }

    return MSQ_SUCCESS;
}

int msq_destroy(int msqid) {
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return MSQ_FAIL;
    };

    return MSQ_SUCCESS;
}

int msq_send(int msqid, long dest, long from, int type, char *payload) {
    msg_buf buf;
    buf.dest = dest;
    buf.from = from;
    buf.type = type;
    strncpy(buf.payload, payload, PAYLOAD_BUF_SIZE);

    if (msgsnd(msqid, &buf, sizeof(msg_buf) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        return MSQ_FAIL;
    }

    return MSQ_SUCCESS;
}

int msq_recv(int msqid, long dest, msg_buf *buf) {
    if (msgrcv(msqid, buf, sizeof(msg_buf) - sizeof(long), dest, 0) == -1) {
        perror("msgrcv");
        return MSQ_FAIL;
    }

    return MSQ_SUCCESS;
}
