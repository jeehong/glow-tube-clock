#ifndef __I2C_BUS_H__
#define	__I2C_BUS_H__

#include "stm32f10x.h"


typedef struct {
	u32 rcc_scl;
	u32 rcc_sda;
	GPIO_TypeDef* group_scl;
	GPIO_TypeDef* group_sda;
	u16 pin_scl;
	u16 pin_sda;
	GPIOSpeed_TypeDef speed;
} I2C_RESOURCE_t;

typedef enum  {
	input = 0,
	output = 1,
} GPIO_DIR_e;

typedef enum {
	ds = 0,
	sht = 1,
	chip_all = 2,
} CHIP_LIST_e;


void i2c_bus_start(CHIP_LIST_e chip);
void i2c_bus_sda_dir(CHIP_LIST_e chip, GPIO_DIR_e dir);
BitAction I2C_BUS_SDA_STATE(CHIP_LIST_e chip);
void i2c_bus_init(void);
u8 i2c_bus_write_byte(CHIP_LIST_e chip, u8 value);
u8 i2c_bus_read_byte(CHIP_LIST_e chip, u8 ack);
void i2c_bus_write_ds3231(CHIP_LIST_e chip, u8 addr, u8 reg, const u8 *pdata, u8 len);
void i2c_bus_read_ds3231(CHIP_LIST_e chip, u8 addr, u8 reg, u8 *pdata, u8 len);



#endif

