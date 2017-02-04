#ifndef __HAL_CLI__
#define __HAL_CLI__

#include "FreeRTOS.h"

typedef unsigned short (*data_switch)(signed char *, unsigned short);

portBASE_TYPE hal_cli_data_tx(signed char *data, unsigned short len);
portBASE_TYPE hal_cli_data_rx(signed char *data, unsigned short len);

#endif
