#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_buz.h"

#include "main.h"

#include "stm32f10x_tim.h"

static u8 warning_times = 0, running = FALSE;

void app_buz_task(void *parame)
{
    int index;
        
    while(1)
    {
		if(running == FALSE)
			vTaskDelay(100);
		else
		{
	        for(index = warning_times; index > 0; index--)
	        {
	            TIM_CtrlPWMOutputs(TIM1, ENABLE);
	            vTaskDelay(mainDELAY_MS(500)); 
	            TIM_CtrlPWMOutputs(TIM1, DISABLE);
	            vTaskDelay(mainDELAY_MS(500)); 
	        }
			running = FALSE;
		}
    }
}

void app_buzzer_running(u8 state)
{
	running = state;
}

void app_buzzer_set_times(u8 times)
{
	warning_times = times;
}
