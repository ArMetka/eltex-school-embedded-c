#include <stdio.h>
#include <stdlib.h>

#include "./queue.h"

#define QUEUE_DATATYPE int
#define QUEUE_DATATYPE_SCAN_FUNC getInt // QUEUE_DATATYPE* (* QUEUE_DATATYPE_SCAN_FUNC) (void)
#define QUEUE_DATATYPE_PRINT_FUNC printInt // void (* QUEUE_DATATYPE_PRINT_FUNC) (QUEUE_DATATYPE*)

char charp;

int printMenu() {
    int result = 0;

    printf("\n----Priority Queue CLI----\n");

    printf("1) Push\n");
    printf("2) Pop\n");
    printf("3) Pop with exact priority\n");
    printf("4) Pop with at least priority\n");
    printf("5) Clear queue\n");
    printf("6) Print queue\n");
    printf("0) Exit\n");

    printf("\nselect op: ");
    scanf("%d%c", &result, &charp);

    return result;
}

int *getInt() {
    int *result = (int*)calloc(1, sizeof(int));
    scanf("%d", result);

    return result;
}

void printInt(void *value) {
    printf("%d", *((int *) value));
}

QUEUE_DATATYPE *getData() {
    printf("enter data: ");
    QUEUE_DATATYPE *result = QUEUE_DATATYPE_SCAN_FUNC();
    scanf("%c", &charp);

    return result;
}

unsigned char getPriority() {
    unsigned int result = 1;
    do {
        if (result > 255) {
            printf("Priority must be in range [0,255]!\n");
        }
        printf("enter priority (0-255): ");
        scanf("%u%c", &result, &charp);
    } while (result > 255);

    return (unsigned char) result;
}

int main() {
    Queue queue = initQueue();

    while (1 != 0) {
        int op = printMenu();
        if (op == 0) { // exit
            break;
        }
        
        QUEUE_DATATYPE *buf;
        switch (op) {
            case 1: // push
                push(&queue, getData(), getPriority());
                break;
            case 2: // pop
                buf = pop(&queue);
                printf("value = ");
                if (buf) {
                    QUEUE_DATATYPE_PRINT_FUNC(buf);
                    free(buf);
                } else {
                    printf("NULL");
                }
                printf("\n");
                break;
            case 3: // pop exact
                buf = popExactPriority(&queue, getPriority());
                if (buf) {
                    QUEUE_DATATYPE_PRINT_FUNC(buf);
                    free(buf);
                } else {
                    printf("NULL");
                }
                printf("\n");
                break;
            case 4: // pop at least
                buf = popAtLeastPriority(&queue, getPriority());
                if (buf) {
                    QUEUE_DATATYPE_PRINT_FUNC(buf);
                    free(buf);
                } else {
                    printf("NULL");
                }
                printf("\n");
                break;
            case 5: // clear
                destroyQueue(&queue);
                queue = initQueue();
                break;
            case 6: // print
                printQueue(&queue, printInt);
                break;
            default:
                printf("Invalid input!\n");
        }
    }

    return 0;
}
