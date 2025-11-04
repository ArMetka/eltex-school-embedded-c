#include "./queue.h"

Queue initQueue() {
    Queue queue;
    queue.head = NULL;

    return queue;
}

void destroyQueue(Queue *queue) {
    if (queue) {
        QueueNode *tmp;
        while (queue->head) {
            tmp = queue->head;
            queue->head = tmp->next;
            free(tmp);
        }
    }
}

int push(Queue *queue, void *data, unsigned char priority) {
    if (!queue) {
        return 0;
    }

    QueueNode *newnode = (QueueNode*)calloc(1, sizeof(QueueNode));
    newnode->priority = priority;
    newnode->data = data;

    if (queue->head == NULL) {
        newnode->next = NULL;
        queue->head = newnode;
        return 1;
    }

    QueueNode *curr = queue->head;
    QueueNode *prev = NULL;
    while (curr) {
        if (curr->priority > priority) {
            newnode->next = curr;        
            if (prev) {
                prev->next = newnode;
            } else {
                queue->head = newnode;
            }
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }

    // lowest priority
    newnode->next = NULL;
    prev->next = newnode;

    return 1;
}

void *pop(Queue *queue) {
    if (!queue) {
        return NULL;
    }

    QueueNode *tmp = queue->head;
    if (tmp) {
        void *result = tmp->data;
        queue->head = tmp->next;
        free(tmp);

        return result;
    }

    return NULL;
}

void *popExactPriority(Queue *queue, unsigned char priority) {
    if (!queue) {
        return NULL;
    }

    QueueNode *curr = queue->head;
    QueueNode *prev = NULL;
    while (curr) {
        if (curr->priority == priority) {
            if (prev) {
                prev->next = curr->next;    
            } else {  // pop first
                queue->head = curr->next;
            }

            void *result = curr->data;
            free(curr);

            return result;
        } else if (curr->priority > priority) {
            return NULL;
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void *popAtLeastPriority(Queue *queue, unsigned char priority) {
    if (!queue) {
        return NULL;
    }

    QueueNode *curr = queue->head;
    QueueNode *prev = NULL;
    while (curr) {
        if (curr->priority <= priority) {
            if (prev) {
                prev->next = curr->next;    
            } else {  // pop first
                queue->head = curr->next;
            }

            void *result = curr->data;
            free(curr);

            return result;
        } else {
            return NULL;
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void printQueue(Queue *queue, void (*print_callback)(void *)) {
    if (!queue) {
        printf("Queue is NULL!\n");
        return;
    }

    QueueNode *tmp = queue->head;
    printf("(head)\t->\t");
    while (tmp) {
        print_callback(tmp->data);
        printf("(p=%u)\t->\t", tmp->priority);
        tmp = tmp->next;
    }
    printf("NULL\n");
}
