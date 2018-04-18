#include "string.h"

#include "FreeRTOS.h"
#include "os_inc.h"

#include "stm32f10x.h"

#include "app_inc.h"

#define DEFAULT_COLOR_BLUE		1
#define MAX_TYPES_COLOR			3

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

static U16 led_color = DEFAULT_COLOR_BLUE;
static U16 led_status = ON;
const uint16_t port_list[MAX_TYPES_COLOR] = {LED_PIN_R, LED_PIN_G, LED_PIN_B};

void app_led_task_blink(void *parame)
{
	portTickType xLastWakeTime;
	
	app_data_read_led_color(&led_color);
	app_data_read_led_status(&led_status);
	if(led_color >= MAX_TYPES_COLOR)
	{
		led_color = DEFAULT_COLOR_BLUE;
	}
	app_led_set_running(led_status);
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		LED_PIN_GROUP->BSRR = port_list[led_color];
		vTaskDelayUntil(&xLastWakeTime, mainDELAY_MS(100));	
		LED_PIN_GROUP->BRR = port_list[led_color];
		vTaskDelayUntil(&xLastWakeTime, mainDELAY_MS(1900));
	}
}

void app_led_set_color(U8 color)
{
	if(color >= MAX_TYPES_COLOR)
	{
		led_color = DEFAULT_COLOR_BLUE;
	}
	else
	{
		led_color = color;
	}
	app_data_write_led_color(led_color);
}

void app_led_set_status(U16 status)
{
	led_status = status & 0x1;
	app_led_set_running(led_status);
	app_data_write_led_status(led_status);
}

void app_led_set_running(U16 status)
{
	TaskHandle_t pled = main_get_task_handle(TASK_HANDLE_LED);
	
	if(status & led_status)
	{
		vTaskResume(pled);
	}
	else
	{
		GPIO_ResetBits(LED_PIN_GROUP, LED_PIN_R | LED_PIN_G | LED_PIN_B);
		vTaskSuspend(pled);
	}
}
