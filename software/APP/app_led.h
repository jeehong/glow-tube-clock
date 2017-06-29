#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "stm32f10x.h"

#include "main.h"


#define	LED_RCC_APB			RCC_APB2Periph_GPIOC
#define	LED_PIN_GROUP		GPIOC
#define	LED_PIN_R			GPIO_Pin_5
#define	LED_PIN_G			GPIO_Pin_2
#define	LED_PIN_B			GPIO_Pin_3


void app_led_init(void);	 
void app_led_task_blink(void *parame);
	 

#endif
