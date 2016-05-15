#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "stm32f10x.h"

#include "app_led.h"

#define mainDELAY			( ( TickType_t ) 100 / portTICK_PERIOD_MS )

void vLedTask(void *pvParameters)
{
	portTickType xLastWakeTime;

	xLastWakeTime = xTaskGetTickCount();
	
	while(1)
	{

		GPIO_SetBits(GPIOC, GPIO_Pin_12);
		vTaskDelayUntil(&xLastWakeTime, 1920);
		GPIO_ResetBits(GPIOC, GPIO_Pin_12);
		vTaskDelayUntil(&xLastWakeTime, 80);
	}
}

void vRelay1Task(void *pvParameters)
{

	while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_0);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_0);
		vTaskDelay(1000);
	}
}

void vRelay2Task(void *pvParameters)
{
	while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_1);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_1);
		vTaskDelay(3000);
	}
}

void vRelay3Task(void *pvParameters)
{
	while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_2);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_2);
		vTaskDelay(7000);
	}
}


