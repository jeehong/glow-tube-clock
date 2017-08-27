#include "os_inc.h"

#include "stm32f10x_tim.h"

#include "app_inc.h"


static U8 warning_times = 0;

void app_buz_task(void *parame)
{
	U32 index;

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

void app_buzzer_set_times(U8 times)
{
	warning_times = times;
}
