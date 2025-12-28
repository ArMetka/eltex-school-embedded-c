#include <arpa/inet.h>
#include <dirent.h>
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

#define USAGE_PROMPT "usage: ./server PORT"
#define FILES_DIR "./files"
#define BUF_SIZE 1024
#define QUEUE_SIZE 5

#define USER_PROMPT_CHOOSE_FUNC \
    "\nChoose function:\n1. Add\n2. Sub\n3. Mul\n4. Div\n5. File transfer\n0. Exit\n"
#define USER_PROMPT_UNKNOWN_FUNC "Unknown function, try something else\n"
#define USER_PROMPT_ARG1 "Enter first argument: "
#define USER_PROMPT_ARG2 "Enter second argument: "
#define USER_PROMPT_DISCONNECT "disconnect"
#define USER_PROMPT_SERVER_FILES "Server files:\n"
#define USER_PROMPT_CHOOSE_FILE_FUNK "Choose action:\n1. Download\n2. Upload\n0. Exit\n"
#define USER_PROMPT_CHOOSE_FILE_DOWNLOAD "File to download: "
#define USER_PROMPT_CHOOSE_FILE_UPLOAD "File name (server): "
#define USER_PROMPT_FILE_DOWNLOAD "download"
#define USER_PROMPT_FILE_UPLOAD "upload"

int nclients = 0;

void handle_connection(int);
int get_args(int sock, float *arg1, float *arg2);
void handle_files(int sock);

void error(const char *msg);
void printusers();
int list_files(const char *dir_name, char *out);

float add(float arg1, float arg2);
float subtract(float arg1, float arg2);
float multiply(float arg1, float arg2);
float divide(float arg1, float arg2);

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    int portno;
    int pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "no port found in argv\n%s\n", USAGE_PROMPT);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("failed to open socket");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("failed to bind socket");
    }

    listen(sockfd, QUEUE_SIZE);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            error("failed to accept connection");
        }
        nclients++;

        printf("new connection from %s\n", (char *)inet_ntoa(cli_addr.sin_addr));
        printusers();

        pid = fork();
        if (pid < 0) {
            error("failed to fork");
        } else if (pid == 0) {  // child
            close(sockfd);
            handle_connection(newsockfd);
            nclients--;
            exit(0);
        } else {  // parent
            close(newsockfd);
        }
    }

    close(sockfd);
    return 0;
}

void handle_connection(int sock) {
    int bytes_recv;
    float arg1, arg2;
    char buff[BUF_SIZE];

    while (1) {
        write(sock, USER_PROMPT_CHOOSE_FUNC, sizeof(USER_PROMPT_CHOOSE_FUNC));
        bytes_recv = read(sock, buff, BUF_SIZE - 1);
        if (bytes_recv < 0) {
            error("failed to read from socket");
        }
        buff[bytes_recv] = 0;

        switch (atoi(buff)) {
            case 1:  // add
                if (!get_args(sock, &arg1, &arg2)) {
                    return;
                }
                sprintf(buff, "%f\n", add(arg1, arg2));
                write(sock, buff, strlen(buff));
                break;
            case 2:  // sub
                if (!get_args(sock, &arg1, &arg2)) {
                    return;
                }
                sprintf(buff, "%f\n", subtract(arg1, arg2));
                write(sock, buff, strlen(buff));
                break;
            case 3:  // mul
                if (!get_args(sock, &arg1, &arg2)) {
                    return;
                }
                sprintf(buff, "%f\n", multiply(arg1, arg2));
                write(sock, buff, strlen(buff));
                break;
            case 4:  // div
                if (!get_args(sock, &arg1, &arg2)) {
                    return;
                }
                sprintf(buff, "%f\n", divide(arg1, arg2));
                write(sock, buff, strlen(buff));
                break;
            case 5:  // file transfer
                bytes_recv = sprintf(buff, "%s", USER_PROMPT_SERVER_FILES);
                bytes_recv += list_files(FILES_DIR, buff + bytes_recv);
                sprintf(buff + bytes_recv, "%s", USER_PROMPT_CHOOSE_FILE_FUNK);
                write(sock, buff, strlen(buff));
                handle_files(sock);
                break;
            case 0:
                write(sock, USER_PROMPT_DISCONNECT, sizeof(USER_PROMPT_DISCONNECT));
                return;
            default:
                write(sock, USER_PROMPT_UNKNOWN_FUNC, sizeof(USER_PROMPT_UNKNOWN_FUNC));
                break;
        }
    }
}

int get_args(int sock, float *arg1, float *arg2) {
    int bytes_recv;
    char buff[BUF_SIZE];

    write(sock, USER_PROMPT_ARG1, sizeof(USER_PROMPT_ARG1));
    bytes_recv = read(sock, buff, BUF_SIZE - 1);
    if (bytes_recv < 0) {
        error("failed to read from socket");
    }
    if (sscanf(buff, "%f", arg1) != 1) {
        return 0;
    }

    write(sock, USER_PROMPT_ARG2, sizeof(USER_PROMPT_ARG2));
    bytes_recv = read(sock, buff, BUF_SIZE - 1);
    if (bytes_recv < 0) {
        error("failed to read from socket");
    }
    if (sscanf(buff, "%f", arg2) != 1) {
        return 0;
    }

    return 1;
}

void printusers() {
    printf("%d user(s) online\n", nclients);
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int list_files(const char *dir_name, char *out) {
    DIR *dir;
    struct dirent *ent;
    int bytes_written = 0;

    if ((dir = opendir(dir_name))) {
        while ((ent = readdir(dir))) {
            if (ent->d_name[0] == '.') {  // skip ".", ".." and hidden files
                continue;
            }
            bytes_written += sprintf(out + bytes_written, "\t%s\n", ent->d_name);
        }
    }

    return bytes_written;
}

void handle_files(int sock) {
    int bytes_recv;
    char buff[BUF_SIZE];
    char filename[BUF_SIZE];
    int opt;
    int file;
    int filesize;
    off_t offset = 0;
    struct stat file_stat;

    bytes_recv = read(sock, buff, BUF_SIZE - 1);
    if (bytes_recv < 0) {
        error("failed to read from socket");
    }
    if (sscanf(buff, "%d", &opt) != 1) {
        return;
    }

    switch (opt) {
        case 1:  // client downloading
            write(sock, USER_PROMPT_CHOOSE_FILE_DOWNLOAD, sizeof(USER_PROMPT_CHOOSE_FILE_DOWNLOAD));
            bytes_recv = read(sock, buff, BUF_SIZE - 1);
            if (bytes_recv < 0) {
                error("failed to read from socket");
            }
            snprintf(filename, BUF_SIZE - 1, "%s/%s", FILES_DIR, buff);
            file = open(filename, O_RDONLY);
            if (file <= 0) {
                return;
            }

            write(sock, USER_PROMPT_FILE_DOWNLOAD, sizeof(USER_PROMPT_FILE_DOWNLOAD));

            read(sock, buff, BUF_SIZE - 1);
            fstat(file, &file_stat);
            filesize = file_stat.st_size;
            sprintf(buff, "%d", filesize);
            write(sock, buff, strlen(buff));
            read(sock, buff, BUF_SIZE - 1);

            while (offset < filesize) {
                offset += sendfile(sock, file, &offset, filesize - offset);
            }

            close(file);
            break;
        case 2:  // client uploading
            write(sock, USER_PROMPT_CHOOSE_FILE_UPLOAD, sizeof(USER_PROMPT_CHOOSE_FILE_UPLOAD));
            bytes_recv = read(sock, buff, BUF_SIZE - 1);
            if (bytes_recv < 0) {
                error("failed to read from socket");
            }
            snprintf(filename, BUF_SIZE - 1, "%s/%s", FILES_DIR, buff);

            file = open(filename, O_WRONLY | O_CREAT, 0666);

            write(sock, USER_PROMPT_FILE_UPLOAD, sizeof(USER_PROMPT_FILE_UPLOAD));

            recv(sock, buff, sizeof(buff) - 1, 0);
            sscanf(buff, "%d", &filesize);
            write(sock, "1", sizeof("1"));
            int recieved = 0;
            while (recieved < filesize) {
                recieved += recv(sock, buff, BUF_SIZE - 1, 0);
                write(file, buff, recieved);
            }

            close(file);
            break;
        default:
            return;
    }
}

float add(float arg1, float arg2) {
    return arg1 + arg2;
}

float subtract(float arg1, float arg2) {
    return arg1 - arg2;
}

float multiply(float arg1, float arg2) {
    return arg1 * arg2;
}

float divide(float arg1, float arg2) {
    return arg1 / arg2;
}
