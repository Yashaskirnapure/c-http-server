#include "task_queue.h"
#include "thread_pool.h"

task_node_t* create_task(int socket){
	task_node_t* new_task = (task_node_t*)malloc(sizeof(task_node_t));
	new_task->socket = socket;
	new_task->next = NULL;
	return new_task;
}

task_queue_t* task_queue_init(){
	task_queue_t* queue = (task_queue_t*) malloc(sizeof(task_queue_t));
	if(!queue) return NULL;

	queue->first = NULL;
	queue->last = NULL;

	pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);

	return queue;
}

void task_queue_destroy(task_queue_t* queue){
	if(!queue) return;

	pthread_mutex_lock(&queue->mutex);

	task_node_t* curr = queue->first;
	task_node_t* hold = NULL;
	while(curr){
		hold = curr->next;
		free(curr);
		curr = hold;
	}

	pthread_mutex_unlock(&queue->mutex);
	pthread_cond_destroy(&queue->not_empty);
	pthread_mutex_destroy(&queue->mutex);

	free(queue);
}