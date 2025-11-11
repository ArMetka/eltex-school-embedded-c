#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct QueueNode {
    struct QueueNode *next;
    unsigned char priority;
    void *data;
} QueueNode;

typedef struct {
    QueueNode *head;
} Queue;

Queue initQueue();
void destroyQueue(Queue *queue);
int push(Queue *queue, void *data, unsigned char priority);
void *pop(Queue *queue);
void *popExactPriority(Queue *queue, unsigned char priority);
void *popAtLeastPriority(Queue *queue, unsigned char priority);
void *popAtMaxPriority(Queue *queue, unsigned char priority);
void printQueue(Queue *queue, void (*print_callback) (void *));

#endif