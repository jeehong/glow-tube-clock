#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include "common_type.h"

#include "main.h"

#define 	DATA_START_ADDRESS		0x08000000 + 0x40000


#pragma pack(4)
typedef struct _data_map 
{
	U16 showon;
	U16 showoff;
	U32 led_color;		/* 0:R 1:G 2:B */
}	DATA_MAP_t;
#pragma pack()

void app_data_init(void);
void app_data_read_showtime(U16 *on, U16 *off);
void app_data_write_showtime(U16 on, U16 off);
void app_data_store_task(void *parame);
void app_data_read_led_color(U16 *color);
void app_data_write_led_color(U16 color);

#endif
