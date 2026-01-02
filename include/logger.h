#ifndef HELIOS_LOGGER_H
#define HELIOS_LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_INFO,
    LOG_ERROR
} log_level_t;

void logger_init(const char *filename);
void logger_log(log_level_t level, const char *format, ...);
void logger_cleanup();

#endif // HELIOS_LOGGER_H
