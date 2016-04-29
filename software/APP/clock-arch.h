
/*
ancheel changed it for STM32 2010-6-6
www.anchey.com
*/

#ifndef __CLOCK_ARCH_H__
#define __CLOCK_ARCH_H__

#include <stdint.h>

typedef uint32_t clock_time_t;
//typedef uint16_t clock_time_t;	//8λ������16λ����Ϊϵͳ�δ�

#define CLOCK_CONF_SECOND 1000		//ϵͳ�δ����� 1ms


extern volatile clock_time_t sys_ticks;

#define clock_time() sys_ticks//����ʹ�ú궨��������ϵͳ����

void clock_arch_init(void);
void SysTick_Handler(void);
#endif /* __CLOCK_ARCH_H__ */
