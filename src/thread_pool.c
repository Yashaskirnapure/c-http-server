#include "thread_pool.h"
#include "server.h"

static void* worker_threads(void* args){
	thread_pool_t* thread_pool = (thread_pool_t*) args;

	while(1){
		if(atomic_load(&thread_pool->stop)) break;
		int client_socket = task_pop(&thread_pool->queue);
		if(client_socket < 0) break;

		int* socket = (int*)malloc(sizeof(int));
		*socket = client_socket;
		handle_client((void*)client_socket);
	}
	
	pthread_mutex_lock(&thread_pool->lock);
	thread_pool->created_threads--;
	pthread_mutex_lock(&thread_pool->lock);

	return NULL;
}

thread_pool_t* thread_pool_init(){
	thread_pool_t* thread_pool = (thread_pool_t*) malloc(sizeof(thread_pool_t));
	if(!thread_pool) return NULL;

	thread_pool->queue = task_queue_init();
	atomic_init(&thread_pool->stop, false);
	thread_pool->created_threads = 0;
	pthread_mutex_init(&thread_pool->lock, NULL);

	for(int i = 0 ; i < MAX_THREADS ; i++){
		int rv = pthread_create(&thread_pool->worker_threads[i], NULL, worker_threads, thread_pool);
		if(rv == 0){
			pthread_detach(&thread_pool->worker_threads[i]);
			thread_pool->created_threads++;
		}
		else break;
	}

	return thread_pool;
}

void thread_pool_destroy(thread_pool_t* thread_pool){
	atomic_store(&thread_pool->stop, true);
	pthread_cond_broadcast(&thread_pool->queue->not_empty);
	pthread_mutex_destroy(&thread_pool->lock);
	task_queue_destroy(thread_pool->queue);
	free(thread_pool);
}

void task_push(thread_pool_t* pool, int socket){
	task_queue_t* queue = pool->queue;
	pthread_mutex_lock(&queue->mutex);

	if (atomic_load(&pool->stop)) {
		pthread_mutex_unlock(&queue->mutex);
		return;
	}

	if(queue->first == NULL){
		queue->first = create_task(socket);
		queue->last = queue->first;
	}else{
		queue->last->next = create_task(socket);
		queue->last = queue->last->next;
	}

	pthread_cond_signal(&queue->not_empty);
	pthread_mutex_unlock(&queue->mutex);
}

int task_pop(thread_pool_t* pool){
	task_queue_t* queue = pool->queue;
	pthread_mutex_lock(&queue->mutex);
	
	while(queue->first == NULL && !pool->stop)
		pthread_cond_wait(&queue->not_empty, &queue->mutex);
	
	if(atomic_load(&pool->stop)) {
		pthread_mutex_unlock(&queue->mutex);
		return -1;
	}

	task_node_t* task = queue->first;
	queue->first = queue->first->next;
	if(queue->first == NULL) queue->last = NULL;

	pthread_mutex_unlock(&queue->mutex);
	return task->socket;
}