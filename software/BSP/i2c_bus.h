#ifndef __I2C_BUS_H__
#define	__I2C_BUS_H__

#include "stm32f10x.h"

typedef enum {
	ds = 0,
	sht = 1,
	chip_all = 2,
} CHIP_LIST_e;


void i2c_bus_init(void);
u8 i2c_bus_write_byte(CHIP_LIST_e chip, u8 addr, u8 value);
u8 i2c_bus_read_byte(CHIP_LIST_e chip, u8 addr, u8 ack);


#endif

