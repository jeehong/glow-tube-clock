#include "os_inc.h"
#include "common_type.h"
#include "mid_th.h"
#include "mid_dbg.h"
#include "app_inc.h"

static struct _mid_th_data_t th_data;

void app_th_task(void *parame)
{
	while(1)
	{
		vTaskDelay(1000);

		if(mid_th_get_data(&th_data) != STATUS_NORMAL)
		{
			dbg_string("Error: Read Temperature&Humidity faild!\r\n");
		}
	}
}

void app_th_refresh_display(void)
{
	U8 sht_info[7];
	U32 temp32;
	
	temp32 = th_data.temp * 10;
	sht_info[0] = temp32 / 100;
	sht_info[1] = temp32 % 100 /10;
	sht_info[2] = temp32 % 10;
	sht_info[3] = 0;
	temp32 = th_data.hum;
	sht_info[4] = temp32 / 10;
	sht_info[5] = temp32 % 10;
	sht_info[6] = 0x11;
	app_display_show_info(sht_info);
}

float app_th_get_data(SHT10_INFO_e element)
{
	switch (element)
	{
		case TEMP:
			return th_data.temp;
		case HUM:
			return th_data.hum;
		default: return 0;
	}
}
