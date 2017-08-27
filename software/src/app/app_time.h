#ifndef __APP_TIME_H__
#define	__APP_TIME_H__

#include "common_type.h"

#include "mid_rtc.h"

typedef const struct rtc_time rtc_time_attribute;

enum event_id_e
{
	TIME_EVENT_INTEGER_BEEP	= 0,
	TIME_EVENT_DISPLAY_TIME,
	TIME_EVENT_DISPLAY_ON1,
	TIME_EVENT_DISPLAY_OFF1,
	TIME_EVENT_DISPLAY_ON2,
	TIME_EVENT_DISPLAY_OFF2,
};

void app_time_task(void *parame);
void app_time_set_showtime(U16 on, U16 off);
void app_time_get_showtime(U16 *on, U16 *off);
struct event_info *app_time_get_event(U8 index);


#endif


