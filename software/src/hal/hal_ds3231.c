#include "os_inc.h"

#include "misc.h"
#include "stm32f10x_exti.h"

#include "i2c_bus.h"
#include "hal_ds3231.h"


#define	DS1231_SLAVE_ADDR		0xD0

#define DS3231_REG_SECONDS		0x00
#define DS3231_REG_MINUTES		0x01
#define DS3231_REG_HOURS		0x02
#define DS3231_REG_AMPM			0x02
#define DS3231_REG_WDAY			0x03
#define DS3231_REG_MDAY			0x04
#define DS3231_REG_MONTH		0x05
#define DS3231_REG_CENTURY		0x05
#define DS3231_REG_YEAR			0x06
#define DS3231_REG_ALARM1       0x07	/* Alarm 1 BASE */
#define DS3231_REG_ALARM2       0x0B	/* Alarm 2 BASE */
#define DS3231_REG_CR			0x0E	/* Control register */
#define DS3231_REG_CR_nEOSC     0x80
#define DS3231_REG_CR_INTCN     0x04
#define DS3231_REG_CR_A2IE      0x02
#define DS3231_REG_CR_A1IE      0x01

#define DS3231_REG_SR			0x0F	/* control/status register */
#define DS3231_REG_SR_OSF   	0x80
#define DS3231_REG_SR_BSY   	0x04
#define DS3231_REG_SR_A2F   	0x02
#define DS3231_REG_SR_A1F   	0x01

/* interrupt flags */
#define RTC_IRQF 0x80	/* Any of the following is active */
#define RTC_PF 0x40	/* Periodic interrupt */
#define RTC_AF 0x20	/* Alarm interrupt */
#define RTC_UF 0x10	/* Update interrupt for 1Hz RTC */

int hal_ds3231_read_time(struct rtc_time *ptime)
{
	U8 data[7];
	U32 century;

	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_SECONDS, data, 7);

	/* Extract additional information for AM/PM and century */
	ptime->h12 = (data[2] & 0x40) ? 1 : 0;
	ptime->am_pm = (data[2] & 0x20) ? 1 : 0;
	century = (data[5] & 0x80) ? 1 : 0;

	/* Write to rtc_time structure */

	ptime->sec = bcd2bin(data[0]);
	ptime->min = bcd2bin(data[1]); 
	if (ptime->h12) 	
	{
		if (ptime->am_pm)
			ptime->hour = bcd2bin((data[2] & 0x1F)) + 12;
		else
			ptime->hour = bcd2bin((data[2] & 0x1F));
	} 
	else 
	{
		ptime->hour = bcd2bin(data[2]);
	}

	/* Day of the week in linux range is 0~6 while 1~7 in RTC chip */
	ptime->wday = bcd2bin(data[3]);
	ptime->mday = bcd2bin(data[4]);
	/* linux tm_mon range:0~11, while month range is 1~12 in RTC chip */
	ptime->mon = bcd2bin((data[5] & 0x7F));
	ptime->year = bcd2bin(data[6]) + century * 100;

	return rtc_valid_tm(ptime);
}

void hal_ds3231_set_time(const struct rtc_time *ptime)
{
	/* Extract time from rtc_time and load into ds3231*/
	U8 data[7];
	
	data[0] = bin2bcd(ptime->sec);
	data[1] = bin2bcd(ptime->min);
	data[2] = bin2bcd(ptime->hour);
	data[3] = bin2bcd(ptime->wday);
	data[4] = bin2bcd(ptime->mday); /* Date */
	data[5] = bin2bcd(ptime->mon);
	if (ptime->year >= 100) 
	{
		data[6] |= 0x80;
		data[6] |= bin2bcd((ptime->year - 100));
	} 
	else 
	{
		data[6] = bin2bcd(ptime->year);
	}
	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_SECONDS, data, 7);
}

/*
 * DS3232 has two alarm, we only use alarm1
 * According to linux specification, only support one-shot alarm
 * no periodic alarm mode
 */
void hal_ds3231_read_alarm(struct rtc_wkalrm *alarm)
{
	U8 control;
	U8 buf[4];

	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_CR, &control, 1);
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);

	alarm->time.sec = buf[0];
	alarm->time.min = buf[1];
	alarm->time.hour = buf[2];
	alarm->time.mday = buf[3];

	alarm->time.mon = 0;
	alarm->time.year = 0;
	alarm->time.wday = 0;

	alarm->enabled = !!(control & DS3231_REG_CR_A1IE);
}

/*
 * linux rtc-module does not support wday alarm
 * and only 24h time mode supported indeed
 */
void hal_ds3231_set_alarm(struct rtc_wkalrm *alarm)
{
	U8 buf[4];

	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);
	
	buf[0] = bin2bcd(alarm->time.sec) | (buf[0] & 0x80);
	buf[1] = bin2bcd(alarm->time.min) | (buf[1] & 0x80);
	buf[2] = bin2bcd(alarm->time.hour) | (buf[2] & 0x80);
	buf[3] = bin2bcd(alarm->time.mday) | (buf[3] & 0x80);

	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);
}

/*
 * 入口参数:  
 * 0: 当 RTC时间与该值匹配时，中断触发； 
 * 1: 该值不作为判定条件，即无需匹配定时即可触发
 */
void hal_ds3231_set_match(U8 mday_match, U8 hour_match, U8 min_match, U8 sec_match)
{
	U8 buf[4];
	
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);

	buf[0] = sec_match ? (0x80 | buf[0]) : (buf[0] & 0x7f);
	buf[1] = min_match ? (0x80 | buf[1]) : (buf[1] & 0x7f);
	buf[2] = hour_match ? (0x80 | buf[2]) : (buf[2] & 0x7f);
	buf[3] = mday_match ? (0x80 | buf[3]) : (buf[3] & 0x7f);

	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);
}

/* 清除中断状态，当触发中断后，只有清除中断状态，才允许再次触发中断 */
void hal_ds3231_de_irq(void)
{
	U8 stat;

	/* clear any pending alarm flag */
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_SR, &stat, 1);
	stat &= ~(DS3231_REG_SR_A1F | DS3231_REG_SR_A2F);
	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_SR, &stat, 1);
}

void hal_ds3231_en_irq(U8 flag)
{
	U8 control;
	
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_CR, &control, 1);
	if(flag)
	{
		/* enable alarm1 interrupt */
		control |= DS3231_REG_CR_A1IE;
	}
	else 
	{
		/* disable alarm1 interrupt */
		control &= ~(DS3231_REG_CR_A1IE | DS3231_REG_CR_A2IE);
	}
	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_CR, &control, 1);	
}

static QueueHandle_t *time_sync;

void hal_ds3231_init(QueueHandle_t *p_time_sync)
{
	time_sync = p_time_sync;
}

void EXTI0_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(EXTI_GetITStatus(EXTI_Line0) != RESET) 
	{		
		xSemaphoreGiveFromISR(*time_sync, &xHigherPriorityTaskWoken);
		EXTI_ClearITPendingBit(EXTI_Line0);  
	}  
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

