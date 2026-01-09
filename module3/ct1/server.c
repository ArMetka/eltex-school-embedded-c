#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define USAGE_PROMPT "usage: ./server PORT"
#define BUF_SIZE 65536
#define DISCONNECT_MSG "exit"

struct client_list {
    unsigned int ip;
    unsigned short port;
    int msg_count;
    struct client_list *next;
};

sig_atomic_t should_exit = 0;

unsigned short checksum(unsigned short *ptr, int n);
void send_back(int sock_out, struct iphdr *iph_recv, struct udphdr *udph_recv, const char *message);
void sig_handler(int signal);

int main(int argc, char **argv) {
    char buffer[BUF_SIZE];
    char msg_buf[BUF_SIZE];
    int sock_in, sock_out;
    int port;
    struct client_list *head = NULL;
    int client_count = 0;

    if (argc < 2) {
        fprintf(stderr, "No port found in argv\n%s\n", USAGE_PROMPT);
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);

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

    printf("server is listening on UDP port %d\n", port);
    while (!should_exit) {
        struct sockaddr_in saddr;
        socklen_t saddr_size = sizeof(saddr);

        int data_size = recvfrom(sock_in, buffer, BUF_SIZE, 0, (struct sockaddr *)&saddr, &saddr_size);
        if (data_size < 0) {
            if (errno != EINTR) {  // (not) Interrupted system call (exit via ^C)
                perror("failed to receive data");
            }
            continue;
        }

        // IP header
        struct iphdr *iph = (struct iphdr *)buffer;
        unsigned short iphdrlen = iph->ihl * 4;

        // UDP header
        struct udphdr *udph = (struct udphdr *)(buffer + iphdrlen);

        // skip packets to other ports
        if (udph->uh_dport != htons(port)) {
            continue;
        }

        // find client
        struct client_list *ptr = head;
        struct client_list *prev = NULL;
        while (ptr) {
            if (ptr->ip == iph->saddr && ptr->port == udph->uh_sport) {
                break;
            }
            prev = ptr;
            ptr = ptr->next;
        }

        if (!ptr) {  // new client
            ptr = (struct client_list *)malloc(sizeof(struct client_list));
            memset(ptr, 0, sizeof(struct client_list));
            ptr->ip = iph->saddr;
            ptr->port = udph->uh_sport;
            if (!head) {
                head = ptr;
            }
        }
        ptr->msg_count++;

        // "log" message
        unsigned char *data = buffer + iphdrlen + sizeof(struct udphdr);
        int data_length = ntohs(udph->len) - sizeof(struct udphdr);
        printf("%s:%d (#%d): \"", inet_ntoa(*(struct in_addr *)&iph->saddr), ntohs(udph->source),
               ptr->msg_count);
        for (int i = 0; i < data_length; i++) {
            if (data[i] >= 32 && data[i] <= 126) {
                putchar(data[i]);
            } else {
                putchar('.');
            }
        }
        printf("\"\n");

        if (strcmp(data, DISCONNECT_MSG) == 0) {  // disconnect
            if (ptr->next && prev) {
                prev->next = ptr->next;
            } else if (!prev) {
                head = ptr->next;
            }
            free(ptr);
        } else {  // send echo response
            sprintf(msg_buf, "%s %d", (char *)data, ptr->msg_count);
            send_back(sock_out, iph, udph, msg_buf);
        }
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

void send_back(int sock_out, struct iphdr *iph_recv, struct udphdr *udph_recv, const char *message) {
    char packet[BUF_SIZE];

    struct iphdr *iph = (struct iphdr *)packet;
    iph->saddr = iph_recv->daddr;
    iph->daddr = iph_recv->saddr;
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
    udph->source = udph_recv->dest;
    udph->dest = udph_recv->source;
    udph->len = htons(sizeof(struct udphdr) + strlen(message) + 1);  // udph len
    udph->check = 0;
    udph->check = checksum((unsigned short *)udph, sizeof(struct udphdr));

    char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    strncpy(data, message, BUF_SIZE - (sizeof(struct iphdr) + sizeof(struct udphdr)) - 1);

    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_in));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = iph->daddr;

    ssize_t sent =
        sendto(sock_out, packet, ntohs(iph->tot_len), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
    if (sent < 0) {
        perror("failed to send echo response");
    }
}

void sig_handler(int signal) {
    should_exit = 1;
}
