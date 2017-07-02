/* FreeRTOS includes. */
#include <string.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+CLI includes. */
#include "mid_cli.h"
#include "hal_cli.h"
#include "main.h"
#include "app_ds3231.h"
#include "app_sht10.h"
#include "app_display.h"
#include "bsp.h"

/*
 * Example: info
 */
build_var(info, "Device information.\r\n", 0);

/*
 * Example: clear
 */
build_var(clear, "Clear Terminal.\r\n", 0);

/*
 * Example: date
 */
build_var(date, "Read system current time.\r\n", 0);

/*
 * Example: sdate 16 11 27 7 22 48 50
 */
build_var(sdate, "Set system current time,format:[sdate year[0,99] month[0,12] mday[1,31] wday[1-7] hour[0-23] min[0-59] sec[0-59]]\r\n", 7);

/*
 * Example: th
 */
build_var(th, "Read the ambient temperature(C) and humidity(%).\r\n", 0);

/*
 * Example: led 0
 */
build_var(led, "Turn ON/OFF led blink,format:[led state(1:ON,0:OFF)].\r\n", 1);

/*
 * Example: reboot
 */
build_var(reboot, "Reboot system.\r\n", 0);

/*
 * Example: display 1
 */
build_var(lcd, "Turn ON/OFF lcd display,format:[lcd state(1:ON,0:OFF)].\r\n", 1);

/*
 * Example: task
 */
build_var(top, "List all the tasks state.\r\n", 0);

/*
 * Example set: showtime 800 1930
 * get: showtime on off
 */
build_var(setlcd, "Time ON/OFF lcd display and led blink,format:[setlcd on(800) off(1930)]\r\n", 2);

static void app_cli_register(void)
{
	/* Register all the command line commands defined immediately above. */
	mid_cli_register(&info);
	mid_cli_register(&clear);
	mid_cli_register(&date);
	mid_cli_register(&sdate);
	mid_cli_register(&th);
	mid_cli_register(&led);
	mid_cli_register(&reboot);
	mid_cli_register(&lcd);
	mid_cli_register(&top);
	mid_cli_register(&setlcd);
}

static BaseType_t info_main( char *dest, argv_attribute argv, const char * const help_info)
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

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	/* Generate a table of task stats. */
    sprintf(dest, "    %s", help_info);
	for(index = 0; dev_info[index] != NULL; index++)
	{
		strcat(dest, dev_info[index]);
	}

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t clear_main( char *dest, argv_attribute argv, const char * const help_info)
{
	const static char *string = "\033[H\033[J";

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	/* Generate a table of task stats. */
	strcpy(dest, string);

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t date_main( char *dest, argv_attribute argv, const char * const help_info)
{
	struct rtc_time tm;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	app_ds3231_read_time(&tm);
	/* Generate a table of task stats. */
	sprintf(dest, "\t20%02d-%02d-%02d %d %02d:%02d:%02d\r\n", 
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

static BaseType_t sdate_main( char *dest, argv_attribute argv, const char * const help_info)
{
	struct rtc_time tm;
	unsigned char result = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	tm.year = atoi(argv[1]);
	tm.mon = atoi(argv[2]);
	tm.mday = atoi(argv[3]);
	tm.wday = atoi(argv[4]);
	tm.hour = atoi(argv[5]);
	tm.min = atoi(argv[6]);
	tm.sec = atoi(argv[7]);
	if((result == 8) && !rtc_valid_tm(&tm))
	{
		app_ds3231_set_time(&tm);
		memset(&tm, 0, sizeof(struct rtc_time));
		app_ds3231_read_time(&tm);
		/* Generate a table of task stats. */
		sprintf(dest, "\tSet time success,current: 20%d-%d-%d %d %d:%d:%d\r\n", 
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
		strcpy(dest, "	Format incorrect,please try again!\r\n");
	}
	


	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t th_main( char *dest, argv_attribute argv, const char * const help_info)
{
	float temp, hum;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	/* Generate a table of task stats. */
	temp = app_sht10_get_data(TEMP);
	hum = app_sht10_get_data(HUM);
	sprintf(dest, "\tTemperature %.1f(C)  Humidity %.1f(%%)\r\n", temp, hum);

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t led_main( char *dest, argv_attribute argv, const char * const help_info)
{
	TaskHandle_t pled;
	char state;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);
	
	/* Generate a table of task stats. */
	state = atoi(argv[1]);
	if((state != 1) && (state != 0))
	{
		sprintf(dest, "\tCommand of led control Format incorrect,please try again.\r\n");
		return pdFALSE;
	}
		
	pled = main_get_task_handle(TASK_HANDLE_LED);
	
	if(state == 1)
		vTaskResume(pled);
	else
		vTaskSuspend(pled);
	sprintf(dest, "\tLed task is %s\r\n", state ? "working." : "stoped.");

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t reboot_main( char *dest, argv_attribute argv, const char * const help_info)
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	NVIC_SystemReset();
	
	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t lcd_main( char *dest, argv_attribute argv, const char * const help_info)
{
	TaskHandle_t plcd;
	char state;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);
	
	/* Generate a table of task stats. */
	plcd = main_get_task_handle(TASK_HANDLE_DISPLAY);
	state = atoi(argv[1]);
	if((state != 1) && (state != 0))
	{
		sprintf(dest, "\tCommand of lcd control Format incorrect,please try again.\r\n");
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
	
	sprintf(dest, "\tLcd display task is %s\r\n", state ? "working." : "stoped.");

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t top_main( char *dest, argv_attribute argv, const char * const help_info)
{
	TaskHandle_t ptask;
	unsigned char list = 0;
	char string[15];
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	
	/* Generate a table of task stats. */
	sprintf(dest, "\tPri\tName\tState\tMem(B)\r\n");
	
	for(list = 0; list < TASK_HANDLE_ALL; list++)
	{
		ptask = main_get_task_handle(list);
		strcat(dest, "\t");
		sprintf(string, "%d", uxTaskPriorityGet(ptask));
		strcat(dest, string);
		strcat(dest, "\t");
		strcat(dest, pcTaskGetName(ptask));
		strcat(dest, "\t");
		switch(eTaskGetState(ptask))
		{
			case eRunning: strcat(dest, "run"); break;
			case eReady: strcat(dest, "ready"); break;
			case eBlocked: strcat(dest, "block"); break;
			case eSuspended: strcat(dest, "suspend"); break;
			case eDeleted: strcat(dest, "delet"); break;
			case eInvalid: strcat(dest, "invalid"); break;
			default: strcat(dest, "NULL"); break;
		}
		strcat(dest, "\t");
		sprintf(string, "%d", uxTaskGetStackHighWaterMark(ptask));
		strcat(dest, string);
		strcat(dest, "\r\n");
	}	
	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

static BaseType_t setlcd_main( char *dest, argv_attribute argv, const char * const help_info)
{
	char on_hour, on_min, off_hour, off_min;
	short on, off;
	unsigned char result = 0;
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	(void) help_info;
	configASSERT(dest);

	on = atoi(argv[1]);
	off = atoi(argv[2]);

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
		sprintf(dest, "\tSet show time success,current: ON-%02d:%02d  OFF-%02d:%02d\r\n", 
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
		sprintf(dest, "\tFormat incorrect,please try again! current: ON-%02d:%02d  OFF-%02d:%02d\r\n",
													on_hour,
													on_min,
													off_hour,
													off_min);
	}
	


	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

void app_cli_init(u8 priority, char *t, TaskHandle_t *handle)
{
	mid_cli_init(400, priority, t, handle);

	/* Register commands with the FreeRTOS+CLI command interpreter. */
	app_cli_register();
}


