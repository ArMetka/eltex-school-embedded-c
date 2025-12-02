#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024

#define DEFAULT_ADDR INADDR_ANY
#define DEFAULT_PEER_ADDR "127.0.0.1"

#define USAGE "usage: ./client LOCAL_PORT PEER_PORT [-a|--address PEER_ADDR]\n"

int main(int argc, char **argv) {
    int sockfd;
    char buf[BUF_SIZE];
    struct sockaddr_in local_addr, peer_addr;
    pid_t pid;

    bzero(&local_addr, sizeof(local_addr));
    bzero(&peer_addr, sizeof(peer_addr));

    if (argc < 3) {
        fputs(USAGE, stderr);
        exit(EXIT_FAILURE);
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(DEFAULT_ADDR);
    local_addr.sin_port = htons(atoi(argv[1]));

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(atoi(argv[2]));

    if (argc > 4 && (strcmp(argv[3], "-a") == 0 || strcmp(argv[3], "--address") == 0)) {
        if (!inet_aton(argv[4], &peer_addr.sin_addr)) {
            fputs("failed to parse peer address", stderr);
            fputs(USAGE, stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        inet_aton(DEFAULT_PEER_ADDR, &peer_addr.sin_addr);
    }

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // child (listen)
        while (1) {
            socklen_t len = sizeof(peer_addr);

            if (recvfrom(sockfd, buf, BUF_SIZE - 1, 0, (struct sockaddr *)&peer_addr, &len) < 0) {
                perror("recvfrom");
                _exit(EXIT_FAILURE);
            }

            printf("[%s:%d]: %s\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), buf);
        }
    } else {  // parent (write)
        while (1) {
            fgets(buf, BUF_SIZE - 1, stdin);
            *strchr(buf, '\n') = '\0';

            if (sendto(sockfd, buf, strlen(buf) + 1, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
                perror("sendto");
                close(sockfd);
                kill(pid, SIGINT);
                exit(EXIT_FAILURE);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
