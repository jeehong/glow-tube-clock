#ifndef __MAIN_H__
#define	__MAIN_H__


#include "queue.h"


typedef struct {
	QueueHandle_t xQueue;
	char map[7];		/* ǰ6������ʾ�������ݣ����һ�����ĸ������ʾ���� */
} DISPLAY_RESOURCE_t;

#endif

