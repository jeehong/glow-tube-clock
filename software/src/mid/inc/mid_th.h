#ifndef __MID_TH_H__
#define __MID_TH_H__

#include "common_type.h"

#include "hal_sht10.h"

typedef struct _mid_th_data_t
{
	float temperature;
	float humidity;
} mid_th_data_t;

extern ERROR_CODE mid_th_get_data(struct _mid_th_data_t *p);

#endif

