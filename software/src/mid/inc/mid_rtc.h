#ifndef __MID_RTC_H__
#define __MID_RTC_H__

#include "os_inc.h"

#include "common_type.h"

/*
 * The struct used to pass data via the following ioctl. Similar to the
 * struct tm in <time.h>, but it needs to be here so that the kernel
 * source is self contained, allowing cross-compiles, etc. etc.
 */
struct rtc_time {
	U8 sec;
	U8 min;
	U8 hour;
	U8 wday;
	U8 mday;
	U8 mon;
	U8 year;
	U8 am_pm;	/* 1:pm */
	U8 h12;		/* 1:12h */
};

/*
 * This data structure is inspired by the EFI (v0.92) wakeup
 * alarm API.
 */
struct rtc_wkalrm {
	U8 enabled;				/* 0 = alarm disabled, 1 = alarm enabled */
	struct rtc_time time;	/* time the alarm is set to */
};

#define bcd2bin(bcd) 	(((bcd) & 0x0f) + ((bcd) >> 4) * 10)
#define bin2bcd(bin) 	((((bin) / 10) << 4) | ((bin) % 10))

__inline U8 is_leap_year(U32 year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

/*
 * Does the rtc_time represent a valid date/time?
 */
int rtc_valid_tm(struct rtc_time *tm);
U32 rtc_month_days(U32 month, U32 year);

void mid_rtc_init(QueueHandle_t *p_time_sync);

void mid_rtc_set_match(void);
void mid_rtc_set_time(const struct rtc_time *ptime);
void mid_rtc_get_time(struct rtc_time *ptime);
void mid_rtc_en_irq(U8 flag);
void mid_rtc_de_irq(void);
void mid_rtc_read_alarm(struct rtc_wkalrm *alarm);

#endif

