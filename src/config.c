#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int config_load(const char *filename, config_t *config) {
    // Default values
    config->port = 8080;
    config->thread_pool_size = 4;
    config->queue_size = 100;
    strcpy(config->log_path, "logs/server.log");

    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char key[256], value[256];
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            if (strcmp(key, "port") == 0) config->port = atoi(value);
            else if (strcmp(key, "thread_pool_size") == 0) config->thread_pool_size = atoi(value);
            else if (strcmp(key, "queue_size") == 0) config->queue_size = atoi(value);
            else if (strcmp(key, "log_path") == 0) strcpy(config->log_path, value);
        }
    }

    fclose(file);
    return 0;
}
