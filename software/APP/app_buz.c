#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_buz.h"

#include "main.h"

#include "stm32f10x_tim.h"

#include "app_serial.h"

static u8 warning_times = 0;

void app_buz_task(void *parame)
{
	int index;

	while(1)
	{
		for(index = warning_times; index > 0; index--)
		{
			TIM_CtrlPWMOutputs(TIM1, ENABLE);
			vTaskDelay(mainDELAY_MS(500));
			TIM_CtrlPWMOutputs(TIM1, DISABLE);
			vTaskDelay(mainDELAY_MS(500));
		}
		vTaskDelay(mainDELAY_MS(100));
	}
}

void app_buzzer_set_times(u8 times)
{
	warning_times = times;
}
