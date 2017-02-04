#include "hal_cli.h"
#include "app_serial.h"

portBASE_TYPE hal_cli_data_tx(signed char *data, unsigned short len)
{
	vSerialPutString(0, data, len);
	return pdTRUE;
}

portBASE_TYPE hal_cli_data_rx(signed char *data, unsigned short len)
{
	return xSerialGetChar(0, data, 1);
}

