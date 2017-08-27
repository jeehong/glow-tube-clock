#include "os_inc.h"

#include "common_type.h"

#include "mid_th.h"

#include "app_inc.h"

static U8 sht_info[7];

void app_th_task(void *parame)
{
	struct _mid_th_data_t th_data;
	U32 temp32;
	
	while(1)
	{
		vTaskDelay(1000);

		if(mid_th_get_data(&th_data) == STATUS_NORMAL)
		{
	        temp32 = th_data.temp * 10;
			sht_info[0] = temp32 / 100;
			sht_info[1] = temp32 % 100 /10;
			sht_info[2] = temp32 % 10;
			sht_info[3] = 0;
			temp32 = th_data.hum;
			sht_info[4] = temp32 / 10;
			sht_info[5] = temp32 % 10;
	        sht_info[6] = 0x11;
			/* dbg_string("Temperature:%3.1fC   Humidity:%3.1f%%\r\n", temp, hum); */
		}
	}
}

void app_th_refresh_display(void)
{
	app_display_show_info(sht_info);
}
