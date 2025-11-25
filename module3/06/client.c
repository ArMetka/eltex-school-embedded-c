#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "./ipc.h"

int main(int argc, char **argv) {
    int msqid;
    long client_id;

    if (argc < 2 || sscanf(argv[1], "%ld", &client_id) != 1) {
        fputs("failed to get client_id from argv\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (msq_init(&msqid, 0) == MSQ_FAIL) {
        exit(EXIT_FAILURE);
    }

    msq_send(msqid, MSG_DEST_SERVER, client_id, MSG_TYPE_CONNECT, "");

    pid_t pid;
    switch (pid = fork()) {
        case -1:
            msq_send(msqid, MSG_DEST_SERVER, client_id, MSG_TYPE_DISCONNECT, "");
            perror("fork");
            exit(EXIT_FAILURE);
        case 0: // child (listen to messages)
            while (1) {
                msg_buf buf;
                msq_recv(msqid, client_id, &buf);
                switch (buf.type) {
                    case MSG_TYPE_CONNECT:
                        printf("[(SRV)]: client %ld connected\n", buf.from);
                        break;
                    case MSG_TYPE_DISCONNECT:
                        printf("[(SRV)]: client %ld disconnected\n", buf.from);
                        break;
                    case MSG_TYPE_BROADCAST:
                        printf("[%ld -> (ALL)]: %s\n", buf.from, buf.payload);
                        break;
                    case MSG_TYPE_PRIVATE:
                        printf("[%ld -> %ld]: %s\n", buf.from, client_id, buf.payload);
                        break;
                    default:
                        fprintf(stderr, "unknown message type from %ld", buf.from);
                }
            }
        default: // parent (listen to user input)
            while (1) {
                char buf[128];
                char *buf_ptr;
                memset(buf, '\0', 128);

                fgets(buf, 127, stdin);

                if ((buf_ptr = strchr(buf, '\n'))) {
                    *buf_ptr = '\0';
                }

                if (strcmp(buf, "shutdown") == 0) {
                    break;
                }
                
                buf_ptr = strchr(buf, ':');
                if (!buf_ptr) {
                    msq_send(msqid, MSG_DEST_SERVER, client_id, MSG_TYPE_BROADCAST, buf);
                } else {
                    long dest;
                    if (sscanf(buf_ptr + 1, "%ld", &dest) == 1) {
                        int count = 0;
                        long tmp = dest;
                        while (tmp) {
                            count++;
                            tmp /= 10;
                        }
                        memset(buf_ptr, ' ', count + 1);
                        if (buf_ptr == buf) {
                            msq_send(msqid, dest, client_id, MSG_TYPE_PRIVATE, buf_ptr + count + 1);
                        } else {
                            msq_send(msqid, dest, client_id, MSG_TYPE_PRIVATE, buf);
                        }
                    } else {
                        msq_send(msqid, MSG_DEST_SERVER, client_id, MSG_TYPE_BROADCAST, buf);
                    }
                }
            }

            kill(pid, SIGINT);
    }

    msq_send(msqid, MSG_DEST_SERVER, client_id, MSG_TYPE_DISCONNECT, "");
}
