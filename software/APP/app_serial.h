#ifndef __APP_SERIAL_H__
#define __APP_SERIAL_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "misc.h"

#include "FreeRTOS.h"
#include "queue.h"

typedef void * xComPortHandle;

/* Misc defines. */
#define serINVALID_QUEUE				( ( QueueHandle_t ) 0 )
#define serNO_BLOCK						( ( TickType_t ) 0 )
#define serTX_BLOCK_TIME				( 40 / portTICK_PERIOD_MS )

xComPortHandle serial_init( unsigned long ulWantedBaud);
void vStartComTasks(UBaseType_t uxPriority);
void dbg_string(const char *fmt,...);


#endif

