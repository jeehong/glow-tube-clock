#include "FreeRTOS.h"
#include "task.h"

/* Library includes. */
#include "stm32f10x.h"

#include "app_led.h"

void  BSP_LED_Init(void)
{
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启GPIOB和GPIOF的外设时钟*/
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); 

		/*选择要控制的GPIOB引脚*/															   
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_12;	

		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

		/*调用库函数，初始化GPIOB0*/
		GPIO_Init(GPIOC, &GPIO_InitStructure);			  

		/* 关闭所有led灯	*/
		GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
	  GPIO_SetBits(GPIOC, GPIO_Pin_12);
}

void vLedTask( void *pvParameters )
{
  while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_12);
		vTaskDelay(1920);
		GPIO_ResetBits(GPIOC, GPIO_Pin_12);
		vTaskDelay(80);
	}
}

void vRelay1Task( void *pvParameters )
{
  while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_0);
		vTaskDelay(10000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_0);
		vTaskDelay(10000);
	}
}

void vRelay2Task( void *pvParameters )
{
  while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_1);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_1);
		vTaskDelay(3000);
	}
}

void vRelay3Task( void *pvParameters )
{
  while(1)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_2);
		vTaskDelay(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_2);
		vTaskDelay(7000);
	}
}


