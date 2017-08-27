#include "hal_cli.h"
#include "hal_uart.h"

#include "mid_dbg.h"

portBASE_TYPE hal_cli_data_tx(char *data, U16 len)
{
	hal_uart_put_string(MID_DBG_PORT, data, len);
	return pdTRUE;
}

portBASE_TYPE hal_cli_data_rx(char *data, U16 len)
{
	return hal_uart_get_char(MID_DBG_PORT, data, 10);
}

