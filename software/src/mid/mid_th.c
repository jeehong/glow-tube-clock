#include "mid_th.h"

ERROR_CODE mid_th_get_data(struct _mid_th_data_t *p)
{
	ERROR_CODE err = STATUS_NORMAL;

	if(p == NULL)
	{
		err = ERR_POINTER_0;
		goto exit;
	}
	
	p->temp = hal_sht10_get_data(TEMP);
	if(p->temp < 0)
	{
		err = ERR_TIMEOUT;
		goto exit;
	}
	
	p->hum = hal_sht10_get_data(HUM);
	if(p->hum < 0)
	{
		err = ERR_TIMEOUT;
		goto exit;
	}
	
	exit:
	return err;
}

