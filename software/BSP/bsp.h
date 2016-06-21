#ifndef __BSP_H__
#define __BSP_H__

#include "stm32f10x_gpio.h"

#define mainCOM_BAUD_RATE		( 115200 )

/* HV */		
#define	HV_RCC_APB				RCC_APB2Periph_GPIOC
#define	HV_PIN_GROUP			GPIOC
#define	HV_PIN					GPIO_Pin_8

/* 74HC595 */
#define	HC595_OE_RCC_APB		RCC_APB2Periph_GPIOC	/* 输出使能 低电平有效 */
#define	HC595_OE_PIN_GROUP		GPIOC
#define	HC595_OE_PIN			GPIO_Pin_1
#define	HC595_LOCK_RCC_APB		RCC_APB2Periph_GPIOA	/* 数据锁存 上升沿有效 */
#define	HC595_LOCK_PIN_GROUP	GPIOA
#define	HC595_LOCK_PIN			GPIO_Pin_4
#define	HC595_OFFSET_RCC_APB	RCC_APB2Periph_GPIOA	/* 数据偏移 上升沿有效 */
#define	HC595_OFFSET_PIN_GROUP	GPIOA
#define	HC595_OFFSET_PIN		GPIO_Pin_5
#define	HC595_DATA_RCC_APB		RCC_APB2Periph_GPIOA	/* 数据 */
#define	HC595_DATA_PIN_GROUP	GPIOA
#define	HC595_DATA_PIN			GPIO_Pin_6

/* 四个点 */
#define	POINT_LTOP_RCC_APB		RCC_APB2Periph_GPIOA	
#define	POINT_LTOP_PIN_GROUP	GPIOA
#define	POINT_LTOP_PIN			GPIO_Pin_8
#define	POINT_LBOT_RCC_APB		RCC_APB2Periph_GPIOD
#define	POINT_LBOT_PIN_GROUP	GPIOD
#define	POINT_LBOT_PIN			GPIO_Pin_2
#define	POINT_RTOP_RCC_APB		RCC_APB2Periph_GPIOA	
#define	POINT_RTOP_PIN_GROUP	GPIOA
#define	POINT_RTOP_PIN			GPIO_Pin_1
#define	POINT_RBOT_RCC_APB		RCC_APB2Periph_GPIOA	
#define	POINT_RBOT_PIN_GROUP	GPIOA
#define	POINT_RBOT_PIN			GPIO_Pin_7

/* DS3231 IRQ source define */
#define DS3231_RCC_APB			RCC_APB2Periph_GPIOC
#define DS3231_RCC_AFIO			RCC_APB2Periph_AFIO	
#define DS3231_IRQN_CHN			EXTI0_IRQn
#define DS3231_IRQ_SRC_PORT		GPIO_PortSourceGPIOC	
#define DS3231_IRQ_SRC_PIN		GPIO_PinSource0	
#define DS3231_IRQ_PORT			GPIOC
#define DS3231_IRQ_PIN			GPIO_Pin_0	
#define DS3231_IRQ_LINE			EXTI_Line0	


typedef enum {
	OFF = 0,
	ON = 1,
} SWITCH_STATE_e;

void  bsp_init(void);
void bsp_set_hv_state(SWITCH_STATE_e state);
BitAction bsp_get_hv_state(void);

__inline void bsp_set_hv_state(SWITCH_STATE_e state)
{
	if(state == ON)
		HV_PIN_GROUP->BSRR = HV_PIN;
	else
		HV_PIN_GROUP->BRR = HV_PIN;
}

__inline BitAction bsp_get_hv_state(void)
{
	return ((HV_PIN_GROUP->ODR & HV_PIN != (u32)Bit_RESET) ? Bit_SET : Bit_RESET);
}

__inline void bsp_74hc595_set_OE(BitAction act)
{
	if(act == Bit_SET)
		HC595_OE_PIN_GROUP->BSRR = HC595_OE_PIN;
	else
		HC595_OE_PIN_GROUP->BRR = HC595_OE_PIN;
}

__inline void bsp_74hc595_set_ST_CP(BitAction act)
{
	if(act == Bit_SET)
		HC595_LOCK_PIN_GROUP->BSRR = HC595_LOCK_PIN;
	else
		HC595_LOCK_PIN_GROUP->BRR = HC595_LOCK_PIN;
}

__inline void bsp_74hc595_set_SH_CP(BitAction act)
{
	if(act == Bit_SET)
		HC595_OFFSET_PIN_GROUP->BSRR = HC595_OFFSET_PIN;
	else
		HC595_OFFSET_PIN_GROUP->BRR = HC595_OFFSET_PIN;
}

__inline void bsp_74hc595_set_DATA(BitAction act)
{
	if(act == Bit_SET)
		HC595_DATA_PIN_GROUP->BSRR = HC595_DATA_PIN;
	else
		HC595_DATA_PIN_GROUP->BRR = HC595_DATA_PIN;
}



#endif
