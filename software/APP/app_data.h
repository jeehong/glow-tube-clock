#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include "main.h"

#define 	DATA_START_ADDRESS		0x08000000 + 0x40000


#pragma pack(2)
typedef struct _data_map 
{
	unsigned short showon;
	unsigned short showoff;
}	DATA_MAP_t;
#pragma pack()

void app_data_init(void);
void app_data_read_showtime(unsigned short *on, unsigned short *off);
void app_data_write_showtime(unsigned short on, unsigned short off);
void app_data_store_task(GLOBAL_SOURCE_t *p_src);

#endif
