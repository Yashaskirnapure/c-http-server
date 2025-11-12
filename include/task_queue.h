#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE 10

typedef struct {
	int socket;
	task_node_t* next;
} task_node_t;

typedef struct {
	task_node_t* first;
	task_node_t* last;
	pthread_mutex_t mutex;
	pthread_cond_t not_empty;
} task_queue_t;

task_node_t* create_task(int socket);
task_queue_t* task_queue_init(void);
void task_queue_destroy(task_queue_t* task_queue);

#endif