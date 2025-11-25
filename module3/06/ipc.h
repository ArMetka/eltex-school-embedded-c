#ifndef M3_06_IPC_H
#define M3_06_IPC_H

#define MSQ_SUCCESS 1
#define MSQ_FAIL 0

#define FTOK_PATH "./ipc.h"

#define MSG_TYPE_CONNECT 1
#define MSG_TYPE_DISCONNECT 2
#define MSG_TYPE_BROADCAST 3
#define MSG_TYPE_PRIVATE 4

#define MSG_DEST_SERVER 10

#define PAYLOAD_BUF_SIZE 128

typedef struct {
    long dest;
    long from;
    int type;
    char payload[PAYLOAD_BUF_SIZE];
} msg_buf;

int msq_init(int *msqid, int create);
int msq_destroy(int msqid);
int msq_send(int msqid, long dest, long from, int type, char *payload);
int msq_recv(int msqid, long dest, msg_buf *msg_buf);

#endif