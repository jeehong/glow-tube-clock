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

/*
 * This data structure is inspired by the EFI (v0.92) wakeup
 * alarm API.
 */
struct rtc_wkalrm {
	unsigned char enabled;	/* 0 = alarm disabled, 1 = alarm enabled */
	struct rtc_time time;	/* time the alarm is set to */
};

void app_ds3231_task(GLOBAL_SOURCE_t *p_src);

#endif


