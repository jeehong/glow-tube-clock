#include "common_type.h"

#include "i2c_bus.h"
#include "os_inc.h"
#include "hal_sht10.h"


typedef union _hum_temp_e{
	U16 sval;
	float fval;
} hum_temp_e;

#define		MEASURE_TEMP	0x03   //000   00011
#define		MEASURE_HUM		0x05   //000   00101

static void calc_sth11(float *p_humidity ,float *p_temperature)
{
	const float D1=-39.6;	  // for 12 Bit
	const float D2=+0.01; 	// for 12 Bit
	const float C1=-4.0;      // for 12 Bit
	const float C2=+0.0405;     // for 12 Bit
	const float C3=-0.0000028;     // for 12 Bit
	const float T1=+0.01;      // for 14 Bit @ 5V
	const float T2=+0.00008;     // for 14 Bit @ 5V
	float rh=*p_humidity;      // rh: Humidity [Ticks] 12 Bit
	float t=*p_temperature;     // t: Temperature [Ticks] 14 Bit
	float rh_lin;        // rh_lin: Humidity linear
	float rh_true;        // rh_true: Temperature compensated humidity
	float t_C;         // t_C : Temperature
	t_C=t*D2 + D1;       //calc. temperature from ticks
	rh_lin=C3*rh*rh + C2*rh + C1;    //calc. humidity from ticks to [%RH]
	rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;  //calc. temperature compensated humidity [%RH]
	if(rh_true>100)rh_true=100;    //cut if the value is outside of
	if(rh_true<0.1)rh_true=0.1;    //the physical possible range
	*p_temperature=t_C;      //return temperature
	*p_humidity=rh_true;      //return humidity[%RH]
}

static U8 hal_sht10_get_res(U16 *p_value, U8 *p_checksum, SHT10_INFO_e mode)
{
	U8 error = STATUS_NORMAL;

	i2c_bus_start(sht);
	switch(mode)
	{
		case TEMP: 
			error += i2c_bus_write_byte(sht, MEASURE_TEMP);
			break;
		case HUM: 
			error += i2c_bus_write_byte(sht, MEASURE_HUM);
			break;
		default: 
			break;
	}
	i2c_bus_sda_dir(sht, input);
	while(I2C_BUS_SDA_STATE(sht) != 0)
		vTaskDelay(50);

	if(I2C_BUS_SDA_STATE(sht))								
	{
		error += 1;							/* 如果长时间数据线没有拉低，说明测量错误 */
		return error;
	}
	i2c_bus_sda_dir(sht, output);			/* 恢复 AVR IO 为输出模式 */
	*p_value = i2c_bus_read_byte(sht, 0) << 8;	/* 读第一个字节，高字节 (MSB) */
	*p_value |= i2c_bus_read_byte(sht, 0);		/* 读第二个字节，低字节 (LSB) */
	*p_checksum  = i2c_bus_read_byte(sht, 1);	/* read CRC校验码 */
	
	return error;
}

float hal_sht10_get_data(SHT10_INFO_e type)
{
	union _hum_temp_e hum, temp;
	U8 check;
	U8 err = STATUS_NORMAL;
	
	err += hal_sht10_get_res(&temp.sval, &check, TEMP);
	err += hal_sht10_get_res(&hum.sval, &check, HUM);
	if(err != STATUS_NORMAL)
		return -1;
	
	hum.fval = hum.sval;
	temp.fval = temp.sval;
	calc_sth11(&hum.fval, &temp.fval);

	if(type == TEMP)
		return temp.fval;
	else
		return hum.fval;
}

