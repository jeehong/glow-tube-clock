#ifndef __APP_DS3131_H__
#define	__APP_DS3131_H__

#include "main.h"

/*
 * The struct used to pass data via the following ioctl. Similar to the
 * struct tm in <time.h>, but it needs to be here so that the kernel
 * source is self contained, allowing cross-compiles, etc. etc.
 */
struct rtc_time {
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
	unsigned char wday;
	unsigned char mday;
	unsigned char mon;
	unsigned char year;
	unsigned char am_pm;	/* 1:pm */
	unsigned char h12;		/* 1:12h */
};

enum event_id_e
{
	TIME_EVENT_INTEGER_BEEP	= 0,
	TIME_EVENT_DISPLAY_TIME,
	TIME_EVENT_DISPLAY_ON1,
	TIME_EVENT_DISPLAY_OFF1,
	TIME_EVENT_DISPLAY_ON2,
	TIME_EVENT_DISPLAY_OFF2,
};

void app_ds3231_task(void *parame);
int app_ds3231_read_time(struct rtc_time *ptime);
void app_ds3231_set_time(const struct rtc_time *ptime);
int rtc_valid_tm(struct rtc_time *tm);
void app_ds3231_set_showtime(short on, short off);
void app_ds3231_get_showtime(short *on, short *off);
struct event_info *app_ds3231_get_event(u8 index);


#endif


