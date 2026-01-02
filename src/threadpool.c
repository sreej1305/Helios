#include "threadpool.h"
#include <stdlib.h>
#include <unistd.h>

static void *threadpool_worker(void *threadpool);

threadpool_t *threadpool_create(int thread_count, int queue_size) {
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if (!pool) return NULL;

    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = false;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);

    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&(pool->threads[i]), NULL, threadpool_worker, (void *)pool);
    }

    return pool;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *), void *argument) {
    int next;

    pthread_mutex_lock(&(pool->lock));

    next = (pool->tail + 1) % pool->queue_size;

    if (pool->count == pool->queue_size) {
        pthread_mutex_unlock(&(pool->lock));
        return -1; // Queue full
    }

    if (pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
        return -2; // Shutdown in progress
    }

    pool->queue[pool->tail].function = function;
    pool->queue[pool->tail].argument = argument;
    pool->tail = next;
    pool->count++;

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

int threadpool_destroy(threadpool_t *pool) {
    if (!pool) return -1;

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = true;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    free(pool->queue);
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool);

    return 0;
}

static void *threadpool_worker(void *threadpool) {
    threadpool_t *pool = (threadpool_t *)threadpool;

    while (true) {
        pthread_mutex_lock(&(pool->lock));

        while ((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown && (pool->count == 0)) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        threadpool_task_t task = pool->queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count--;

        pthread_mutex_unlock(&(pool->lock));

        (task.function)(task.argument);
    }

    return NULL;
}
