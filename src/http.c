#include "http.h"
#include "metrics.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

static void send_response(int client_fd, int status_code, const char *content_type, const char *body) {
    char response[8192];
    const char *status_text = (status_code == 200) ? "OK" : "Not Found";
    
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code, status_text, content_type, strlen(body), body);

    write(client_fd, response, len);
}

int http_parse_request(const char *raw_request, http_request_t *request) {
    memset(request, 0, sizeof(http_request_t));
    
    const char *line_end = strstr(raw_request, "\r\n");
    if (!line_end) return -1;

    char first_line[MAX_PATH_LEN + MAX_METHOD_LEN + 20];
    size_t first_line_len = line_end - raw_request;
    if (first_line_len >= sizeof(first_line)) return -1;
    
    strncpy(first_line, raw_request, first_line_len);
    first_line[first_line_len] = '\0';

    if (sscanf(first_line, "%s %s", request->method, request->path) != 2) {
        return -1;
    }

    const char *body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        request->body_len = strlen(body_start);
        if (request->body_len < MAX_BODY_LEN) {
            strncpy(request->body, body_start, request->body_len);
            request->body[request->body_len] = '\0';
        }
    }

    return 0;
}

void http_handle_request(int client_fd, http_request_t *request) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    metrics_increment_requests();
    logger_log(LOG_INFO, "Request: %s %s", request->method, request->path);

    if (strcmp(request->path, "/") == 0) {
        send_response(client_fd, 200, "text/plain", "Welcome to Helios Server!");
    } else if (strcmp(request->path, "/health") == 0) {
        char body[256];
        metrics_t m = metrics_get();
        snprintf(body, sizeof(body), "{\"status\":\"OK\",\"connections\":%u,\"uptime\":%ld}", 
                 m.active_connections, time(NULL)); // Uptime calculation simplified for now
        send_response(client_fd, 200, "application/json", body);
    } else if (strcmp(request->path, "/metrics") == 0) {
        char body[512];
        metrics_t m = metrics_get();
        double avg_resp = (m.total_requests > 0) ? (m.total_response_time / m.total_requests) : 0;
        snprintf(body, sizeof(body), 
                 "Requests served: %lu\n"
                 "Active connections: %u\n"
                 "Average response time: %.2f ms\n",
                 m.total_requests, m.active_connections, avg_resp);
        send_response(client_fd, 200, "text/plain", body);
    } else if (strcmp(request->path, "/echo") == 0) {
        if (request->body_len > 0) {
            send_response(client_fd, 200, "text/plain", request->body);
        } else {
            send_response(client_fd, 200, "text/plain", "No body received");
        }
    } else {
        send_response(client_fd, 404, "text/plain", "404: Endpoint not found");
        logger_log(LOG_ERROR, "404 Not Found: %s", request->path);
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    metrics_add_response_time(elapsed);
}
