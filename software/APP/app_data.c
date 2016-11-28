#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_data.h"
#include "app_serial.h"
#include "main.h"

#include "stm32f10x_flash.h"

static DATA_MAP_t mData;
/*
 * 只有拿到该信号量，才有权更新flash数据
 */
static QueueHandle_t dataSync;


static void app_data_load_from_flash(DATA_MAP_t *pdata);

/*
 * 上电即读取flash信息
 */
void app_data_init(void)
{
	app_data_load_from_flash(&mData);
	dataSync = xSemaphoreCreateMutex();
	xSemaphoreTake(dataSync, portMAX_DELAY);
}

/*
 * 加载flash数据
 */
static void app_data_load_from_flash(DATA_MAP_t *pdata)
{
	unsigned short index, 
				length = sizeof(DATA_MAP_t) / sizeof(short), 
				*pbuff = (unsigned short *)pdata;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY)==1) ;
	for(index = 0; index < length; index++)
		*(unsigned short *)(pbuff + index) = *(unsigned short *)(DATA_START_ADDRESS + index * 2);
	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_Lock();	
}

static DATA_MAP_t *app_data_get_data(void)
{
	return &mData;
}

void app_data_read_showtime(unsigned short *on, unsigned short *off)
{
	DATA_MAP_t	*sData = NULL;
	sData = app_data_get_data();
	if (sData == NULL) 
		return;
	
	*on = sData->showon;
	*off = sData->showoff;
}

void app_data_write_showtime(unsigned short on, unsigned short off)
{
	mData.showon = on;
	mData.showoff = off;
	
	xSemaphoreGive(dataSync);
}

void app_data_store_task(GLOBAL_SOURCE_t *p_src)

{	
	const char unit = sizeof(int);
	unsigned short length = sizeof(DATA_MAP_t) / unit;
	unsigned char index;

	while(1)
	{
		xSemaphoreTake(dataSync, portMAX_DELAY);

		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		while(FLASH_GetFlagStatus(FLASH_FLAG_BSY)==1) ;
		
		FLASH_ErasePage(DATA_START_ADDRESS);
		while(FLASH_GetFlagStatus(FLASH_FLAG_BSY) == 1) ;
		taskENTER_CRITICAL();
		for(index = 0; index < length; index++)
		{
			FLASH_ProgramWord(DATA_START_ADDRESS + index * unit, *(unsigned int *)(&mData + index * unit));
		}
		taskEXIT_CRITICAL();
		FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
		FLASH_Lock();		
	}
}
