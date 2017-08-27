#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include "common_type.h"

#include "main.h"

#define 	DATA_START_ADDRESS		0x08000000 + 0x40000


#pragma pack(2)
typedef struct _data_map 
{
	U16 showon;
	U16 showoff;
}	DATA_MAP_t;
#pragma pack()

void app_data_init(void);
void app_data_read_showtime(U16 *on, U16 *off);
void app_data_write_showtime(U16 on, U16 off);
void app_data_store_task(void *parame);

#endif
