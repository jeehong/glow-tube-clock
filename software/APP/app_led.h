#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "stm32f10x.h"
	
#define mainDELAY						( ( TickType_t ) 100 / portTICK_PERIOD_MS )
	 
void  BSP_LED_Init(void);
void vLedTask( void *pvParameters );
void vRelay1Task( void *pvParameters );
void vRelay2Task( void *pvParameters );
void vRelay3Task( void *pvParameters );
	 

#endif
