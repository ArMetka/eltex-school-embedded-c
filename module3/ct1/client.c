#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define USAGE_PROMPT "usage: ./client PORT_CLI PORT_SRV"
#define BUF_SIZE 65536
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define DISCONNECT_MSG "exit"

struct addr_data {
    volatile in_addr_t src_addr;
    volatile in_port_t src_port;
    volatile in_addr_t dst_addr;
    volatile in_port_t dst_port;
};

sig_atomic_t should_exit = 0;
struct addr_data addr_data;
volatile int sock_in;
volatile int sock_out;

unsigned short checksum(unsigned short *ptr, int n);
void send_msg(int sock_out, struct addr_data addr_data, const char *message);
void sig_handler(int signal);

int main(int argc, char **argv) {
    char buffer[BUF_SIZE];
    int port_cli, port_srv;

    if (argc == 2) {
        fprintf(stderr, "No server port found in argv!\n%s\n", USAGE_PROMPT);
        exit(EXIT_FAILURE);
    } else if (argc == 3) {
        port_cli = atoi(argv[1]);
        port_srv = atoi(argv[2]);
    } else {
        fprintf(stderr, "Failed to parse args!\n%s\n", USAGE_PROMPT);
        exit(EXIT_FAILURE);
    }

    addr_data.src_addr = inet_addr(CLIENT_IP);
    addr_data.src_port = htons(port_cli);
    addr_data.dst_addr = inet_addr(SERVER_IP);
    addr_data.dst_port = htons(port_srv);

    sock_in = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock_in < 0) {
        perror("failed to create in socket");
        exit(EXIT_FAILURE);
    }

    sock_out = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock_out < 0) {
        perror("failed to create out socket");
        close(sock_in);
        exit(EXIT_FAILURE);
    }

    // ip header already included
    int one = 1;
    if (setsockopt(sock_out, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("failed to set socket option IP_HDRINCL");
        close(sock_in);
        close(sock_out);
        exit(EXIT_FAILURE);
    }

    // bind signal handler
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("failed to bind signal handler");
        close(sock_in);
        close(sock_out);
        exit(EXIT_FAILURE);
    }

    while (!should_exit) {
        printf("message: ");
        if (!fgets(buffer, BUF_SIZE, stdin)) {
            exit(EXIT_SUCCESS);
        }
        *strchr(buffer, '\n') = '\0';

        if (strcmp(buffer, DISCONNECT_MSG) == 0) {
            should_exit = 1;
        }

        send_msg(sock_out, addr_data, buffer);

        struct sockaddr_in saddr;
        socklen_t saddr_size = sizeof(saddr);

        struct iphdr *iph;
        struct udphdr *udph;
        unsigned short iphdrlen;
        while (!should_exit) {
            int data_size = recvfrom(sock_in, buffer, BUF_SIZE, 0, (struct sockaddr *)&saddr, &saddr_size);
            if (data_size < 0) {
                if (errno != EINTR) {  // (not) Interrupted system call (exit via ^C)
                    perror("failed to receive data");
                }
                continue;
            }

            // IP header
            iph = (struct iphdr *)buffer;
            iphdrlen = iph->ihl * 4;

            // UDP header
            udph = (struct udphdr *)(buffer + iphdrlen);

            // skip packets to other ports
            if (udph->uh_dport != htons(port_cli)) {
                continue;
            }
            break;
        }

        if (should_exit) {
            break;
        }

        // print message
        unsigned char *data = buffer + iphdrlen + sizeof(struct udphdr);
        int data_length = ntohs(udph->len) - sizeof(struct udphdr);
        printf("%s:%d: \"", inet_ntoa(*(struct in_addr *)&iph->saddr), ntohs(udph->source));
        for (int i = 0; i < data_length; i++) {
            if (data[i] >= 32 && data[i] <= 126) {
                putchar(data[i]);
            } else {
                putchar('.');
            }
        }
        printf("\"\n");
    }

    close(sock_in);
    close(sock_out);

    return 0;
}

unsigned short checksum(unsigned short *ptr, int n) {
    register unsigned long sum;

    for (sum = 0; n > 0; n--) {
        sum += *ptr++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (unsigned short)(~sum);
}

void send_msg(int sock_out, struct addr_data addr_data, const char *message) {
    char packet[BUF_SIZE];

    struct iphdr *iph = (struct iphdr *)packet;
    iph->saddr = addr_data.src_addr;
    iph->daddr = addr_data.dst_addr;
    iph->ihl = 5;  // ip header len (bytes)
    iph->version = 4;  // IPv4
    iph->tos = 0;  // type of service
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message) + 1);  // total len
    iph->id = 11111;
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;
    iph->check = checksum((unsigned short *)iph, sizeof(struct iphdr));

    struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));
    udph->source = addr_data.src_port;
    udph->dest = addr_data.dst_port;
    udph->len = htons(sizeof(struct udphdr) + strlen(message) + 1);  // udph len
    udph->check = 0;
    udph->check = checksum((unsigned short *)udph, sizeof(struct udphdr));

    char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    strncpy(data, message, BUF_SIZE - (sizeof(struct iphdr) + sizeof(struct udphdr)) - 1);

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_in));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = iph->daddr;
    dst_addr.sin_port = udph->dest;

    ssize_t sent =
        sendto(sock_out, packet, ntohs(iph->tot_len), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
    if (sent < 0) {
        perror("failed to send message");
    }
}

void sig_handler(int signal) {
    send_msg(sock_out, addr_data, DISCONNECT_MSG);
    should_exit = 1;
}
