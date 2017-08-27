#include "hal_ds3231.h"

#include "mid_rtc.h"

static const U8 rtc_days_in_month[] = 
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

U32 rtc_month_days(U32 month, U32 year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/*
 * Does the rtc_time represent a valid date/time?
 */
int rtc_valid_tm(struct rtc_time *tm)
{
	if (tm->year > 100
		|| (tm->mon) > 12
		|| tm->mday < 1
		|| tm->mday > rtc_month_days(tm->mon, tm->year + 2000)
		|| (tm->wday) > 7
		|| (tm->hour) >= 24
		|| (tm->min) >= 60
		|| (tm->sec) >= 60)
		return 1;

	return 0;
}

void mid_rtc_init(QueueHandle_t *p_time_sync)
{
	//struct rtc_time time1;
	//struct rtc_wkalrm  alarm1, alarm2;

	/* set time */
	/* time1.sec = 0;
	time1.min = 1;
	time1.hour = 22;
	time1.wday = 2;
	time1.mday = 31;
	time1.mon = 5;
	time1.year = 16;
	hal_ds3231_set_time(&time1); */
	
	if(p_time_sync != NULL)
	{
		*p_time_sync = xSemaphoreCreateMutex();
		hal_ds3231_init(p_time_sync);
	}
	/* 设置失能RTC定时中断功能 */
	/* mid_rtc_en_irq(0); */	

    mid_rtc_set_match();
	
	/* 设置使能RTC定时中断功能 */
	/* mid_rtc_en_irq(1); */
	
	/* mid_rtc_read_alarm(&alarm2);
	dbg_string("sec:0x%x\r\n", alarm2.time.sec);
	dbg_string("min:0x%x\r\n", alarm2.time.min);
	dbg_string("hour:0x%x\r\n", alarm2.time.hour);
	dbg_string("mday:0x%x\r\n", alarm2.time.mday); */
	
	mid_rtc_de_irq();
}

void mid_rtc_set_match(void)
{
	/* set alarm */
	/* alarm1.enabled = 1;
	alarm1.time.sec = 0;
	alarm1.time.min = 0;
	alarm1.time.hour = 12;
	alarm1.time.mday = 1;
	hal_ds3231_set_alarm(&alarm1); */
	hal_ds3231_set_match(1, 1, 1, 1);	
}

void mid_rtc_set_time(const struct rtc_time *ptime)
{
	hal_ds3231_set_time(ptime);
}

void mid_rtc_get_time(struct rtc_time *ptime)
{
	hal_ds3231_read_time(ptime);
}

void mid_rtc_en_irq(U8 flag)
{
	hal_ds3231_en_irq(flag);
}

void mid_rtc_de_irq(void)
{
	hal_ds3231_de_irq();
}

void mid_rtc_read_alarm(struct rtc_wkalrm *alarm)
{
	hal_ds3231_read_alarm(alarm);
}

