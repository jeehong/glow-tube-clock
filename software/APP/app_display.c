#include "string.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_display.h"
#include "app_led.h"
#include "app_serial.h"


#include "bsp.h"
#include "main.h"

#define	 	TUBE_NUM		6
#define		CHIP595_NUM		8

static void app_display_write_byte(unsigned char data);
static void app_display_show_data(void);
static void app_display_write_data(const char *pdata);
static void app_display_calc_map(char *desc, const char *src);
static void app_display_set_point(char src);

static char start_dis = 0;
/*
 * 实际测试1us
 */
static void delay50ns(void)
{
	char ns = 10;
	
	while(ns--) ;
}

/* 将一个字节传输至锁存器 */
static void app_display_write_byte(unsigned char data)
{  
	unsigned char index;
	
	for(index = 0; index < CHIP595_NUM; index++) 
	{
	    bsp_74hc595_set_SH_CP(Bit_RESET);
		if((data << index) & 0x80) 
			bsp_74hc595_set_DATA(Bit_SET);
		else 
			bsp_74hc595_set_DATA(Bit_RESET);
		delay50ns();	
		bsp_74hc595_set_SH_CP(Bit_SET);
		delay50ns();
	}
} 

/* 将传入的数据使能到输出管脚 */
static void app_display_show_data(void)
{
	bsp_74hc595_set_ST_CP(Bit_SET);	
	delay50ns();
	bsp_74hc595_set_ST_CP(Bit_RESET);
}

/* 设置锁存器是否输出 1:out */
void app_display_set_show(BitAction act)
{
	if(act == Bit_SET)
	{
		start_dis = 0;
		bsp_74hc595_set_OE(Bit_RESET);
	}
	else
		bsp_74hc595_set_OE(Bit_SET);
}

static void app_display_write_data(const char *pdata)
{
	unsigned char index;

	taskENTER_CRITICAL();
	for(index = CHIP595_NUM; index > 0; index--)
		app_display_write_byte(pdata[index - 1]);

	app_display_show_data();
	taskEXIT_CRITICAL();
}

/*
 * desc:指向向锁存器输入的8字节
 * src:指向待显示的6个辉光管数字[0,9],当超出给定范围时，认定为不显示任何内容
 * 小时高 小时低 分钟高 分钟低  秒高   秒低
 * src[0] src[1] src[2] src[3] src[4] src[5]
 */
static void app_display_calc_map(char *desc, const char *src)
{
	unsigned char tube;		/* 当前操作的管子序号 */
	const unsigned char tube_num = TUBE_NUM - 1;	/* 管子总数 */
	unsigned char calc;

	memset(desc, 0, CHIP595_NUM);
	for(tube = 0; tube <= tube_num; tube++)
	{
		if((unsigned char)src[tube] <= 9)
		{
			calc = tube * 10 + src[tube_num - tube];
			desc[calc >> 3] |= 0x01 << (calc % 8);
		}
		else		/* 当满足这个条件时，则默认为不显示任何内容 */
		{}
	}
}

/*
 * src:指向待显示的亮点类型
 * 高四位    *    *    ·  ·  高位冒号
 *         0 *  1 · 2 * 3 · 
 *
 * 低四位    *    *    ·  ·  低位冒号
 *         0 *  1 · 2 * 3 · 
 * ※表示不显示
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

void app_display_task(GLOBAL_SOURCE_t *p_src)
{
	char map[CHIP595_NUM] = {10};
	char *src;
	//char hv_bak;

	p_src->hv = ON;
	//hv_bak = ON;
	src = &p_src->map[0];
	app_display_set_show(Bit_SET);
	app_display_set_point(p_src->hv);
	
	while(1)
	{
		xSemaphoreTake(p_src->xDisplay, portMAX_DELAY);
        
        if(start_dis <= 9)
        {
    	    p_src->map[0] = start_dis;
            p_src->map[1] = start_dis;
    	    p_src->map[2] = start_dis;
    	    p_src->map[3] = start_dis;
    	    p_src->map[4] = start_dis;
    	    p_src->map[5] = start_dis;
            start_dis++; 
	    }
		if(p_src->hv == ON)
			app_display_set_point(src[TUBE_NUM]);
		else
			app_display_set_point(0);
		app_display_calc_map(map, src);
        app_display_write_data(map);
		xSemaphoreGive(p_src->xDisplay);
		
		vTaskDelay(mainDELAY_MS(200));
	}	
}

