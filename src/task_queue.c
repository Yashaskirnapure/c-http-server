#include "task_queue.h"
#include "thread_pool.h"

task_queue_t* task_queue_init(){
	task_queue_t* queue = (task_queue_t*) malloc(sizeof(task_queue_t));
	if(!queue) return NULL;

	queue->count = 0;
	queue->front = 0;
	queue->rear = 0;

	pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
	pthread_cond_init(&queue->not_full, NULL);

	return queue;
}

bool task_queue_is_empty(task_queue_t* queue) {
    return queue->count == 0;
}

bool task_queue_is_full(task_queue_t* queue) {
    return queue->count == MAX_QUEUE_SIZE;
}

void task_push(thread_pool_t* pool, int socket){
	task_queue_t* queue = pool->queue;
	pthread_mutex_lock(&queue->mutex);

	while(queue->count == MAX_QUEUE_SIZE && !pool->stop)
		pthread_cond_wait(&queue->not_full, &queue->mutex);
	
	if (atomic_load(&pool->stop)) {
		pthread_mutex_unlock(&queue->mutex);
		return;
	}

	queue->sockets[queue->rear] = socket;
	queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
	queue->count++;

	pthread_cond_signal(&queue->not_empty);
	pthread_mutex_unlock(&queue->mutex);
}

int task_pop(thread_pool_t* pool){
	task_queue_t* queue = pool->queue;
	pthread_mutex_lock(&queue->mutex);
	
	while(queue->count == 0 && !pool->stop)
		pthread_cond_wait(&queue->not_empty, &queue->mutex);
	
	if(atomic_load(&pool->stop)) {
		pthread_mutex_unlock(&queue->mutex);
		return -1;
	}
	
	int client_socket = queue->sockets[queue->front];
	queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
	queue->count--;

	pthread_cond_signal(&queue->not_full);

	pthread_mutex_unlock(&queue->mutex);
	return client_socket;
}

void task_queue_destroy(task_queue_t* queue){
	if(!queue) return;

	pthread_mutex_lock(&queue->mutex);
	queue->count = 0;
	queue->front = 0;
	queue->rear = 0;
	pthread_mutex_unlock(&queue->mutex);

	pthread_cond_destroy(&queue->not_empty);
	pthread_cond_destroy(&queue->not_full);
	pthread_mutex_destroy(&queue->mutex);

	free(queue);
}