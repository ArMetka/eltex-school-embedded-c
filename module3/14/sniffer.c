#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define USAGE_PROMPT "usage: ./sniffer PORT"
#define BUF_SIZE 65536
#define DUMP_FILE "./dump.bin"

int main(int argc, char *argv[]) {
    char buffer[BUF_SIZE];
    int raw_sock;
    int target_port;

    int file = open(DUMP_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (file < 0) {
        perror("failed to open dump file");
        exit(EXIT_FAILURE);
    }

    if (argc < 2) {
        fprintf(stderr, "No port found in argv\n%s\n", USAGE_PROMPT);
    }
    target_port = atoi(argv[1]);

    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_sock < 0) {
        perror("failed to create socket");
        exit(EXIT_FAILURE);
    }

    printf("sniffer is running on UDP port %d\n", target_port);
    while (1) {
        struct sockaddr_in saddr;
        socklen_t saddr_size = sizeof(saddr);

        int data_size = recvfrom(raw_sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&saddr, &saddr_size);
        if (data_size < 0) {
            perror("failed to receive data");
            continue;
        }

        // IP header
        struct iphdr *iph = (struct iphdr *)buffer;
        unsigned short iphdrlen = iph->ihl * 4;

        // UDP header
        struct udphdr *udph = (struct udphdr *)(buffer + iphdrlen);

        if (ntohs(udph->source) == target_port || ntohs(udph->dest) == target_port) {
            write(file, buffer, data_size);

            unsigned char *data = buffer + iphdrlen + sizeof(struct udphdr);
            int data_length = ntohs(udph->len) - sizeof(struct udphdr);

            if (data_length > 0) {
                printf("src: %s:%d\n", inet_ntoa(*(struct in_addr *)&iph->saddr), ntohs(udph->source));
                printf("dst: %s:%d\n", inet_ntoa(*(struct in_addr *)&iph->daddr), ntohs(udph->dest));
                printf("data: ");

                for (int i = 0; i < data_length; i++) {
                    if (data[i] >= 32 && data[i] <= 126) {
                        putchar(data[i]);
                    } else {
                        putchar('.');
                    }
                }
                printf("\n");
            }
        }
    }

    close(raw_sock);
    close(file);
    return 0;
}
