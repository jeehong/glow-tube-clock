#ifndef __I2C_BUS_H__
#define	__I2C_BUS_H__

#include "common_type.h"

#include "stm32f10x.h"

typedef struct {
	U32 rcc_scl;
	U32 rcc_sda;
	GPIO_TypeDef* group_scl;
	GPIO_TypeDef* group_sda;
	U16 pin_scl;
	U16 pin_sda;
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
U8 i2c_bus_write_byte(CHIP_LIST_e chip, U8 value);
U8 i2c_bus_read_byte(CHIP_LIST_e chip, U8 ack);
void i2c_bus_write_ds3231(CHIP_LIST_e chip, U8 addr, U8 reg, const U8 *pdata, U8 len);
void i2c_bus_read_ds3231(CHIP_LIST_e chip, U8 addr, U8 reg, U8 *pdata, U8 len);

#endif

