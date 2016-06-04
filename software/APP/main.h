#ifndef __MAIN_H__
#define	__MAIN_H__


#include "queue.h"

#define mainDELAY_MS(ms)			( ( TickType_t ) ms / portTICK_PERIOD_MS )


typedef struct {
	QueueHandle_t xMutex;
    QueueHandle_t xBuzzer;
    char buz[2];
	char map[7];		/* 前6个是显示数字内容，最后一个是四个点的显示内容 */
} GLOBAL_SOURCE_t;

#endif

