#ifndef __APP_DISPLAY_H__
#define	__APP_DISPLAY_H__

#include "main.h"
#include "bsp.h"
#include "app_ds3231.h"

/* ÉèÖÃËø´æÆ÷ÊÇ·ñÊä³ö 1:out */
__inline app_display_set_show(BitAction act)
{
		bsp_74hc595_set_OE((BitAction)!act);
}

void app_display_task(void *parame);
void app_display_show_info(u8 *src);
void app_display_on(const struct rtc_time *tm);
void app_display_off(const struct rtc_time *tm);


#endif 
