#include "threadpool.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void * thread_job(void *);

struct threadpool thread_pool_constructor (int num_threads){
	struct threadpool threadpool;
	threadpool.num_threads = num_threads;
	threadpool.active = 1;

	threadpool.pool = (pthread_t *)malloc(sizeof(pthread_t[num_threads]));
	for(int i = 0; i < num_threads; i++){
		pthread_create(&threadpool.pool[i], NULL, thread_job,NULL);
	}

	threadpool.work = queue_cocnstructor();
	threadpool.lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	threadpool.signal = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

	return threadpool;
};

void thread_pool_destructor(struct threadpool *thread_pool){
	thread_pool->active = 0;
	for(int i = 0; i < thread_pool; i++){
		pthread_cond_signal(&thread_pool->signal);
	}
	for(int i = 0; i < thread_pool->num_threads; i++){
		pthread_join(thread_pool->pool[i], NULL);
	}
	free(thread_pool->pool);
	queue_destructor(&thread_pool->work);
}

struct threadJob thread_job_constructor(void * (*job_func)(void *arg), void *arg){
	struct threadJob job;
	job.job = job_func;
	job.arg = arg;
	return job;
}

void *thread_job(void *arg){
	struct threadpool *thread_pool = (struct threadpool *)arg;
	while (thread_pool->active == 1){
		pthread_mutex_lock(&thread_pool->lock);
		pthread_cond_wait(&thread_pool->signal, &thread_pool->lock);
		struct threadJob job = *(struct ThreadJob *)thread_pool->work.peak(&thread_pool->work);
		thread_pool->work.pop(&thread_pool->work);
		pthread_mutex_unlock(&thread_pool->lock);

		if(job.job){
			job.job(job.arg);
		}
	}
	return NULL;
}







typedef struct ThreadPool {
	/* TODO: benötigte Attribute hinzufügen */
} ThreadPool;

/* TODO: interne, globale Variablen hinzufügen */

/* TODO: interne Hilfsfunktionen hinzufügen */

int tpInit(size_t size) {
	return 0;
}

void tpRelease(void) {}

void tpAsync(Future *future) {}

void tpAwait(Future *future) {}
