#ifndef __APP_SHT10_H__
#define	__APP_SHT10_H__



//*******************¦Ì??¡¤  ?¨¹¨¢? *******************************
#define		MEASURE_TEMP	0x03   //000   00011
#define		MEASURE_HUM		0x05   //000   00101

#define		HUM			1
#define		TEMP		2


float app_sht10_get_info(unsigned char type);
void app_sht10_task(void *parame);


#endif



