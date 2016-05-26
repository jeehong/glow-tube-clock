#include "string.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_display.h"
#include "app_led.h"
#include "app_serial.h"


#include "bsp.h"
#include "main.h"

static void app_display_set_byte(unsigned char data);
static void app_display_show_data(void);
static void app_display_set_show(BitAction act);
static void app_display_set_data(char *pdata);
static void app_display_set_map(char *desc, char *src);
static void app_display_set_point(char src);

static void delay50ns(void)
{
	char ns = 5;
	
	while(ns--) ;
}

/* ��һ���ֽڴ����������� */
static void app_display_set_byte(unsigned char data)
{  
	unsigned char index;
	
	for(index = 0; index < 8; index++) 
	{
		if((data << index) & 0x80) 
			bsp_74hc595_set_DATA(Bit_SET);
		else 
			bsp_74hc595_set_DATA(Bit_RESET);
		bsp_74hc595_set_OFFSET(Bit_RESET);
		delay50ns();
		bsp_74hc595_set_OFFSET(Bit_SET);
	}
} 

/* �����������ʹ�ܵ�����ܽ� */
static void app_display_show_data(void)
{
	bsp_74hc595_set_LOCK(Bit_RESET);
	delay50ns();
	bsp_74hc595_set_LOCK(Bit_SET);	
}

/* �����������Ƿ���� 1:out */
static void app_display_set_show(BitAction act)
{
	if(act == Bit_SET)
		bsp_74hc595_set_OE(Bit_RESET);
	else
		bsp_74hc595_set_OE(Bit_SET);
}

static void app_display_set_data(char *pdata)
{
	unsigned char index;

	for(index = 0; index < 8; index++)
		app_display_set_byte(pdata[index]);
}

/*
 * desc:ָ���������������8�ֽ�
 * src:ָ�����ʾ��6���Թ������[0,9],������������Χʱ���϶�Ϊ����ʾ�κ�����
 * Сʱ�� Сʱ�� ���Ӹ� ���ӵ�  ���   ���
 * src[0] src[1] src[2] src[3] src[4] src[5]
 */
static void app_display_set_map(char *desc, char *src)
{
	unsigned char tube;		/* ��ǰ�����Ĺ������ */
	const unsigned char tube_num = TUBE_NUM - 1;	/* �������� */
	unsigned char calc;

	memset(desc, 0, sizeof(char) << 3);
	for(tube = 0; tube <= tube_num; tube++)
	{
		if((unsigned char)src[tube] <= 9)
		{
			calc = tube * 10 + src[tube_num - tube];
			desc[calc >> 3] = 0x01 << (calc % 8);
		}
		else		/* �������������ʱ����Ĭ��Ϊ����ʾ�κ����� */
		{}
	}
}

/*
 * src:ָ�����ʾ����������
 * ����λ    *    *    ��  ��  ��λð��
 *         0 *  1 �� 2 * 3 �� 
 *
 * ����λ    *    *    ��  ��  ��λð��
 *         0 *  1 �� 2 * 3 �� 
 * ����ʾ����ʾ
 */
static void app_display_set_point(char src)
{
	switch(src >> 4)
	{
		case 0:
			POINT_LBOT_PIN_GROUP->BRR = POINT_LBOT_PIN;
			POINT_LTOP_PIN_GROUP->BRR = POINT_LTOP_PIN;
			break;
		case 1:
			POINT_LBOT_PIN_GROUP->BSRR = POINT_LBOT_PIN;
			POINT_LTOP_PIN_GROUP->BRR = POINT_LTOP_PIN;
			break;
		case 2:
			POINT_LTOP_PIN_GROUP->BSRR = POINT_LTOP_PIN;
			POINT_LBOT_PIN_GROUP->BRR = POINT_LBOT_PIN;
			break;
		case 3:
			POINT_LBOT_PIN_GROUP->BSRR = POINT_LBOT_PIN;
			POINT_LTOP_PIN_GROUP->BSRR = POINT_LTOP_PIN;	
			break;
		default: 
			break;
	}	
	switch(src & 0x0f)
	{
		case 0:
			POINT_RBOT_PIN_GROUP->BRR = POINT_RBOT_PIN;
			POINT_RTOP_PIN_GROUP->BRR = POINT_RTOP_PIN;
			break;
		case 1:
			POINT_RBOT_PIN_GROUP->BSRR = POINT_RBOT_PIN;
			POINT_RTOP_PIN_GROUP->BRR = POINT_RTOP_PIN;
			break;
		case 2:
			POINT_RTOP_PIN_GROUP->BSRR = POINT_RTOP_PIN;
			POINT_RBOT_PIN_GROUP->BRR = POINT_RBOT_PIN;
			break;
		case 3:
			POINT_RTOP_PIN_GROUP->BSRR = POINT_RTOP_PIN;
			POINT_RBOT_PIN_GROUP->BSRR = POINT_RBOT_PIN;	
			break;
		default: 
			break;
	}	
}	

void app_dispaly_show_task(DISPLAY_RESOURCE_t *display)
{
	char map[8] = {0};
	char *src;
	
	bsp_set_hv_state(ON);
	app_display_set_show(Bit_RESET);
	src = &display->map[0];
	while(1)
	{
		xSemaphoreTake(display->xMutex, portMAX_DELAY);
		app_display_set_point(src[TUBE_NUM]);
		app_display_set_map(map, src);
		xSemaphoreGive(display->xMutex);
		app_display_set_data(map);
		portENTER_CRITICAL();
		app_display_show_data();
		portEXIT_CRITICAL();
		app_display_set_show(Bit_SET);
		vTaskDelay(mainDELAY_MS(100));
	}	
}

