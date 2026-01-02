#ifndef HELIOS_THREADPOOL_H
#define HELIOS_THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    threadpool_task_t *queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    bool shutdown;
} threadpool_t;

threadpool_t *threadpool_create(int thread_count, int queue_size);
int threadpool_add(threadpool_t *pool, void (*function)(void *), void *argument);
int threadpool_destroy(threadpool_t *pool);

#endif // HELIOS_THREADPOOL_H
