#ifndef HELIOS_METRICS_H
#define HELIOS_METRICS_H

#include <stdint.h>
#include <pthread.h>

typedef struct {
    uint64_t total_requests;
    uint32_t active_connections;
    double total_response_time;
    pthread_mutex_t lock;
} metrics_t;

void metrics_init();
void metrics_increment_requests();
void metrics_add_response_time(double ms);
void metrics_increment_connections();
void metrics_decrement_connections();
metrics_t metrics_get();
void metrics_cleanup();

#endif // HELIOS_METRICS_H
