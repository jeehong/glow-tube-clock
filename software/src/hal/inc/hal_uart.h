#ifndef __HAL_UART_H__
#define __HAL_UART_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_type.h"

#include "misc.h"

#include "os_inc.h"

typedef void * xComPortHandle;

/* Misc defines. */
#define serINVALID_QUEUE				( ( QueueHandle_t ) 0 )
#define serNO_BLOCK						( ( TickType_t ) 0 )
#define serTX_BLOCK_TIME				( 40 / portTICK_PERIOD_MS )

#define mainCOM_BAUD_RATE		( 115200 )

xComPortHandle hal_uart_init(U32 ulWantedBaud);

void hal_uart_put_string( xComPortHandle pxPort, const char* const pcString, U16 usStringLength );
signed portBASE_TYPE hal_uart_put_char( xComPortHandle pxPort, char cOutChar, TickType_t xBlockTime );
signed portBASE_TYPE hal_uart_get_char( xComPortHandle pxPort, char *pcRxedChar, TickType_t xBlockTime );

#endif

