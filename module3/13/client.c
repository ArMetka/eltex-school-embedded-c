#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define USAGE_PROMPT "usage: ./client HOSTNAME PORT"
#define BUF_SIZE 1024

#define SERVER_MSG_DISCONNECT "disconnect"
#define SERVER_MSG_DOWNLOAD "download"
#define SERVER_MSG_UPLOAD "upload"

void error(const char *msg);

int main(int argc, char *argv[]) {
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[BUF_SIZE];

    if (argc < 3) {
        fprintf(stderr, "no host or port found in argv\n%s\n", USAGE_PROMPT);
        exit(0);
    }

    portno = atoi(argv[2]);

    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) {
        error("failed to open socket");
    }

    server = gethostbyname(argv[1]);
    if (!server) {
        fprintf(stderr, "failed to resovle hostname\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("failed to connect");
    }

    while ((n = recv(my_sock, buff, sizeof(buff) - 1, 0)) > 0) {
        buff[n] = 0;
        printf("%s", buff);

        if (strcmp(buff, SERVER_MSG_DISCONNECT) == 0) {
            close(my_sock);
            return 0;
        } else if (strcmp(buff, SERVER_MSG_DOWNLOAD) == 0) {
            int filesize;
            char filepath[BUF_SIZE];
            int file;

            printf("\npath to save file: ");
            fgets(filepath, BUF_SIZE - 1, stdin);
            *strchr(filepath, '\n') = '\0';

            file = open(filepath, O_WRONLY | O_CREAT, 0666);

            write(my_sock, "1", sizeof("1"));
            recv(my_sock, buff, sizeof(buff) - 1, 0);
            sscanf(buff, "%d", &filesize);
            write(my_sock, "1", sizeof("1"));
            int recieved = 0;
            while (recieved < filesize) {
                recieved += recv(my_sock, buff, BUF_SIZE - 1, 0);
                write(file, buff, recieved);
            }

            close(file);
            continue;
        } else if (strcmp(buff, SERVER_MSG_UPLOAD) == 0) {
            int filesize;
            char filepath[BUF_SIZE];
            struct stat file_stat;
            int file;

            printf("\nfile to upload: ");
            fgets(filepath, BUF_SIZE - 1, stdin);
            *strchr(filepath, '\n') = '\0';

            file = open(filepath, O_RDONLY);

            fstat(file, &file_stat);
            filesize = file_stat.st_size;
            sprintf(buff, "%d", filesize);
            write(my_sock, buff, strlen(buff));
            read(my_sock, buff, BUF_SIZE - 1);

            off_t offset = 0;
            while (offset < filesize) {
                offset += sendfile(my_sock, file, &offset, filesize - offset);
            }

            close(file);
            continue;
        }

        fgets(buff, sizeof(buff) - 1, stdin);
        *strchr(buff, '\n') = '\0';

        send(my_sock, buff, strlen(buff), 0);
    }

    fprintf(stderr, "failed to recieve message\n");
    close(my_sock);
    return -1;
}

void error(const char *msg) {
    perror(msg);
    exit(0);
}
