#ifndef __MAIN_H__
#define	__MAIN_H__


#include "queue.h"
#include "task.h"

#include "bsp.h"

#define mainDELAY_MS(ms)			( ( TickType_t ) ms / portTICK_PERIOD_MS )

enum TASK_HANDLE_e 
{
	HD_DISPLAY = 0,
	HD_SHT10,
	HD_DS3131,
	HD_BUZ,
	HD_LED,
	HD_ALL,
};

/* ��ǰ��Ȩ��ʹ��display�ĳ�Ա */
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
	char map[7];		/* ǰ6������ʾ�������ݣ����һ�����ĸ������ʾ���� */
	TaskHandle_t *ptaskHandle;
} GLOBAL_SOURCE_t;

TaskHandle_t main_get_task_handle(unsigned char id);

#endif

