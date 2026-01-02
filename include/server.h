#ifndef HELIOS_SERVER_H
#define HELIOS_SERVER_H

#include "config.h"
#include <stdbool.h>

void server_start(config_t *config);
void server_stop();

#endif // HELIOS_SERVER_H
