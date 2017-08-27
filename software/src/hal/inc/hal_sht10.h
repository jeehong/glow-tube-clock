#ifndef __HAL_SHT10_H__
#define __HAL_SHT10_H__

#include "common_type.h"

typedef enum {
	HUM = 0,
	TEMP,
} SHT10_INFO_e;

float hal_sht10_get_data(U8 type);

#endif

