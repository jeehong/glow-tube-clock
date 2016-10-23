#ifndef __UART_CONSOLE_H__
#define	__UART_CONSOLE_H__

#include "FreeRTOS.h" 

void vUARTCommandConsoleStart( unsigned short usStackSize, UBaseType_t uxPriority, TaskHandle_t *handle);

#endif
