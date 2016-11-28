#include "misc.h"
#include "bsp.h"
#include "i2c_bus.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "app_serial.h"
#include "app_led.h"
#include "app_data.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_flash.h"
#include "misc.h"

static void prvSetupHardware(void);
static void bsp_hv_io_init(void);
static void bsp_74hc595_io_init(void);
static void bsp_point_io_init(void);
static void bsp_ds3231_irq_init(void);
static void bsp_TIM1_CH4_pwm_init(void); 
static void bsp_TIM1_CH4_io_init(void); 


void bsp_init(void)
{
	prvSetupHardware();
	serial_init(mainCOM_BAUD_RATE);
	i2c_bus_init();
	app_led_init();
	bsp_hv_io_init();
	bsp_74hc595_io_init();
	bsp_point_io_init();
	bsp_ds3231_irq_init();
    bsp_TIM1_CH4_io_init();
    bsp_TIM1_CH4_pwm_init();
	app_data_init();
}

static void prvSetupHardware(void)
{
	/* Start with the clocks in their expected state. */
	RCC_DeInit();

	/* Enable HSE (high speed external clock). */
	RCC_HSEConfig( RCC_HSE_ON );

	/* Wait till HSE is ready. */
	while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
	{
	}

	/* 2 wait states required on the flash. */
	*( ( unsigned long * ) 0x40022000 ) = 0x02;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	/* PCLK2 = HCLK */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

	/* PCLK1 = HCLK/2 */
	RCC_PCLK1Config( RCC_HCLK_Div2 );

	/* PLLCLK = 8MHz * 9 = 72 MHz. */
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

	/* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

	/* Wait till PLL is used as system clock source. */
	while( RCC_GetSYSCLKSource() != 0x08 )
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	/* RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE ); */

	/* SPI2 Periph clock enable */
	/* RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE ); */


	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

static void bsp_hv_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(HV_RCC_APB, ENABLE); 													   
	GPIO_InitStructure.GPIO_Pin = HV_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(HV_PIN_GROUP, &GPIO_InitStructure);			  
	GPIO_ResetBits(HV_PIN_GROUP, HV_PIN);
}



static void bsp_74hc595_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(HC595_LOCK_RCC_APB | HC595_OE_RCC_APB, ENABLE); 													   
	GPIO_InitStructure.GPIO_Pin = HC595_LOCK_PIN | HC595_OFFSET_PIN | HC595_DATA_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(HC595_LOCK_PIN_GROUP, &GPIO_InitStructure);			  
	

	GPIO_InitStructure.GPIO_Pin = HC595_OE_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(HC595_OE_PIN_GROUP, &GPIO_InitStructure); 
	GPIO_SetBits(HC595_OE_PIN_GROUP, HC595_OE_PIN);	 /* 初始化阶段，74HC595输出失能 */
}

static void bsp_point_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(POINT_LTOP_RCC_APB | POINT_LBOT_RCC_APB | POINT_RTOP_RCC_APB | POINT_RBOT_RCC_APB, ENABLE);													   
	GPIO_InitStructure.GPIO_Pin = POINT_LTOP_PIN | POINT_RTOP_PIN | POINT_RBOT_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(POINT_LTOP_PIN_GROUP, &GPIO_InitStructure);			  
	

	GPIO_InitStructure.GPIO_Pin = POINT_LBOT_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(POINT_LBOT_PIN_GROUP, &GPIO_InitStructure); 
	GPIO_ResetBits(POINT_LTOP_PIN_GROUP, POINT_LTOP_PIN | POINT_RTOP_PIN | POINT_RBOT_PIN);  
	GPIO_ResetBits(POINT_LBOT_PIN_GROUP, POINT_LBOT_PIN);  
}

static void bsp_ds3231_irq_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* config the extiline clock and AFIO clock */
	RCC_APB2PeriphClockCmd(DS3231_RCC_APB | DS3231_RCC_AFIO, ENABLE);
												
	/* config the NVIC */
	/* Configure one bit for preemption priority */
	NVIC_InitStructure.NVIC_IRQChannel = DS3231_IRQN_CHN;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	/* EXTI line gpio config*/	
	GPIO_InitStructure.GPIO_Pin = DS3231_IRQ_PIN;       
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(DS3231_IRQ_PORT, &GPIO_InitStructure);

	/* EXTI line mode config */
	GPIO_EXTILineConfig(DS3231_IRQ_SRC_PORT, DS3231_IRQ_SRC_PIN); 
	EXTI_InitStructure.EXTI_Line = DS3231_IRQ_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	
}


static void bsp_TIM1_CH4_io_init(void) 
{ 
    GPIO_InitTypeDef GPIO_InitStructure; 

	/* TIM1 clock enable */
	/* PCLK1 经过2倍频后座位TIM3的时钟源等于72MHz */
    /* GPIOA and GPIOB clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); 

    /* 
     * Config PA12 为浮空输入，
     * PA12已经连接到PA11，引用PA11上的TIM1_CH4的资源，
     * 为避免冲突，设置PA12为浮空输入 
     */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 


    
    /*GPIOA Configuration: TIM3 channel 4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		    /* 复用推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}  


static void bsp_TIM1_CH4_pwm_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	u16 CCR_Val = 1000; 

    /* -----------------------------------------------------------------------
    TIM1 Configuration: generate 4 PWM signals with 4 different duty cycles:
    TIM1CLK = 72 MHz, Prescaler = 0x0, TIM1 counter clock = 72 MHz
    TIM1 ARR Register = 999 => TIM1 Frequency = TIM1 counter clock/(ARR + 1)
    TIM1 Frequency = 72 KHz.
    TIM1 Channel4 duty cycle = (TIM1_CCR1 / TIM1_ARR) * 100 = 50%
	----------------------------------------------------------------------- */

	/* Time base configuration */		 
	TIM_TimeBaseStructure.TIM_Period = 48000;             /* 从0计数至999，即1000为一个定时周期 */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;	        /* 不设置预分频，即72MHz */
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	    /* 设置时钟分频系数:不分频 */
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     /* 向上计数 */
	
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	/* PWM1 Mode configuration: Channel4 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	            /* PWM mode1 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
	TIM_OCInitStructure.TIM_Pulse = CCR_Val;	                    /* 设置跳变值，当计数器累加到该值，电平跳变 */
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       /* 小于跳变值，为高电平 */
	
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);	
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM1, ENABLE);			 /* 使能TIM1重载寄存器ARR */
	
	/* TIM1 enable counter */
	TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, DISABLE);
}

