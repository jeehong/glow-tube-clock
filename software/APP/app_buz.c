#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_buz.h"

#include "main.h"

#include "stm32f10x_tim.h"

void app_buz_task(GLOBAL_SOURCE_t *p_src)
{
    int index;
        
    while(1)
    {
		xSemaphoreTake(p_src->xBuzzer, portMAX_DELAY);
        for(index = p_src->buz[0]; index > 0; index--)
        {
            TIM_CtrlPWMOutputs(TIM1, ENABLE);
            vTaskDelay(mainDELAY_MS(500)); 
            TIM_CtrlPWMOutputs(TIM1, DISABLE);
            vTaskDelay(mainDELAY_MS(500)); 
        }
		       
    }
}
