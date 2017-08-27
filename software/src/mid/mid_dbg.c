#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal_uart.h"

#include "mid_dbg.h"

void mid_dbg_init(void)
{
	hal_uart_init(mainCOM_BAUD_RATE);
}

void dbg_string(const char *fmt, ...)
{
	va_list vp;
	char dbg_buf[100];
	
	va_start(vp, fmt);
	vsprintf(dbg_buf, fmt, vp);
	va_end(vp);
	
	hal_uart_put_string(MID_DBG_PORT, dbg_buf, strlen(dbg_buf));
}

