#ifndef HELIOS_HTTP_H
#define HELIOS_HTTP_H

#include <stddef.h>

#define MAX_METHOD_LEN 10
#define MAX_PATH_LEN 1024
#define MAX_BODY_LEN 4096

typedef struct {
    char method[MAX_METHOD_LEN];
    char path[MAX_PATH_LEN];
    char body[MAX_BODY_LEN];
    size_t body_len;
} http_request_t;

int http_parse_request(const char *raw_request, http_request_t *request);
void http_handle_request(int client_fd, http_request_t *request);

#endif // HELIOS_HTTP_H
