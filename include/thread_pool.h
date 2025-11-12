#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include "task_queue.h"
#include <stdbool.h>
#include <stdatomic.h>
#include <stdlib.h>

#define MAX_THREADS 10

typedef struct {
	pthread_t worker_threads[MAX_THREADS];
	task_queue_t* queue;
	atomic_bool stop;
	pthread_mutex_t lock;
} thread_pool_t;

thread_pool_t* thread_pool_init();
void thread_pool_destroy(thread_pool_t* thread_pool);
void task_push(thread_pool_t* pool, int socket);
int task_pop(thread_pool_t* pool);

#endif