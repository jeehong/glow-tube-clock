#ifndef __MAIN_H__
#define	__MAIN_H__


#include "queue.h"
#include "task.h"

#include "bsp.h"

#define mainDELAY_MS(ms)			( ( TickType_t ) ms / portTICK_PERIOD_MS )

/* 当前有权限使用display的成员 */
typedef enum {
    SHT_ACT = 0,
    DS3231_ACT,
    FREE_ACT,
} ACTIVE_MEMBER_e;  

typedef struct {
	QueueHandle_t xDisplay;
    QueueHandle_t xBuzzer;
    ACTIVE_MEMBER_e flag;
    SWITCH_STATE_e hv;
    char buz[2];
	char map[7];		/* 前6个是显示数字内容，最后一个是四个点的显示内容 */
} GLOBAL_SOURCE_t;

TaskHandle_t main_get_task_handle(unsigned char id);

#endif

