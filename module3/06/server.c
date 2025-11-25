#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "./ipc.h"

#define MAX_CLIENTS 16

int msqid = -1;
int clients_count = 0;
long clients[MAX_CLIENTS];

void send_all(int msqid, long from, int type, char *payload);
void destroy_on_exit(int sig);

int main() {
    if (msq_init(&msqid, 1) == MSQ_FAIL) {
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, destroy_on_exit);

    while (1) {
        msg_buf buf;

        if (msq_recv(msqid, MSG_DEST_SERVER, &buf) == MSQ_FAIL) {
            exit(EXIT_FAILURE);
        }

        switch (buf.type) {
            case MSG_TYPE_CONNECT:
                if (clients_count == MAX_CLIENTS - 1) {
                    fputs("maximum number of clients is reached\n", stderr);
                }
                clients[clients_count++] = buf.from;
                send_all(msqid, buf.from, MSG_TYPE_CONNECT, "");
                printf("[(SRV)]: client %ld connected\n", buf.from);
                break;
            case MSG_TYPE_DISCONNECT:
                for (int i = --clients_count; i < MAX_CLIENTS - 1; i++) {
                    clients[i] = clients[i + 1];
                }
                send_all(msqid, buf.from, MSG_TYPE_CONNECT, "");
                printf("[(SRV)]: client %ld disconnected\n", buf.from);
                break;
            case MSG_TYPE_BROADCAST:
                send_all(msqid, buf.from, MSG_TYPE_BROADCAST, buf.payload);
                printf("[%ld -> (ALL)]: %s\n", buf.from, buf.payload);
                break;
            case MSG_TYPE_PRIVATE:
                printf("[%ld -> (SRV)]: %s\n", buf.from, buf.payload);
                break;
            default:
                fprintf(stderr, "unknown message type from %ld", buf.from);
        }
    }

    return 0;
}

void send_all(int msqid, long from, int type, char *payload) {
    for (int i = 0; i < clients_count; i++) {
        msq_send(msqid, clients[i], from, type, payload);
    }
}

void destroy_on_exit(int sig) {
    if (msqid != -1) {
        msq_destroy(msqid);
    }
}
