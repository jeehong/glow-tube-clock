#include "string.h"

#include "os_inc.h"

#include "i2c_bus.h"
#include "mid_rtc.h"

#include "app_inc.h"

struct event_info
{
	struct rtc_time time;
	void (*functions)(rtc_time_attribute *);
};

#define ITEMS_MAX	20
struct time_event
{
	List_t list;
	U8 valid_num;
	ListItem_t items[ITEMS_MAX];
	struct event_info info[ITEMS_MAX];
};


static U16 ontime = 0, offtime = 0;
static struct time_event events;
static TimerHandle_t xTimer500ms;


void app_time_set_showtime(U16 on, U16 off)
{
	struct event_info *origin;
	
	ontime = on;
	offtime = off;
	app_data_write_showtime(ontime, offtime);
	origin = app_time_get_event(TIME_EVENT_DISPLAY_ON1);
	origin->time.hour = ontime / 100;
	origin->time.min = ontime % 100;
	origin = app_time_get_event(TIME_EVENT_DISPLAY_OFF1);
	origin->time.hour = offtime / 100;
	origin->time.min = offtime % 100;	
}

void app_time_get_showtime(U16 *on, U16 *off)
{
	*on = ontime;
	*off = offtime;
}

U8 app_time_init_event(U8 index, 
									U8 year, U8 mon, U8 mday, 
									U8 hour, U8 min, U8 sec, 
									void (*functions)(rtc_time_attribute *tm))
{
	if(events.valid_num >= ITEMS_MAX)
		return 0xFF;
	if(index >= ITEMS_MAX)
		return index;
	
	vListInitialiseItem(&events.items[index]);
	events.info[index].time.year = year;
	events.info[index].time.year = year;
	events.info[index].time.mon = mon;
	events.info[index].time.mday = mday;
	events.info[index].time.hour = hour;
	events.info[index].time.min = min;
	events.info[index].time.sec = sec;
	events.info[index].functions = functions;
	events.items[index].pvOwner = &events.info[index];
	events.items[index].xItemValue = portMAX_DELAY;	/* 对链表的优先级无要求，总是插入链表尾 */
	
	events.valid_num ++;
	
	return index;
}

void app_time_insert_event(U8 number)
{
	if(number >= ITEMS_MAX)
	{
		return;
	}
	
	if(listIS_CONTAINED_WITHIN(&events.list, &events.items[number]))
	{
		return;
	}

	vListInsert(&events.list, &events.items[number]);
	
	return;
}

void app_time_remove_event(U8 number)
{
	if(number >= ITEMS_MAX)
	{
		return;
	}
	
	if(!listIS_CONTAINED_WITHIN(&events.list, &events.items[number]))
	{
		return;
	}

	uxListRemove(&events.items[number]);
}

struct event_info *app_time_get_event(U8 index)
{
	if(index >= ITEMS_MAX)
		return NULL;
	
	return &events.info[index];
}

#define no_match_condition(val1, val2)	if(val1 != val2 && val1 != 0xFF) continue;

static void events_triger_check(rtc_time_attribute *tm)
{
	ListItem_t *p_item = NULL, *p_item_next = NULL;
	//ListItem_t const *p_item_end = NULL;
	struct event_info *data = NULL;
	U8 index;

	if(listLIST_IS_EMPTY(&events.list))
		return;
	//p_item_end = listGET_END_MARKER(&events.list);

	for(index = 0; index < events.valid_num; index ++)
	{
		if(index == 0)
		{
			p_item_next = listGET_HEAD_ENTRY(&events.list);
		}
		else
		{
			p_item = listGET_NEXT(p_item_next);
			p_item_next = p_item;
		}
		data = (struct event_info *)listGET_LIST_ITEM_OWNER( p_item_next );
		no_match_condition(data->time.sec, tm->sec);
		no_match_condition(data->time.min, tm->min);
		no_match_condition(data->time.hour, tm->hour);
		no_match_condition(data->time.mday, tm->mday);
		no_match_condition(data->time.mon, tm->mon);
		no_match_condition(data->time.year, tm->year);
		data->functions(tm);
	}
}

static void integer_time_beep(rtc_time_attribute *tm)
{
	if(tm->hour > 12)
	{
		app_buzzer_set_times(tm->hour - 12);
	}
	else if(tm->hour > 8)
	{
		app_buzzer_set_times(tm->hour);
	}
	else
	{}
}

static void display_time(rtc_time_attribute *tm)
{
	U8 info[7];

	if((tm->sec > 15 && tm->sec <= 20)
		|| (tm->sec > 35 && tm->sec <= 40)
		|| (tm->sec > 55 && tm->sec <= 59))
	{
		app_th_refresh_display();
	}
	else
	{
		if(xTimer500ms != NULL)
		{
	        xTimerStart(xTimer500ms, 0);
	    }
		info[0] = tm->hour / 10;
		info[1] = tm->hour % 10;
		info[2] = tm->min / 10;
		info[3] = tm->min % 10;
		info[4] = tm->sec / 10;
		info[5] = tm->sec % 10;
		info[6] = 0x33;
		app_display_show_info(info);
	}
}

static void timer_500ms_callback(TimerHandle_t xTimer)
{
	struct rtc_time time;
	U8 info[7];
	
	mid_rtc_get_time(&time);
	info[0] = time.hour / 10;
	info[1] = time.hour % 10;
	info[2] = time.min / 10;
	info[3] = time.min % 10;
	info[4] = time.sec / 10;
	info[5] = time.sec % 10;
	info[6] = 0;
	app_display_show_info(info);
}

void app_time_task(void *parame)
{
	QueueHandle_t timeSync;
	
	struct rtc_time /*time1, */time2;

	mid_rtc_init(&timeSync);

	vListInitialise(&events.list);
	app_data_read_showtime(&ontime, &offtime);
    xTimer500ms = xTimerCreate("Timer500ms", 500, pdFALSE, ( void * ) 0, timer_500ms_callback);
	
	app_time_init_event(TIME_EVENT_INTEGER_BEEP, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, integer_time_beep);
	app_time_init_event(TIME_EVENT_DISPLAY_TIME, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, display_time);
	app_time_init_event(TIME_EVENT_DISPLAY_ON1, 0xFF, 0xFF, 0xFF, ontime / 100, ontime % 100, 0, app_display_on);
	app_time_init_event(TIME_EVENT_DISPLAY_OFF1, 0xFF, 0xFF, 0xFF, offtime / 100, offtime % 100, 0, app_display_off);
	app_time_init_event(TIME_EVENT_DISPLAY_ON2, 0xFF, 0xFF, 0xFF, 7, 0, 0, app_display_on);	/* on 07:00 */
	app_time_init_event(TIME_EVENT_DISPLAY_OFF2, 0xFF, 0xFF, 0xFF, 8, 40, 0, app_display_off);	/* off 08:40 */

	app_time_insert_event(TIME_EVENT_DISPLAY_ON1);
	app_time_insert_event(TIME_EVENT_DISPLAY_TIME);
	app_time_insert_event(TIME_EVENT_INTEGER_BEEP);
	app_time_insert_event(TIME_EVENT_DISPLAY_OFF1);
	app_time_insert_event(TIME_EVENT_DISPLAY_ON2);
	app_time_insert_event(TIME_EVENT_DISPLAY_OFF2);

	while(1)
	{
		xSemaphoreTake(timeSync, portMAX_DELAY);
		
		mid_rtc_get_time(&time2);
		mid_rtc_de_irq();

		events_triger_check(&time2);
	}
}





