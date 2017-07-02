#ifndef __MAIN_H__
#define	__MAIN_H__

#include "queue.h"
#include "task.h"

#include "bsp.h"

/* 软件版本信息 */
#define PRJ_VER_MAJOR		1
#define PRJ_VER_MINOR		1
#define PRJ_VER_REVISION	0

#define mainDELAY_MS(ms)			( ( TickType_t ) ms / portTICK_PERIOD_MS )

/* Some useful definitions */
#undef FALSE
#define FALSE   ((int) 0)
#undef TRUE
#define TRUE    ((int) 1)
#undef SKIP
#define SKIP	((int) 2)

/* Macros for min/max.  */
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif


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

