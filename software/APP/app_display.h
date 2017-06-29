#ifndef __APP_DISPLAY_H__
#define	__APP_DISPLAY_H__

#include "main.h"

void app_display_task(void *parame);
void app_display_set_show(BitAction act);
void app_display_show_info(u8 *src);
void app_display_set_hv(SWITCH_STATE_e state);


#endif 
