#include "FreeRTOS.h"
#include "task.h"

#include "i2c_bus.h"

#include "app_ds3231.h"
#include "app_serial.h"

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

#define bcd2bin(bcd) 	((bcd & 0x0f) + (bcd >> 4) * 10)
#define bin2bcd(bin) 	(((bin / 10) << 4) | (bin % 10))
static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static __inline unsigned char is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

static int rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/*
 * Does the rtc_time represent a valid date/time?
 */
static int rtc_valid_tm(struct rtc_time *tm)
{
	if (tm->year < 70
		|| ((unsigned)tm->mon) >= 12
		|| tm->mday < 1
		|| tm->mday > rtc_month_days(tm->mon, tm->year + 1900)
		|| ((unsigned)tm->hour) >= 24
		|| ((unsigned)tm->min) >= 60
		|| ((unsigned)tm->sec) >= 60)
		return 1;

	return 0;
}

static int app_ds3231_read_time(struct rtc_time *ptime)
{
	struct rtc_time data;
	unsigned int century;

	
	i2c_bus_read_data(ds, DS1231_SLAVE_ADDR, &data);

	/* Extract additional information for AM/PM and century */
	ptime->h12 = (data.hour & 0x40) ? 1 : 0;
	ptime->am_pm = (data.hour & 0x20) ? 1 : 0;
	century = (data.mon & 0x80) ? 1 : 0;

	/* Write to rtc_time structure */

	ptime->sec = bcd2bin(data.sec);
	ptime->min = bcd2bin(data.min); 
	if (!ptime->h12) 
	{
		/* Convert to 24 hr */
		if (ptime->am_pm)
			ptime->hour = bcd2bin((data.hour & 0x1F)) + 12;
		else
			ptime->hour = bcd2bin((data.hour & 0x1F));
	} 
	else 
	{
		ptime->hour = bcd2bin(data.hour);
	}

	/* Day of the week in linux range is 0~6 while 1~7 in RTC chip */
	ptime->wday = bcd2bin(data.wday);
	ptime->mday = bcd2bin(data.mday);
	/* linux tm_mon range:0~11, while month range is 1~12 in RTC chip */
	ptime->mon = bcd2bin((data.mon & 0x7F));
	ptime->year = bcd2bin(data.year) + century * 100;

	return rtc_valid_tm(ptime);
}

static void app_ds3231_set_time(struct rtc_time *ptime)
{
	/* Extract time from rtc_time and load into ds3231*/
	struct rtc_time data;
	
	data.sec = bin2bcd(ptime->sec);
	data.min = bin2bcd(ptime->min);
	data.hour = bin2bcd(ptime->hour);
	data.wday = bin2bcd(ptime->wday);
	data.mday = bin2bcd(ptime->mday); /* Date */
	/* linux tm_mon range:0~11, while month range is 1~12 in RTC chip */
	data.mon = bin2bcd(ptime->mon);
	if (ptime->year >= 100) 
	{
		data.year |= 0x80;
		data.year |= bin2bcd((ptime->year - 100));
	} 
	else 
	{
		data.year = bin2bcd(ptime->year);
	}
	i2c_bus_write_data(ds, DS1231_SLAVE_ADDR, DS3231_REG_SECONDS, &data);
}

void app_ds3231_task(void *parame)
{
	struct rtc_time time1, time2;

	time1.sec = 0;
	time1.min = 30;
	time1.hour = 19;
	time1.wday = 7;
	time1.mday = 29;
	time1.mon = 5;
	time1.year = 16;
	while(1)
	{
		//app_ds3231_set_time(&time1);
		
		app_ds3231_read_time(&time2);
		
		dbg_string("Time:%d-%d-%d %d %d:%d:%d\r\n", 
									time2.year, 
									time2.mon, 
									time2.mday, 
									time2.wday, 
									time2.hour, 
									time2.min,
									time2.sec);
		vTaskDelay(1000);
	}
}


