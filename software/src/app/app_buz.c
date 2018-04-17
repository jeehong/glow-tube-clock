#include "os_inc.h"

#include "stm32f10x_tim.h"

#include "app_buz.h"


static U32 warning_times = 0;

void app_buzzer_task(void *parame)
{
	warning_times = 0;
	(void) parame;
	while(1)
	{
		for(; warning_times > 0; warning_times--)
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
	warning_times += times;
}
