#ifndef __MAIN_H__
#define	__MAIN_H__


#include "queue.h"

#define mainDELAY_MS(ms)			( ( TickType_t ) ms / portTICK_PERIOD_MS )


typedef struct {
	QueueHandle_t xMutex;
    QueueHandle_t xBuzzer;
    char buz[2];
	char map[7];		/* ǰ6������ʾ�������ݣ����һ�����ĸ������ʾ���� */
} GLOBAL_SOURCE_t;

#endif

