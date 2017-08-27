#ifndef __HAL_CLI__
#define __HAL_CLI__

#include "common_type.h"

#include "os_inc.h"


typedef unsigned short (*data_switch)(char *, U16);

portBASE_TYPE hal_cli_data_tx(char *data, U16 len);
portBASE_TYPE hal_cli_data_rx(char *data, U16 len);

#endif
