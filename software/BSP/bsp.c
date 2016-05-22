#include "misc.h"
#include "bsp.h"
#include "i2c_bus.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "app_serial.h"
#include "app_led.h"

/* HV */		
#define	HV_RCC_APB			RCC_APB2Periph_GPIOC
#define	HV_PIN_GROUP		GPIOC
#define	HV_PIN				GPIO_Pin_8

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



static void prvSetupHardware(void);
static void bsp_hv_io_init(void);
static void bsp_74hc595_io_init(void);


void bsp_init(void)
{
	prvSetupHardware();
	serial_init(mainCOM_BAUD_RATE);
	i2c_bus_init();
	app_led_init();
	bsp_hv_io_init();
	bsp_74hc595_io_init();
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

void bsp_set_hv_state(SWITCH_STATE_e state)
{
	if(state == ON)
		HV_PIN_GROUP->BSRR = HV_PIN;
	else
		HV_PIN_GROUP->BRR = HV_PIN;
}

BitAction bsp_get_hv_state(void)
{
	return ((HV_PIN_GROUP->ODR & HV_PIN != (u32)Bit_RESET) ? Bit_SET : Bit_RESET);
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

__inline void bsp_74hc595_set_OE(BitAction act)
{
	if(act == Bit_SET)
		HC595_OE_PIN_GROUP->BSRR = HC595_OE_PIN;
	else
		HC595_OE_PIN_GROUP->BRR = HC595_OE_PIN;
}

__inline void bsp_74hc595_set_LOCK(BitAction act)
{
	if(act == Bit_SET)
		HC595_LOCK_PIN_GROUP->BSRR = HC595_LOCK_PIN;
	else
		HC595_LOCK_PIN_GROUP->BRR = HC595_LOCK_PIN;
}

__inline void bsp_74hc595_set_OFFSET(BitAction act)
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




