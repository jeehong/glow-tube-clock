#include "misc.h"
#include "bsp.h"
#include "i2c_bus.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "app_serial.h"
#include "app_led.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"

static void prvSetupHardware(void);
static void bsp_hv_io_init(void);
static void bsp_74hc595_io_init(void);
static void bsp_point_io_init(void);
static void bsp_ds3231_irq_init(void);


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



