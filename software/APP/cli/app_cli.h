#ifndef __APP_CLI_H__
#define __APP_CLI_H__

#include "freertos.h"

void app_cli_init(u8 priority, char *t, TaskHandle_t *handle);


#endif

