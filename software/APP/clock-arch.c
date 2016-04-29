/*STM32 uip 移植
*ancheel
*www.anchey.com
*/
#include <stm32f10x.h>
#include <misc.h>
#include "clock-arch.h"

volatile clock_time_t sys_ticks;

//使用SysTick作为系统滴答
void clock_arch_init(void)
{
    sys_ticks = 0;

    SysTick_Config( 72000000UL / CLOCK_CONF_SECOND);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    sys_ticks++;
}

/*
;*****************************************************************************************************
;*                            			End Of File
;*****************************************************************************************************
;*/
