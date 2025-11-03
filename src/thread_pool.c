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
		if(rv == 0) thread_pool->created_threads++;
		else break;
	}

	return thread_pool;
}

void add_task(thread_pool_t* thread_pool, int socket){
	if(atomic_load(&thread_pool->stop)) return;
	task_push(&thread_pool->queue, socket);
}

void thread_pool_destroy(thread_pool_t* thread_pool){
	atomic_store(&thread_pool->stop, true);

	pthread_cond_broadcast(&thread_pool->queue->not_empty);
	pthread_cond_broadcast(&thread_pool->queue->not_full);

	for(int i = 0 ; i < thread_pool->created_threads ; i++){
		pthread_join(thread_pool->worker_threads[i], NULL);
	}

	pthread_mutex_destroy(&thread_pool->lock);
	task_queue_destroy(thread_pool->queue);
	free(thread_pool);
}