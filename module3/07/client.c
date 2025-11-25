#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./ipc.h"

int main(int argc, char **argv) {
    int reverse = 0;
    mqd_t mqd0, mqd1;

    if (argc > 1 && (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--reverse") == 0)) {
        reverse = 1;
    }

    if (queue_open(&mqd0, &mqd1, reverse) != MQ_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    int reading = 1;  // normal -> read first
    if (reverse) {  // reverse -> write first
        reading = 0;
    }

    char buf[128];
    while (1) {
        memset(buf, '\0', 128);
        if (reading) {
            if (queue_recv(mqd0, buf, 0) != MQ_SUCCESS) {
                break;
            }
            printf("received message: %s\n", buf);
        } else {
            printf("message: ");
            fgets(buf, 127, stdin);
            char *tmp_ptr = strchr(buf, '\n');
            if (!tmp_ptr) {
                fputs("message too long", stderr);
                break;
            }
            *tmp_ptr = '\0';
            if (queue_send(mqd1, buf, 0) != MQ_SUCCESS) {
                break;
            }
        }
        reading = !reading;
    }

    queue_close(mqd0);
    queue_close(mqd1);

    if (!reverse) {
        queue_destroy();
    }

    exit(EXIT_SUCCESS);
}
