/* FreeRTOS includes. */
#include <string.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "os_inc.h"

#include "mid_cli.h"
#include "mid_th.h"
#include "mid_rtc.h"

#include "app_inc.h"

#include "bsp.h"

/*
 * Example: info
 */
build_var(info, "Device information.", 0);

/*
 * Example: clear
 */
build_var(clear, "Clear Terminal.", 0);

/*
 * Example: date
 */
build_var(date, "Read system current time.", 0);

/*
 * Example: sdate 16 11 27 7 22 48 50
 */
build_var(sdate, "Set system current time,format: sdate year[0,99] month[0,12] mday[1,31] wday[1-7] hour[0-23] min[0-59] sec[0-59]", 7);

/*
 * Example: th
 */
build_var(th, "Read the ambient temperature(C) and humidity(%).", 0);

/*
 * Example: led R 1
 */
build_var(led, "Turn ON/OFF led blink,format: led color[R,G,B] state[1:ON,0:OFF].", 2);

/*
 * Example: reboot
 */
build_var(reboot, "Reboot system.", 0);

/*
 * Example: lcd 1
 */
build_var(lcd, "Turn ON/OFF lcd display,format: lcd state[1:ON,0:OFF].", 1);

/*
 * Example: top
 */
build_var(top, "List all the tasks state.", 0);

/*
 * Example set: showtime 800 1930
 * get: showtime on off
 */
build_var(setlcd, "Time ON/OFF lcd display and led blink,format: setlcd on[800] off[1930]", 2);

static void app_cli_register(void)
{
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
	U8 index;
	const static char *dev_info[] =
	{
		" 	Dev:	STM32F103RCT6 \r\n",
		" 	Cpu:	ARM 32-bit Cortex-M3\r\n",
		" 	Freq: 	72 MHz max,1.25 DMIPS/MHz\r\n",
		" 	Mem:	256 Kbytes of Flash memory\r\n",
		" 	Ram:	64 Kbytes of SRAM\r\n",
		NULL
	};

	(void) help_info;
	configASSERT(dest);

	for(index = 0; dev_info[index] != NULL; index++)
	{
		strcat(dest, dev_info[index]);
	}

	return pdFALSE;
}

static BaseType_t clear_main(char *dest, argv_attribute argv, const char * const help_info)
{
	const static char *string = "\033[H\033[J";

	(void) help_info;
	configASSERT(dest);

	strcpy(dest, string);

	return pdFALSE;
}

static BaseType_t date_main( char *dest, argv_attribute argv, const char * const help_info)
{
	struct rtc_time tm;

	(void) help_info;
	configASSERT(dest);

	mid_rtc_get_time(&tm);
	sprintf(dest, "\t20%02d-%02d-%02d %d %02d:%02d:%02d\r\n", 
												tm.year,
												tm.mon,
												tm.mday,
												tm.wday,
												tm.hour,
												tm.min,
												tm.sec);
	
	return pdFALSE;
}

static BaseType_t sdate_main( char *dest, argv_attribute argv, const char * const help_info)
{
	struct rtc_time tm;

	(void) help_info;
	configASSERT(dest);

	tm.year = atoi(argv[1]);
	tm.mon = atoi(argv[2]);
	tm.mday = atoi(argv[3]);
	tm.wday = atoi(argv[4]);
	tm.hour = atoi(argv[5]);
	tm.min = atoi(argv[6]);
	tm.sec = atoi(argv[7]);
	if(!rtc_valid_tm(&tm))
	{
		mid_rtc_set_time(&tm);
		memset(&tm, 0, sizeof(struct rtc_time));
		mid_rtc_get_time(&tm);
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

	return pdFALSE;
}

static BaseType_t th_main( char *dest, argv_attribute argv, const char * const help_info)
{
	struct _mid_th_data_t th_data;
	
	(void) help_info;
	configASSERT(dest);

	if(mid_th_get_data(&th_data) == STATUS_NORMAL)
		sprintf(dest, "\tTemperature %.1f(C)  Humidity %.1f(%%)\r\n", th_data.temp, th_data.hum);
	else
		sprintf(dest, "\tError: read temperature and humidity timeout!\r\n");
	
	return pdFALSE;
}

static BaseType_t led_main( char *dest, argv_attribute argv, const char * const help_info)
{
	TaskHandle_t pled;
	char state, color;
	
	(void) help_info;
	configASSERT(dest);
	
	color = *argv[1];
	state = atoi(argv[2]);
	if((color != 'R' && color != 'G' && color != 'B') 
		|| (state != 1 && state != 0))
	{
		sprintf(dest, "\tCommand of led control Format incorrect,please try again.\r\n");
		return pdFALSE;
	}
		
	pled = main_get_task_handle(TASK_HANDLE_LED);
	switch(color)
	{
		case 'R': app_led_set_color(0); break;
		case 'G': app_led_set_color(1); break;
		case 'B': app_led_set_color(2); break;
		default: break;
	}
	
	if(state == 1)
		vTaskResume(pled);
	else
		vTaskSuspend(pled);
	sprintf(dest, "\tLed task is %s\r\n", state ? "working." : "stoped.");

	return pdFALSE;
}

static BaseType_t reboot_main( char *dest, argv_attribute argv, const char * const help_info)
{
	(void) help_info;
	configASSERT(dest);

	NVIC_SystemReset();
	
	return pdFALSE;
}

static BaseType_t lcd_main( char *dest, argv_attribute argv, const char * const help_info)
{
	char state;
	
	(void) help_info;
	configASSERT(dest);
	
	state = atoi(argv[1]);
	if((state != 1) && (state != 0))
	{
		sprintf(dest, "\tCommand of lcd control Format incorrect,please try again.\r\n");
		return pdFALSE;
	}
	
	if(state == 1)
	{
		app_display_on(NULL);
	}
	else
	{
		app_display_off(NULL);
	}
	
	sprintf(dest, "\tLcd display task is %s\r\n", state ? "working." : "stoped.");

	return pdFALSE;
}

static BaseType_t top_main( char *dest, argv_attribute argv, const char * const help_info)
{
	TaskHandle_t ptask;
	U8 list = 0;
	char string[15];
	
	(void) help_info;
	configASSERT(dest);

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
	
	return pdFALSE;
}

static BaseType_t setlcd_main( char *dest, argv_attribute argv, const char * const help_info)
{
	U16 on_hour, on_min, off_hour, off_min;
	U16 on, off;

	(void) help_info;
	configASSERT(dest);

	on = atoi(argv[1]);
	off = atoi(argv[2]);

	on_hour = on / 100;
	on_min = on % 100;
	off_hour = off / 100;
	off_min = off % 100;
	
	if(on_hour != 0
		|| on_min != 0
		|| off_hour != 0
		|| off_min != 0)
	{
		app_time_set_showtime(on, off);
		sprintf(dest, "\tSet show time success,current: ON-%02d:%02d  OFF-%02d:%02d\r\n", 
													on_hour,
													on_min,
													off_hour,
													off_min);
	}
	else
	{
		app_time_get_showtime(&on, &off);
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

	return pdFALSE;
}

void app_cli_init(U8 priority, char *t, TaskHandle_t *handle)
{
	mid_cli_init(400, priority, t, handle);

	app_cli_register();
}


