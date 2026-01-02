#include "server.h"
#include "logger.h"
#include "metrics.h"
#include "config.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handle_sigint(int sig) {
    printf("\nGracefully shutting down...\n");
    server_stop();
    logger_cleanup();
    metrics_cleanup();
    exit(0);
}

int main(int argc, char *argv[]) {
    config_t config;
    if (config_load("helios.conf", &config) != 0) {
        printf("Using default configuration.\n");
    }

    logger_init(config.log_path);
    metrics_init();

    signal(SIGINT, handle_sigint);

    server_start(&config);

    return 0;
}
