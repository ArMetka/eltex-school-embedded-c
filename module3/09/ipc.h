#ifndef M3_09_IPC_H
#define M3_09_IPC_H

#define IPC_SUCCESS 1;
#define IPC_FAIL 0;

#define SEMAPHORE_NAME "m3_09_sem0"
#define MAX_LINE_LEN 128

int init_ipc(char *filename);
int write_line(char *buf);
int read_line(char *buf);

#endif