#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "stm32f10x.h"

#include "app_led.h"
#include "app_serial.h"



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


void app_led_task_blink(void *pvParameters)
{
	portTickType xLastWakeTime;
	uint16_t port_list[3] = {LED_PIN_R, LED_PIN_G, LED_PIN_B};
	unsigned char index = 0;
	
	xLastWakeTime = xTaskGetTickCount();
	
	while(1)
	{
		/* GPIO_ResetBits(LED_PIN_GROUP, port_list[index % 3]); */
		LED_PIN_GROUP->BRR = port_list[index % 3];
		vTaskDelayUntil(&xLastWakeTime, mainDELAY_MS(80));
		/* GPIO_SetBits(LED_PIN_GROUP, port_list[index % 3]); */
		LED_PIN_GROUP->BSRR = port_list[index % 3];
		vTaskDelayUntil(&xLastWakeTime, mainDELAY_MS(1920));
		index++;
		dbg_string("[%d]\r\n", index);	
	}
}


