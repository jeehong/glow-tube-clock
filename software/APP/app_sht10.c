#include "FreeRTOS.h"
#include "task.h"

#include "i2c_bus.h"
#include "app_sht10.h"
#include "app_serial.h"

union {
	unsigned short s_val;
	float f_val;
} hum, temp;

void app_sht10_calc_th( float *p_humidity , float *p_temperature )
//----------------------------------------------------------------------------------------
// ����������¶Ⱥ����ʪ��
{ 
	const float C1 = -4.0;							/* for 12 Bit ʪ��������ʽ */
	const float C2 = 0.0405;						/* for 12 Bit ʪ��������ʽ */
	const float C3 = -0.0000028;					/* for 12 Bit ʪ��������ʽ */
	const float T1 = 0.01;							/* for 14 Bit @ 5V �¶�������ʽ */
	const float T2 = 0.00008;						/* for 14 Bit @ 5V �¶�������ʽ */
	float rh = *p_humidity;
	float t = *p_temperature;
	float rh_lin;
	float rh_true;
	float t_C;
	t_C = t * 0.01 - 40;							/* �����¶� */
	rh_lin = C3 * rh * rh + C2 * rh + C1;			/* ���ʪ�ȷ����Բ��� */
	rh_true = (t_C - 25) * (T1 + T2 * rh) + rh_lin; /* ���ʪ�ȶ����¶������Բ��� */
	
	rh_true -= 5.0;									/* ע��proteus ������У���� */
	rh_true -= 0.5; 
	
	if( rh_true > 100 )
	{
		rh_true = 100;			/* ʪ��������� */
	}
	if( rh_true < 0.1 )
	{
		rh_true = 0.1;			/* ʪ����С���� */
	}
	*p_temperature = t_C;					/* �����¶Ƚ�� */
	*p_humidity = rh_true;					/* ����ʪ�Ƚ�� */
}


u8 app_sht10_get_res(u16 *p_value, u8 *p_checksum, u8 mode)
{
	u8 error = 0;
	u8 index = 0;
	
	switch(mode)
	{
		case TEMP : error += i2c_bus_write_byte(sht, MEASURE_TEMP);
					break;
		case HUM  : error += i2c_bus_write_byte(sht, MEASURE_HUM);
					break;
		default   : break;
	}

	i2c_bus_sda_dir(sht, input);
	
	while((I2C_BUS_SDA_STATE(sht) != 0) && (index++ < 10))
	{
		vTaskDelay(100);
	}
	
	if(I2C_BUS_SDA_STATE(sht))								
	{
		error += 1;							/* �����ʱ��������û�����ͣ�˵���������� */
		return error;
	}
	
	i2c_bus_sda_dir(sht, output);			/* �ָ� AVR IO Ϊ���ģʽ */
	
	*p_value = i2c_bus_read_byte(sht) << 8;	/* ����һ���ֽڣ����ֽ� (MSB) */
	*p_value |= i2c_bus_read_byte(sht);		/* ���ڶ����ֽڣ����ֽ� (LSB) */
	*p_checksum  = i2c_bus_read_byte(sht);	/* read CRCУ����*/

	
	
	return error;
}

float app_sht10_get_info(unsigned char  type)
{
	u8 check;
	
	app_sht10_get_res(&temp.s_val, &check, TEMP);
	app_sht10_get_res(&hum.s_val, &check, HUM);
	app_sht10_calc_th(&hum.f_val, &temp.f_val);

	if(type == TEMP)
		return temp.f_val;
	else
		return hum.f_val;
}

void app_sht10_task(void *parame)
{
	float temp, hum;
	
	while(1)
	{
		temp = app_sht10_get_info(TEMP);
		hum = app_sht10_get_info(HUM);
		dbg_string("TEMP:%f   HUM:%f\r\n", temp, hum);	
	}
}

