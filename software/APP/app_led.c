#include "string.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/* Library includes. */
#include "stm32f10x.h"

#include "main.h"

#include "app_led.h"
#include "app_serial.h"
#include "app_display.h"
#include "app_sht10.h"

#define DEFAULT_COLOR_BLUE		1

void app_led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LED_RCC_APB, ENABLE); 													   
	GPIO_InitStructure.GPIO_Pin = LED_PIN_R | LED_PIN_G | LED_PIN_B;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(LED_PIN_GROUP, &GPIO_InitStructure);			  
	GPIO_ResetBits(LED_PIN_GROUP, LED_PIN_R | LED_PIN_G | LED_PIN_B);	
}

static u8 color = DEFAULT_COLOR_BLUE;

void app_led_task_blink(void *parame)
{
	portTickType xLastWakeTime;
	uint16_t port_list[3] = {LED_PIN_R, LED_PIN_G, LED_PIN_B};
	u8 col = color;
	
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		col = color;
		LED_PIN_GROUP->BSRR = port_list[col];
		vTaskDelayUntil(&xLastWakeTime, mainDELAY_MS(100));	
		LED_PIN_GROUP->BRR = port_list[col];
		vTaskDelayUntil(&xLastWakeTime, mainDELAY_MS(1900));
	}
}

void app_led_set_color(u8 led)
{
	if(led > 2)
		color = DEFAULT_COLOR_BLUE;
	else
		color = led;
}
