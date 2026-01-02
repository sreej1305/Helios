#ifndef HELIOS_CONFIG_H
#define HELIOS_CONFIG_H

typedef struct {
    int port;
    int thread_pool_size;
    int queue_size;
    char log_path[256];
} config_t;

int config_load(const char *filename, config_t *config);

#endif // HELIOS_CONFIG_H
