#ifndef __MAIN_H__
#define	__MAIN_H__


#include "queue.h"


typedef struct {
	QueueHandle_t xQueue;
	char map[7];		/* 前6个是显示数字内容，最后一个是四个点的显示内容 */
} DISPLAY_RESOURCE_t;

#endif

