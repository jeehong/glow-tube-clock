#ifndef __BSP_H__
#define __BSP_H__

#include "stm32f10x_gpio.h"

#define mainCOM_BAUD_RATE		( 115200 )

typedef enum {
	ON = 0,
	OFF = 1,
} SWITCH_STATE_e;

void  bsp_init(void);
void bsp_set_hv_state(SWITCH_STATE_e state);
BitAction bsp_get_hv_state(void);
void bsp_74hc595_set_OE(BitAction act);
void bsp_74hc595_set_LOCK(BitAction act);
void bsp_74hc595_set_OFFSET(BitAction act);
void bsp_74hc595_set_DATA(BitAction act);


#endif
