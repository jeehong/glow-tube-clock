#ifndef __APP_SHT10_H__
#define	__APP_SHT10_H__

#include "main.h"

//*******************¦Ì??¡¤  ?¨¹¨¢? *******************************
#define		MEASURE_TEMP	0x03   //000   00011
#define		MEASURE_HUM		0x05   //000   00101

typedef enum {
	HUM = 0,
	TEMP,
} SHT10_INFO_e;

float app_sht10_get_data(unsigned char  type);
void app_sht10_task(void *parame);
void app_sht10_refresh_display(void);


#endif



