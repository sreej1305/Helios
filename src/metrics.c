#include "metrics.h"
#include <string.h>

static metrics_t global_metrics;

void metrics_init() {
    memset(&global_metrics, 0, sizeof(metrics_t));
    pthread_mutex_init(&global_metrics.lock, NULL);
}

void metrics_increment_requests() {
    pthread_mutex_lock(&global_metrics.lock);
    global_metrics.total_requests++;
    pthread_mutex_unlock(&global_metrics.lock);
}

void metrics_add_response_time(double ms) {
    pthread_mutex_lock(&global_metrics.lock);
    global_metrics.total_response_time += ms;
    pthread_mutex_unlock(&global_metrics.lock);
}

void metrics_increment_connections() {
    pthread_mutex_lock(&global_metrics.lock);
    global_metrics.active_connections++;
    pthread_mutex_unlock(&global_metrics.lock);
}

void metrics_decrement_connections() {
    pthread_mutex_lock(&global_metrics.lock);
    if (global_metrics.active_connections > 0) {
        global_metrics.active_connections--;
    }
    pthread_mutex_unlock(&global_metrics.lock);
}

metrics_t metrics_get() {
    metrics_t snapshot;
    pthread_mutex_lock(&global_metrics.lock);
    snapshot = global_metrics;
    pthread_mutex_unlock(&global_metrics.lock);
    return snapshot;
}

void metrics_cleanup() {
    pthread_mutex_destroy(&global_metrics.lock);
}
