#include "mid_th.h"

ERROR_CODE mid_th_get_data(struct _mid_th_data_t *p)
{
	ERROR_CODE err = STATUS_NORMAL;
	struct _mid_th_data_t dt;

	if(p == NULL)
	{
		err = ERR_POINTER_0;
	}
	if(err == STATUS_NORMAL)
	{
		dt.temp = hal_sht10_get_data(TEMP);
		if(dt.temp < 0)
		{
			err = ERR_TIMEOUT;
		}
	}
	if(err == STATUS_NORMAL)
	{
		dt.hum = hal_sht10_get_data(HUM);
		if(dt.hum < 0)
		{
			err = ERR_TIMEOUT;
		}
		memcpy(p, &dt, sizeof(struct _mid_th_data_t));
	}
	return err;
}

