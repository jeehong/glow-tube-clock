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

/* FreeRTOS includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

#include "bsp.h"

/* FreeRTOS+CLI includes. */
#include "main.h"
#include "cli_api.h" 
#include "app_ds3231.h"
#include "app_sht10.h"

static BaseType_t prvInfoCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvClearCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvDateCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvSetDateCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvTHCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvLedCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvRebootCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvDisplayCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvTaskCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);
static BaseType_t prvShowtimeCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString);

/*
 * Example: info
 */
static const CLI_Command_Definition_t xInfoCommand =
{
	"info",
	"Device information.\r\n",
	prvInfoCommand,
	0
};

/*
 * Example: clear
 */
static const CLI_Command_Definition_t xClearCommand =
{
	"clear",
	"Clear Terminal.\r\n",
	prvClearCommand,
	0
};


/*
 * Example: date
 */
static const CLI_Command_Definition_t xDateCommand =
{
	"date",
	"Read system current time.\r\n",
	prvDateCommand,
	0
};

/*
 * Example: sdate 16 11 27 7 22 48 50
 */
static const CLI_Command_Definition_t xSetDateCommand =
{
	"sdate",
	"Set system current time,format:[sdate year[0,99] month[0,12] mday[1,31] wday[1-7] hour[0-23] min[0-59] sec[0-59]]\r\n",
	prvSetDateCommand,
	7
};

/*
 * Example: th
 */
static const CLI_Command_Definition_t xTHCommand =
{
	"th",
	"Read the ambient temperature(C) and humidity(%).\r\n",
	prvTHCommand,
	0
};

/*
 * Example: led 0
 */
static const CLI_Command_Definition_t xLedCommand =
{
	"led",
	"Turn ON/OFF led blink,format:[led state(1:ON,0:OFF)].\r\n",
	prvLedCommand,
	1
};

/*
 * Example: reboot
 */
static const CLI_Command_Definition_t xRebootCommand =
{
	"reboot",
	"Reboot system.\r\n",
	prvRebootCommand,
	0
};

/*
 * Example: display 1
 */
static const CLI_Command_Definition_t xDisplayCommand =
{
	"display",
	"Turn ON/OFF lcd display,format:[display state(1:ON,0:OFF)].\r\n",
	prvDisplayCommand,
	1
};

/*
 * Example: task
 */
static const CLI_Command_Definition_t xTaskCommand =
{
	"top",
	"List all the tasks state.\r\n",
	prvTaskCommand,
	0
};

/*
 * Example set: showtime 800 1930
 * get: showtime on off
 */
static const CLI_Command_Definition_t xShowtimeCommand =
{
	"showtime",
	"Time ON/OFF lcd display and led blink,format:[showtime on(800) off(1930)]\r\n",
	prvShowtimeCommand,
	2
};


/*-----------------------------------------------------------*/

void vRegisterCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */
	FreeRTOS_CLIRegisterCommand( &xInfoCommand );
    FreeRTOS_CLIRegisterCommand( &xClearCommand );
	FreeRTOS_CLIRegisterCommand( &xDateCommand );
	FreeRTOS_CLIRegisterCommand( &xSetDateCommand );
	FreeRTOS_CLIRegisterCommand( &xTHCommand );
	FreeRTOS_CLIRegisterCommand( &xLedCommand );
	FreeRTOS_CLIRegisterCommand( &xRebootCommand );
	FreeRTOS_CLIRegisterCommand( &xDisplayCommand );
	FreeRTOS_CLIRegisterCommand( &xTaskCommand );
	FreeRTOS_CLIRegisterCommand( &xShowtimeCommand );
}

static BaseType_t prvInfoCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	unsigned char index;
	const static char *dev_info[] =
	{
		" 	Dev:	STM32F103RCT6 \r\n",
		" 	Cpu:	ARM 32-bit Cortex-M3\r\n",
		" 	Freq: 	72 MHz max,1.25 DMIPS/MHz\r\n",
		" 	Mem:	256 Kbytes of Flash memory\r\n",
		" 	Ram:	64 Kbytes of SRAM\r\n",
		NULL
	};

	( void ) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	/* Generate a table of task stats. */
    sprintf(pcWriteBuffer, "    %s", pHelpString);
	for(index = 0; dev_info[index] != NULL; index++)
	{
		strcat(pcWriteBuffer, dev_info[index]);
	}

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvClearCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	const static char *string = "\033[H\033[J";

	( void ) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	/* Generate a table of task stats. */
	strcpy(pcWriteBuffer, string);

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvDateCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	struct rtc_time tm;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	app_ds3231_read_time(&tm);
	/* Generate a table of task stats. */
	sprintf(pcWriteBuffer, "    20%02d-%02d-%02d %d %02d:%02d:%02d\r\n", 
												tm.year,
												tm.mon,
												tm.mday,
												tm.wday,
												tm.hour,
												tm.min,
												tm.sec);

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvSetDateCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	struct rtc_time tm;
	char string[10], year, mon, mday, wday, hour, min, sec;
	unsigned char result = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	result = sscanf(pcCommandInput, "%s %d %d %d %d %d %d %d", string, &year, &mon, &mday, &wday, &hour, &min, &sec);
	tm.year = year;
	tm.mon = mon;
	tm.mday = mday;
	tm.wday = wday;
	tm.hour = hour;
	tm.min = min;
	tm.sec = sec;	
	if((result == 8) && !rtc_valid_tm(&tm))
	{
		app_ds3231_set_time(&tm);
		memset(&tm, 0, sizeof(struct rtc_time));
		app_ds3231_read_time(&tm);
		/* Generate a table of task stats. */
		sprintf(pcWriteBuffer, "    Set time success,current: 20%d-%d-%d %d %d:%d:%d\r\n", 
													tm.year,
													tm.mon,
													tm.mday,
													tm.wday,
													tm.hour,
													tm.min,
													tm.sec);
	}
	else
	{
		strcpy(pcWriteBuffer, "	Format incorrect,please try again!\r\n");
	}
	


	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvTHCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	float temp, hum;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);


	/* Generate a table of task stats. */
	temp = app_sht10_get_info(TEMP);
	hum = app_sht10_get_info(HUM);
	sprintf(pcWriteBuffer, "    Temperature %.1f(C)  Humidity %.1f(%%)\r\n", temp, hum);

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvLedCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	TaskHandle_t pled;
	char string[10], state;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	
	/* Generate a table of task stats. */
	sscanf(pcCommandInput, "%s %d", string, &state);
	if((state != 1) && (state != 0))
	{
		sprintf(pcWriteBuffer, "    Command of led control Format incorrect,please try again.\r\n");
		return pdFALSE;
	}
		
	pled = main_get_task_handle(HD_LED);
	
	if(state == 1)
		vTaskResume(pled);
	else
		vTaskSuspend(pled);
	sprintf(pcWriteBuffer, "    Led task is %s\r\n", state ? "working." : "stoped.");

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvRebootCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	NVIC_SystemReset();
	
	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvDisplayCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	TaskHandle_t plcd;
	char string[10], state;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	
	/* Generate a table of task stats. */
	plcd = main_get_task_handle(HD_DISPLAY);
	sscanf(pcCommandInput, "%s %d", string, &state);
	
	if((state != 1) && (state != 0))
	{
		sprintf(pcWriteBuffer, "    Command of lcd control Format incorrect,please try again.\r\n");
		return pdFALSE;
	}
	
	if(state == 1)
	{
		bsp_set_hv_state(ON);
		app_display_set_show(Bit_SET);
		vTaskResume(plcd);
	}
	else
	{
		bsp_set_hv_state(OFF);
		app_display_set_show(Bit_RESET);
		vTaskSuspend(plcd);
	}
	
	sprintf(pcWriteBuffer, "    Lcd display task is %s\r\n", state ? "working." : "stoped.");

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvTaskCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	TaskHandle_t ptask;
	unsigned char list = 0;
	char string[15];
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	
	/* Generate a table of task stats. */
	sprintf(pcWriteBuffer, "	Priority	Name	State		Mem(B)\r\n");
	for(list = 0; main_get_task_handle(list) != NULL; list++)
	{
		ptask = main_get_task_handle(list);
		strcat(pcWriteBuffer, "	");
		sprintf(string, "%d", uxTaskPriorityGet(ptask));
		strcat(pcWriteBuffer, string);
		strcat(pcWriteBuffer, "		");
		strcat(pcWriteBuffer, pcTaskGetName(ptask));
		strcat(pcWriteBuffer, "	");
		switch(eTaskGetState(ptask))
		{
			case eRunning: strcat(pcWriteBuffer, "run"); break;
			case eReady: strcat(pcWriteBuffer, "ready"); break;
			case eBlocked: strcat(pcWriteBuffer, "block"); break;
			case eSuspended: strcat(pcWriteBuffer, "suspend"); break;
			case eDeleted: strcat(pcWriteBuffer, "delet"); break;
			case eInvalid: strcat(pcWriteBuffer, "invalid"); break;
			default: strcat(pcWriteBuffer, "NULL"); break;
		}
		strcat(pcWriteBuffer, "		");
		sprintf(string, "%d", uxTaskGetStackHighWaterMark(ptask));
		strcat(pcWriteBuffer, string);
		strcat(pcWriteBuffer, "\r\n");
	}	
	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t prvShowtimeCommand(const char * const pcCommandInput, char *pcWriteBuffer, size_t xWriteBufferLen, const char * const pHelpString)
{
	char string[10], on_hour, on_min, off_hour, off_min;
	short on, off;
	unsigned char result = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) pcCommandInput;
	( void ) pHelpString;
	( void ) xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	result = sscanf(pcCommandInput, "%s %d %d", string, &on, &off);

	on_hour = on / 100;
	on_min = on % 100;
	off_hour = off / 100;
	off_min = off % 100;
	if((result == 3) 
		&& on_hour <= 23
		&& on_min <= 59
		&& off_hour <= 23
		&& off_min <= 59)
	{
		app_ds3231_set_showtime(on, off);
		/* Generate a table of task stats. */
		sprintf(pcWriteBuffer, "    Set show time success,current: ON-%02d:%02d  OFF-%02d:%02d\r\n", 
													on_hour,
													on_min,
													off_hour,
													off_min);
	}
	else
	{
		app_ds3231_get_showtime(&on, &off);
		on_hour = on / 100;
		on_min = on % 100;
		off_hour = off / 100;
		off_min = off % 100;
		sprintf(pcWriteBuffer, "	Format incorrect,please try again! current: ON-%02d:%02d  OFF-%02d:%02d\r\n",
													on_hour,
													on_min,
													off_hour,
													off_min);
	}
	


	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}


