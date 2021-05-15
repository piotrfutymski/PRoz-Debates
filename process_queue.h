#ifndef PROCESS_QUEUEH
#define PROCESS_QUEUEH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/* boolean */
#define TRUE 1
#define FALSE 0

typedef struct {
    int process;
    int priority;
}queue_element_t;

typedef struct{
    queue_element_t * data;
    int size;
    int reserved;
}process_queue_t;

void insertElement(process_queue_t * process_queue, queue_element_t el);
void removeProcess(process_queue_t * process_queue, int process);
void removeNFirstElements(process_queue_t * process_queue, int N);
int findProcess(process_queue_t * process_queue, int process);

void reserveQueue(process_queue_t * process_queue, int reserved);
void initQueue(process_queue_t * process_queue, int initialReserved);
void freeQueue(process_queue_t * process_queue);

int moreImportant(queue_element_t elementA, queue_element_t elementB);

#endif