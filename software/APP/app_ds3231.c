#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "misc.h"
#include "stm32f10x_exti.h"

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

/* interrupt flags */
#define RTC_IRQF 0x80	/* Any of the following is active */
#define RTC_PF 0x40	/* Periodic interrupt */
#define RTC_AF 0x20	/* Alarm interrupt */
#define RTC_UF 0x10	/* Update interrupt for 1Hz RTC */


#define bcd2bin(bcd) 	(((bcd) & 0x0f) + ((bcd) >> 4) * 10)
#define bin2bcd(bin) 	((((bin) / 10) << 4) | ((bin) % 10))

static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static QueueHandle_t timeSync;

static __inline unsigned char is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

static int rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

static int app_ds3231_read_time(struct rtc_time *ptime);
static void app_ds3231_set_time(struct rtc_time *ptime);
static void app_ds3231_enable_irq(u8 flag);
static void app_ds3231_read_alarm(struct rtc_wkalrm *alarm);
static void app_ds3231_set_alarm(struct rtc_wkalrm *alarm);


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
	u8 data[7];
	unsigned int century;

	
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

static void app_ds3231_set_time(struct rtc_time *ptime)
{
	/* Extract time from rtc_time and load into ds3231*/
	u8 data[7];
	
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
static void app_ds3231_read_alarm(struct rtc_wkalrm *alarm)
{
	u8 control;
	u8 buf[4];

	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_CR, &control, 1);
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);

	/* alarm->time.sec = bcd2bin((buf[0] & 0x7F));
	alarm->time.min = bcd2bin((buf[1] & 0x7F));
	alarm->time.hour = bcd2bin((buf[2] & 0x7F));
	alarm->time.mday = bcd2bin((buf[3] & 0x7F)); */
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
static void app_ds3231_set_alarm(struct rtc_wkalrm *alarm)
{
	u8 buf[4];

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
static void app_ds3231_set_match(u8 mday_match, u8 hour_match, u8 min_match, u8 sec_match)
{
	u8 buf[4];
	
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);

	buf[0] = sec_match ? (0x80 | buf[0]) : (buf[0] & 0x7f);
	buf[1] = min_match ? (0x80 | buf[1]) : (buf[1] & 0x7f);
	buf[2] = hour_match ? (0x80 | buf[2]) : (buf[2] & 0x7f);
	buf[3] = mday_match ? (0x80 | buf[3]) : (buf[3] & 0x7f);

	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_ALARM1, buf, 4);
}

/* 清除中断状态，当触发中断后，只有清除中断状态，才允许再次触发中断 */
static void app_ds3231_clear_state(void)
{
	u8 stat;

	/* clear any pending alarm flag */
	i2c_bus_read_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_SR, &stat, 1);
	stat &= ~(DS3231_REG_SR_A1F | DS3231_REG_SR_A2F);
	i2c_bus_write_ds3231(ds, DS1231_SLAVE_ADDR, DS3231_REG_SR, &stat, 1);
}

static void app_ds3231_enable_irq(u8 flag)
{
	u8 control;
	
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

void app_ds3231_task(GLOBAL_SOURCE_t *p_src)
{
	struct rtc_time /*time1, */time2;
	struct rtc_wkalrm /* alarm1, alarm2*/;

	/* set time */
	/* time1.sec = 0;
	time1.min = 1;
	time1.hour = 22;
	time1.wday = 2;
	time1.mday = 31;
	time1.mon = 5;
	time1.year = 16;
	app_ds3231_set_time(&time1); */
	
	/* set alarm */
	// alarm1.enabled = 1;
	// alarm1.time.sec = 0;
	// alarm1.time.min = 0;
	// alarm1.time.hour = 12;
	// alarm1.time.mday = 1;
	/* 设置失能RTC定时中断功能 */
	// app_ds3231_enable_irq(0);	
	/* 设置定时信息 */
	// app_ds3231_set_alarm(&alarm1);	
	/* 设置闹钟模式，采用秒触发 */	
	app_ds3231_set_match(1, 1, 1, 1);	
	/* 设置使能RTC定时中断功能 */
	// app_ds3231_enable_irq(1);
	
	/* app_ds3231_read_alarm(&alarm2);
	dbg_string("sec:0x%x\r\n", alarm2.time.sec);
	dbg_string("min:0x%x\r\n", alarm2.time.min);
	dbg_string("hour:0x%x\r\n", alarm2.time.hour);
	dbg_string("mday:0x%x\r\n", alarm2.time.mday); */
	timeSync = xSemaphoreCreateMutex();
	app_ds3231_clear_state();
	while(1)
	{
		xSemaphoreTake(timeSync, portMAX_DELAY);
		
		app_ds3231_read_time(&time2);
		
		if((time2.min == 0) && (time2.sec == 0))
		{
			if(time2.hour > 12)
				p_src->buz[0] = time2.hour - 12;
			else
				p_src->buz[0] = time2.hour;
			
			xSemaphoreGive(p_src->xBuzzer);
		}
		app_ds3231_clear_state();

		if(p_src->flag == DS3231_ACT)
		{
			if(xSemaphoreTake(p_src->xDisplay, mainDELAY_MS(5)) == pdPASS)
			{
	    		p_src->map[0] = time2.hour / 10;
	    		p_src->map[1] = time2.hour % 10;
	    		p_src->map[2] = time2.min / 10;
	    		p_src->map[3] = time2.min % 10;
	    		p_src->map[4] = time2.sec / 10;
	    		p_src->map[5] = time2.sec % 10;
	            p_src->map[6] = 0x33;
	            xSemaphoreGive(p_src->xDisplay);
				vTaskDelay(mainDELAY_MS(490));
			}
			if(xSemaphoreTake(p_src->xDisplay, portMAX_DELAY) == pdPASS)
			{
				p_src->map[6] = 0;
				xSemaphoreGive(p_src->xDisplay);
			}
		}
        if(((time2.sec > 20) && (time2.sec < 25)) || ((time2.sec > 40) && (time2.sec < 45)))
        {
            p_src->flag = SHT_ACT;
        }
        else
            p_src->flag = DS3231_ACT;
        
        dbg_string("Time:20%02d-%d-%02d %d %02d:%02d:%02d\r\n", 
									time2.year, 
									time2.mon, 
									time2.mday, 
									time2.wday, 
									time2.hour, 
									time2.min,
									time2.sec);
	}
}


void EXTI0_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if(EXTI_GetITStatus(EXTI_Line0) != RESET) 
	{		
		xSemaphoreGiveFromISR(timeSync, &xHigherPriorityTaskWoken);
		EXTI_ClearITPendingBit(EXTI_Line0);  
	}  
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


