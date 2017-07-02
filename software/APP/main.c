/*
    FreeRTOS V9.0.0rc2 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Fast Interrupt Test" - A high frequency periodic interrupt is generated
 * using a free running timer to demonstrate the use of the
 * configKERNEL_INTERRUPT_PRIORITY configuration constant.  The interrupt
 * service routine measures the number of processor clocks that occur between
 * each interrupt - and in so doing measures the jitter in the interrupt timing.
 * The maximum measured jitter time is latched in the ulMaxJitter variable, and
 * displayed on the LCD by the 'Check' task as described below.  The
 * fast interrupt is configured and handled in the timertest.c source file.
 *
 * "LCD" task - the LCD task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the LCD send the message on a queue to the LCD task instead of
 * accessing the LCD themselves.  The LCD task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" task -  This only executes every five seconds but has the highest
 * priority so is guaranteed to get processor time.  Its main function is to
 * check that all the standard demo tasks are still operational.  Should any
 * unexpected behaviour within a demo task be discovered the 'check' task will
 * write an error to the LCD (via the LCD task).  If all the demo tasks are
 * executing with their expected behaviour then the check task writes PASS
 * along with the max jitter time to the LCD (again via the LCD task), as
 * described above.
 *
 */

/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Library includes. */
#include "stm32f10x.h"

#include "netconfig.h"

#include "app_cli.h"

#include "bsp.h"
#include "main.h"

#include "app_led.h"
#include "app_serial.h"
#include "app_display.h"
#include "app_sht10.h"
#include "app_ds3231.h"
#include "app_buz.h"
#include "app_data.h"

#include "LwIPEntry.h"

/* The check task uses the sprintf function so requires a little more stack. */
#define mainLED_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 50 )
	
static TaskHandle_t task_handle[TASK_HANDLE_ALL];

 TaskHandle_t main_get_task_handle(unsigned char id)
{
	return task_handle[id];
}


/* The time between cycles of the 'check' task. */
int main( void )
{
	static char cmd_prefix[20];
	bsp_init();
	dbg_string("------Glow tube clock!------\r\n");
	
	/* ≥ı ºªØLwIP */
	//vlwIPInit();
	//LwIP_Init();

	sprintf(cmd_prefix, "%s-%d.%d.%d ", "clock", PRJ_VER_MAJOR, PRJ_VER_MINOR, PRJ_VER_REVISION);
	app_cli_init(tskIDLE_PRIORITY + 1, cmd_prefix, &task_handle[TASK_HANDLE_CLI]);

	//sys_thread_new("web_server", LwIPEntry, ( void * )NULL, 250, 5);
	xTaskCreate((pdTASK_CODE)app_display_task, "display", 280, NULL, 3, &task_handle[TASK_HANDLE_DISPLAY]);
	xTaskCreate((pdTASK_CODE)app_sht10_task, "sht10", 280, NULL, 4, &task_handle[TASK_HANDLE_SHT10]);
	xTaskCreate((pdTASK_CODE)app_ds3231_task, "ds3231", 280, NULL, 3, &task_handle[TASK_HANDLE_DS3231]);
    xTaskCreate((pdTASK_CODE)app_buz_task, "buz", 290, NULL, 4, &task_handle[TASK_HANDLE_BUZ]);
	xTaskCreate((pdTASK_CODE)app_led_task_blink, "led", 200, NULL, 4, &task_handle[TASK_HANDLE_LED]);
	xTaskCreate((pdTASK_CODE)app_data_store_task, "data", 200, NULL, 4, &task_handle[TASK_HANDLE_DATA]);
		
	/* Start the scheduler. */
	vTaskStartScheduler();

	return 0;
}




