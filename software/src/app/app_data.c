#include <stdio.h>
#include <string.h>

#include "os_inc.h"

#include "app_inc.h"

#include "stm32f10x_flash.h"

static DATA_MAP_t mData;
/*
 * ֻ���õ����ź���������Ȩ����flash����
 */
static QueueHandle_t dataSync;


static void app_data_load_from_flash(DATA_MAP_t *pdata);

/*
 * �ϵ缴��ȡflash��Ϣ
 */
void app_data_init(void)
{
	app_data_load_from_flash(&mData);
	dataSync = xSemaphoreCreateMutex();
	xSemaphoreTake(dataSync, portMAX_DELAY);
}

/*
 * ����flash����
 */
static void app_data_load_from_flash(DATA_MAP_t *pdata)
{
	U16 index, 
				length = sizeof(DATA_MAP_t) / sizeof(U16), 
				*pbuff = (U16 *)pdata;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	while(FLASH_GetFlagStatus(FLASH_FLAG_BSY)==1) ;
	for(index = 0; index < length; index++)
		*(U16 *)(pbuff + index) = *(U16 *)(DATA_START_ADDRESS + index * 2);
	
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_Lock();	
}

static DATA_MAP_t *app_data_get_data(void)
{
	return &mData;
}

void app_data_read_showtime(U16 *on, U16 *off)
{
	DATA_MAP_t	*sData = NULL;
	sData = app_data_get_data();
	if (sData == NULL) 
		return;
	
	*on = sData->showon;
	*off = sData->showoff;
}

void app_data_write_showtime(U16 on, U16 off)
{
	mData.showon = on;
	mData.showoff = off;
	
	xSemaphoreGive(dataSync);
}

void app_data_store_task(void *parame)

{	
	const U8 unit = sizeof(U8);
	U16 length = sizeof(DATA_MAP_t) / unit;
	U8 index;

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
			FLASH_ProgramWord(DATA_START_ADDRESS + index * unit, *(U32 *)(&mData + index * unit));
		}
		taskEXIT_CRITICAL();
		FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
		FLASH_Lock();		
	}
}
