#ifndef __BSP_H__
#define __BSP_H__

#define mainCOM_BAUD_RATE		( 115200 )

typedef enum {
	ON = 0,
	OFF = 1,
} SWITCH_STATE_e;

void  bsp_init(void);
void bsp_set_power_hv(SWITCH_STATE_e state);
BitAction bsp_get_power_hv(void);


#endif
