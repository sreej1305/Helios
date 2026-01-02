#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void logger_init(const char *filename) {
    log_file = fopen(filename, "a");
    if (!log_file) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void logger_log(log_level_t level, const char *format, ...) {
    pthread_mutex_lock(&log_mutex);
    if (!log_file) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    time_t rawtime;
    struct tm *timeinfo;
    char time_str[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    const char *level_str = (level == LOG_INFO) ? "INFO" : "ERROR";
    fprintf(log_file, "[%s] [%s] ", time_str, level_str);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
    pthread_mutex_unlock(&log_mutex);
}

void logger_cleanup() {
    pthread_mutex_lock(&log_mutex);
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}
