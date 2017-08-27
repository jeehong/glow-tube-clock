#ifndef __APP_DISPLAY_H__
#define	__APP_DISPLAY_H__

#include "common_type.h"

#include "main.h"
#include "bsp.h"
#include "app_time.h"

/* ÉèÖÃËø´æÆ÷ÊÇ·ñÊä³ö 1:out */
__inline app_display_set_show(BitAction act)
{
		bsp_74hc595_set_OE((BitAction)!act);
}

void app_display_task(void *parame);
void app_display_show_info(U8 *src);
void app_display_on(rtc_time_attribute *tm);
void app_display_off(rtc_time_attribute *tm);


#endif 
