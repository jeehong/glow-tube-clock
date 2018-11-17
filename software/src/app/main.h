#ifndef __MAIN_H__
#define	__MAIN_H__

#include "common_type.h"
#include "os_inc.h"

#include "bsp.h"

/* software version of project */
#define PRJ_VER_MAJOR		2
#define PRJ_VER_MINOR		0
#define PRJ_VER_REVISION	0

#define mainDELAY_MS(ms)			( ( TickType_t ) ms / portTICK_PERIOD_MS )


enum TASK_HANDLE_e 
{
	TASK_HANDLE_CLI = 0,
	TASK_HANDLE_DISPLAY,
	TASK_HANDLE_SHT10,
	TASK_HANDLE_DS3231,
	TASK_HANDLE_BUZ,
	TASK_HANDLE_LED,
	TASK_HANDLE_DATA,
	TASK_HANDLE_ALL,
};

TaskHandle_t main_get_task_handle(unsigned char id);

#endif

