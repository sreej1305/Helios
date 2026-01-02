#include "server.h"
#include "logger.h"
#include "metrics.h"
#include "threadpool.h"
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_EVENTS 64
#define BUFFER_SIZE 8192

static int server_fd;
static int epoll_fd;
static threadpool_t *pool = NULL;
static volatile bool running = false;

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

typedef struct {
    int client_fd;
} client_context_t;

static void handle_client_task(void *arg) {
    client_context_t *ctx = (client_context_t *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(ctx->client_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        http_request_t request;
        if (http_parse_request(buffer, &request) == 0) {
            http_handle_request(ctx->client_fd, &request);
        }
    }

    close(ctx->client_fd);
    metrics_decrement_connections();
    free(ctx);
}

void server_start(config_t *config) {
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        logger_log(LOG_ERROR, "Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        logger_log(LOG_ERROR, "Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config->port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        logger_log(LOG_ERROR, "Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 100) < 0) {
        logger_log(LOG_ERROR, "Listen failed");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(server_fd);

    epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    pool = threadpool_create(config->thread_pool_size, config->queue_size);
    running = true;

    logger_log(LOG_INFO, "Server started on port %d", config->port);
    printf("Helios Server is running on port %d...\n", config->port);

    struct epoll_event events[MAX_EVENTS];
    while (running) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                while (true) {
                    struct sockaddr_in client_addr;
                    socklen_t addrlen = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        logger_log(LOG_ERROR, "Accept failed");
                        break;
                    }

                    set_nonblocking(client_fd);
                    metrics_increment_connections();

                    client_context_t *ctx = malloc(sizeof(client_context_t));
                    ctx->client_fd = client_fd;
                    if (threadpool_add(pool, handle_client_task, ctx) != 0) {
                        logger_log(LOG_ERROR, "Failed to add task to thread pool");
                        close(client_fd);
                        metrics_decrement_connections();
                        free(ctx);
                    }
                }
            }
        }
    }

    server_stop();
}

void server_stop() {
    if (!running) return;
    running = false;
    logger_log(LOG_INFO, "Stopping server...");
    close(server_fd);
    close(epoll_fd);
    if (pool) threadpool_destroy(pool);
}
