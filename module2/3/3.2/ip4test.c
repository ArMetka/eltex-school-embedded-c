#include "./ip4test.h"
#include <string.h>

int main(int argc, char **argv) {
    unsigned int gateway;
    unsigned int netmask;
    int n;

    if (!readArgs(argc, argv, &gateway, &netmask, &n)) {
        return 1;
    }

    printf("Gateway: ");
    printAddr4(gateway);
    printf("\nNetmask: ");
    printAddr4(netmask);
    printf("\nNumber of packets: %d\n", n);

    int netmask_num = 0;
    unsigned int tmp = netmask;
    while (tmp != 0u) {
        netmask_num++;
        tmp = tmp << 1;
    }
    // 192.168.0.0/24
    // 2^8/2^32 == 1/2^24
    // 192.168.0.0/8
    // 2^24/2^32 == 1/2^8
    double hit_prob = 1. / pow(2, netmask_num);
    printf("Theoretical hit probability: %.6lf\n", hit_prob);
    printf("Theoretical hit count: %d\n", (int) round(hit_prob * n));

    return 0;
}

int readArgs(int argc, char **argv, unsigned int *gateway, unsigned int *netmask, int *n) {
    if (argc < 3) {
        printf("Not enough arguments!\n%s\n", USAGE_PROMPT);
        return 0;
    } else if (argc > 4) {
        printf("Too many arguments!\n%s\n", USAGE_PROMPT);
        return 0;
    } else {
        char addr_copy[128];
        strncpy(addr_copy, argv[1], 128);
        if (!parseAddr4(addr_copy, gateway)) {
            printf("Failed to parse gateway address!\n");
            return 0;
        }

        char *tmp = strchr(argv[1], '/');
        if (tmp) {
            unsigned int num_netmask = 0;
            if (sscanf(tmp + 1, "%u", &num_netmask) != 1) {
                printf("Failed to parse netmask!\n");
                return 0;
            }
            *netmask = 0xFFFFFFFFu << (32 - num_netmask);
            if (sscanf(argv[2], "%d", n) != 1) {
                printf("Failed to parse n!\n");
                return 0;
            }
        } else {
            if (!parseAddr4(argv[2], netmask)) {
                printf("Failed to parse netmask!\n");
                return 0;
            }
            if (sscanf(argv[3], "%d", n) != 1) {
                printf("Failed to parse n!\n");
                return 0;
            }
        }
    }

    return 1;
}

int parseAddr4(char *src, unsigned int *result) {
    *result = 0;
    char *octet = strtok(src, "./");
    if (!octet) {
        return 0;
    }

    for (int i = 0; i < 4; i++) {
        unsigned int tmp;
        if (sscanf(octet, "%u", &tmp) != 1) {
            return 0;
        }
        *result += tmp;
        if (i != 3) {
            *result = *result << 8;
        }

        octet = strtok(NULL, "./");
        if (!octet && i != 3) {
            return 0;
        }
    }

    return 1;
}

void printAddr4(unsigned int addr) {
    printf("%u.%u.%u.%u", (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF);
}
