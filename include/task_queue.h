#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE 10

typedef struct {
	int sockets[MAX_QUEUE_SIZE];
	int front;
	int rear;
	int count;
	pthread_mutex_t mutex;
	pthread_cond_t not_empty;
	pthread_cond_t not_full;
} task_queue_t;

task_queue_t* task_queue_init(void);
void task_push(task_queue_t* queue, int socket);
int task_pop(task_queue_t* queue);
bool task_queue_is_empty(task_queue_t* queue);
bool task_queue_is_full(task_queue_t* queue);
void task_queue_destroy(task_queue_t* task_queue);

#endif