#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_inc.h"

#include "common_type.h"

#include "stm32f10x_usart.h"

#include "hal_uart.h"


#define comSTACK_SIZE				configMINIMAL_STACK_SIZE
#define comTX_LED_OFFSET			( 0 )
#define comRX_LED_OFFSET			( 1 )
#define comTOTAL_PERMISSIBLE_ERRORS ( 2 )

/* The Tx task will transmit the sequence of characters at a pseudo random
interval.  This is the maximum and minimum block time between sends. */
#define comTX_MAX_BLOCK_TIME		( ( TickType_t ) 0x96 )
#define comTX_MIN_BLOCK_TIME		( ( TickType_t ) 0x32 )
#define comOFFSET_TIME				( ( TickType_t ) 3 )

/* We should find that each character can be queued for Tx immediately and we
don't have to block to send. */
#define comNO_BLOCK					( ( TickType_t ) 0 )

/* The Rx task will block on the Rx queue for a long period. */
#define comRX_BLOCK_TIME			( ( TickType_t ) 0xffff )

/* The sequence transmitted is from comFIRST_BYTE to and including comLAST_BYTE. */
#define comFIRST_BYTE				( ' ' )
#define comLAST_BYTE				( '~' )

#define comINITIAL_RX_COUNT_VALUE	( 0 )


/* The queue used to hold received characters. */
static QueueHandle_t xRxedChars; 
static QueueHandle_t xCharsForTx;

xComPortHandle hal_uart_init(U32 ulWantedBaud)
{
	xComPortHandle xReturn;
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStr;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Create the queues used to hold Rx/Tx characters. */
	xRxedChars = xQueueCreate(configCOMMAND_INT_MAX_OUTPUT_SIZE, (unsigned portBASE_TYPE)sizeof(S8)); 
	xCharsForTx = xQueueCreate(configCOMMAND_INT_MAX_OUTPUT_SIZE, (unsigned portBASE_TYPE)sizeof(S8));
	/* If the queue/semaphore was created correctly then setup the serial port
	hardware. */
	if(( xRxedChars != serINVALID_QUEUE ) && ( xCharsForTx != serINVALID_QUEUE ) )
	{
		/* Enable USART1 clock */
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE );	

		/* Configure USART1 Rx (PA10) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init( GPIOA, &GPIO_InitStructure );
		
		/* Configure USART1 Tx (PA9) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init( GPIOA, &GPIO_InitStructure );

		USART_InitStructure.USART_BaudRate = ulWantedBaud;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No ;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init( USART1, &USART_InitStructure );
		
		USART_ClockInitStr.USART_Clock = USART_Clock_Disable;
		USART_ClockInitStr.USART_CPOL = USART_CPOL_Low;
		USART_ClockInitStr.USART_CPHA = USART_CPHA_2Edge;
		USART_ClockInitStr.USART_LastBit = USART_LastBit_Disable;
		USART_ClockInit(USART1, &USART_ClockInitStr );
		
		
		USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );
		
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init( &NVIC_InitStructure );
		
		USART_Cmd( USART1, ENABLE );		
	}
	else
	{
		xReturn = ( xComPortHandle ) 0;
	}

	/* This demo file only supports a single port but we have to return
	something to comply with the standard demo header file. */
	return xReturn;
}

signed portBASE_TYPE hal_uart_get_char( xComPortHandle pxPort, char *pcRxedChar, TickType_t xBlockTime )
{
	( void ) pxPort;

	if( xQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
		return pdTRUE;
	else
		return pdFALSE;
}
/*-----------------------------------------------------------*/

void hal_uart_put_string( xComPortHandle pxPort, const char * const pcString, U16 usStringLength )
{
	u16 index;

	for(index = 0; index < usStringLength; index ++)
	{
		hal_uart_put_char( pxPort, pcString[index], serNO_BLOCK );
	}
}

signed portBASE_TYPE hal_uart_put_char( xComPortHandle pxPort, char cOutChar, TickType_t xBlockTime )
{
	signed portBASE_TYPE xReturn;

	if( xQueueSend( xCharsForTx, &cOutChar, xBlockTime ) == pdPASS )
	{
		xReturn = pdPASS;
		USART_ITConfig( USART1, USART_IT_TXE, ENABLE );
	}
	else
	{
		xReturn = pdFAIL;
	}

	return xReturn;
}

void USART1_IRQHandler( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	S8 cChar;

	if( USART_GetITStatus( USART1, USART_IT_TXE ) == SET )
	{
		/* The interrupt was caused by the THR becoming empty.  Are there any
		more characters to transmit? */
		while(xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE)
		{
			/* A character was retrieved from the queue so can be sent to the
			THR now. */
			USART_SendData( USART1, cChar );	
            while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
                ;
		}
		USART_ClearITPendingBit(USART1, USART_IT_TXE);
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);				
	}
	
	if( USART_GetITStatus( USART1, USART_IT_RXNE ) == SET )
	{
		cChar = USART_ReceiveData( USART1 );
		xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	} 	
	
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

