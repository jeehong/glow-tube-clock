#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "i2c_bus.h"
#include "app_sht10.h"
#include "app_serial.h"

#include "main.h"

union {
	unsigned short sval;
	float fval;
} hum, temp;

/* 有效位为0.1 */
static void app_sht10_calc_th(void)
{
	const int c1 = -4;
	const float c2 = 0.0405, c3 = -2.8, d1 = -39.63, d2 = 0.01;
	const float carry = 0.05;  /* 当保留小数点后以为有效时，四舍五入操作 */
	
	temp.fval = temp.sval * d2 + d1 + carry;
	hum.fval = hum.sval * c2 + c3 * hum.sval * hum.sval / 1000000.0 + c1 + carry;	
}

u8 app_sht10_get_res(u16 *p_value, u8 *p_checksum, u8 mode)
{
	u8 error = 0;

	i2c_bus_start(sht);
	
	switch(mode)
	{
		case TEMP : error += i2c_bus_write_byte(sht, MEASURE_TEMP);
					break;
		case HUM  : error += i2c_bus_write_byte(sht, MEASURE_HUM);
					break;
		default   : break;
	}

	i2c_bus_sda_dir(sht, input);
	
	while(I2C_BUS_SDA_STATE(sht) != 0)
	{
		vTaskDelay(50);
	}
	
	if(I2C_BUS_SDA_STATE(sht))								
	{
		error += 1;							/* 如果长时间数据线没有拉低，说明测量错误 */
		return error;
	}
	
	i2c_bus_sda_dir(sht, output);			/* 恢复 AVR IO 为输出模式 */
	
	*p_value = i2c_bus_read_byte(sht, 0) << 8;	/* 读第一个字节，高字节 (MSB) */
	*p_value |= i2c_bus_read_byte(sht, 0);		/* 读第二个字节，低字节 (LSB) */
	*p_checksum  = i2c_bus_read_byte(sht, 1);	/* read CRC校验码*/
	
	return error;
}

float app_sht10_get_info(unsigned char  type)
{
	u8 check;
	
	app_sht10_get_res(&temp.sval, &check, TEMP);
	app_sht10_get_res(&hum.sval, &check, HUM);
	app_sht10_calc_th();

	if(type == TEMP)
		return temp.fval;
	else
		return hum.fval;
}

void app_sht10_task(GLOBAL_SOURCE_t *p_src)
{
	float temp, hum;
    unsigned int temp32;
	
	while(1)
	{
		temp = app_sht10_get_info(TEMP);
		hum = app_sht10_get_info(HUM);

        if((xSemaphoreTake(p_src->xDisplay, mainDELAY_MS(5)) == pdPASS) && (p_src->flag == SHT_ACT))
		{
            temp32 = temp * 10;
    		p_src->map[0] = temp32 / 100;
    		p_src->map[1] = temp32 % 100 /10;
    		p_src->map[2] = temp32 % 10;
    		p_src->map[3] = 10;
    		p_src->map[4] = (unsigned int)hum / 10;
    		p_src->map[5] = (unsigned int)hum % 10;
            p_src->map[6] = 0x10;
            xSemaphoreGive(p_src->xDisplay);
		}
		dbg_string("Temperature:%3.1fC   Humidity:%3.1f%%\r\n", temp, hum);
		vTaskDelay(2000);
	}
}

