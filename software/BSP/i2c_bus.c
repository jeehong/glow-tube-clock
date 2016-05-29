#include "i2c_bus.h"
#include "stm32f10x.h"



/*
 * DS1231 hardware resource
 * SCL:PC14
 * SDA:PC15
 */
#define	I2C_DS_RCC			RCC_APB2Periph_GPIOC
#define	I2C_DS_GROUP		GPIOC
#define	I2C_DS_SCL			GPIO_Pin_14
#define	I2C_DS_SDA			GPIO_Pin_15

/*
 * SHT10 hardware resource
 * SCL:PC10
 * SDA:PA2
 */
#define	I2C_SHT_RCC_SCL		RCC_APB2Periph_GPIOC
#define	I2C_SHT_RCC_SDA		RCC_APB2Periph_GPIOA
#define	I2C_SHT_GROUP_SCL	GPIOC
#define	I2C_SHT_GROUP_SDA	GPIOA
#define	I2C_SHT_SCL			GPIO_Pin_10
#define	I2C_SHT_SDA			GPIO_Pin_2

#define	I2C_BUS_SDA_HIGH(chip)		(chip.group_sda->BSRR = chip.pin_sda)
#define	I2C_BUS_SDA_LOW(chip)		(chip.group_sda->BRR = chip.pin_sda)
#define	I2C_BUS_SCL_HIGH(chip)		(chip.group_scl->BSRR = chip.pin_scl)
#define	I2C_BUS_SCL_LOW(chip)		(chip.group_scl->BRR = chip.pin_scl)


static void delayus(u16 us);

static void i2c_bus_stop(CHIP_LIST_e chip);
static __inline void i2c_bus_send_ack(CHIP_LIST_e chip, u8 ack);
static __inline unsigned char i2c_bus_get_ack(CHIP_LIST_e chip);
static void i2c_bus_chip_reset(CHIP_LIST_e chip);

static I2C_RESOURCE_t i2c_bus[chip_all];


void i2c_bus_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u8 index = 0;

	i2c_bus[ds].rcc_scl = I2C_DS_RCC;
	i2c_bus[ds].rcc_sda = I2C_DS_RCC;
	i2c_bus[ds].group_scl = I2C_DS_GROUP;
	i2c_bus[ds].group_sda = I2C_DS_GROUP;
	i2c_bus[ds].pin_scl = I2C_DS_SCL;
	i2c_bus[ds].pin_sda = I2C_DS_SDA;
	i2c_bus[ds].speed = GPIO_Speed_50MHz;

	i2c_bus[sht].rcc_scl = I2C_SHT_RCC_SCL;
	i2c_bus[sht].rcc_sda = I2C_SHT_RCC_SDA;
	i2c_bus[sht].group_scl = I2C_SHT_GROUP_SCL;
	i2c_bus[sht].group_sda = I2C_SHT_GROUP_SDA;
	i2c_bus[sht].pin_scl = I2C_SHT_SCL;
	i2c_bus[sht].pin_sda = I2C_SHT_SDA;
	i2c_bus[sht].speed = GPIO_Speed_50MHz;

	for(index = ds; index < chip_all; index++)
	{
		RCC_APB2PeriphClockCmd(i2c_bus[index].rcc_scl | i2c_bus[index].rcc_sda, ENABLE); 
		
		GPIO_InitStructure.GPIO_Pin = i2c_bus[index].pin_scl | i2c_bus[index].rcc_sda;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
		GPIO_InitStructure.GPIO_Speed = i2c_bus[index].speed; 
		GPIO_Init(i2c_bus[index].group_scl, &GPIO_InitStructure);			  
		GPIO_ResetBits(i2c_bus[index].group_scl, i2c_bus[index].pin_scl);
	}	
	i2c_bus_chip_reset(ds);
	i2c_bus_chip_reset(sht);
}

/*
 * 1us/72000000=1.38e-8;
 * 计算得出大概75个时钟周期
 * 实际测试 us = 1,实际时间:1.875us
 */
static void delayus(u16 us) 
{
	us *= 1;
	while(us--) ;
}

void i2c_bus_sda_dir(CHIP_LIST_e chip, GPIO_DIR_e dir)
{
	GPIO_InitTypeDef GPIO_InitType;

	GPIO_InitType.GPIO_Pin = i2c_bus[chip].pin_sda;
	GPIO_InitType.GPIO_Speed = i2c_bus[chip].speed;

	if(dir ==input)
		GPIO_InitType.GPIO_Mode = GPIO_Mode_IPU;
	else
		GPIO_InitType.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(i2c_bus[chip].group_sda, &GPIO_InitType); 	
}

/*
 * 启动传输
 *       _____         _____
 * DATA:      |_______|
 *           ___     ___
 * SCK : ___|   |___|   |___
 */
void i2c_bus_start(CHIP_LIST_e chip)
{
	i2c_bus_sda_dir(chip, output);
	
	I2C_BUS_SDA_HIGH(i2c_bus[chip]);		
	I2C_BUS_SCL_LOW(i2c_bus[chip]);		
	delayus(1);
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);		
	delayus(1);
	I2C_BUS_SDA_LOW(i2c_bus[chip]);		
	delayus(1);
	I2C_BUS_SCL_LOW(i2c_bus[chip]);		
	delayus(3);
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);		
	delayus(1);
	I2C_BUS_SDA_HIGH(i2c_bus[chip]);		
	delayus(1);
	I2C_BUS_SCL_LOW(i2c_bus[chip]);		
}

static __inline void i2c_bus_stop(CHIP_LIST_e chip)
{
	i2c_bus_sda_dir(chip, output);
	
	I2C_BUS_SDA_LOW(i2c_bus[chip]);		
	I2C_BUS_SCL_LOW(i2c_bus[chip]);		
	delayus(1);
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);		
	delayus(1);	
}


/*
 * 连接复位;
 *       _____________________________________________________         ________
 * DATA:                                                      |_______|
 *          _    _    _    _    _    _    _    _    _        ___     ___
 * SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
 */
static __inline void i2c_bus_chip_reset(CHIP_LIST_e chip)
{
	u8 index;

	i2c_bus_sda_dir(chip, output);
	I2C_BUS_SDA_HIGH(i2c_bus[chip]);							
	I2C_BUS_SCL_LOW(i2c_bus[chip]);							
	delayus(1);
	for(index = 0; index < 9; index++)		/* DATA保持高，SCK时钟触发9次，发送启动传输，通迅即复位 */
	{ 
		I2C_BUS_SCL_HIGH(i2c_bus[chip]);			
		I2C_BUS_SCL_LOW(i2c_bus[chip]);			
	}
	i2c_bus_start(chip);					
}

static __inline void i2c_bus_send_ack(CHIP_LIST_e chip, u8 ack)
{
	if(ack == 1)
		I2C_BUS_SDA_HIGH(i2c_bus[chip]);	
	else
		I2C_BUS_SDA_LOW(i2c_bus[chip]);	
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);	
	delayus(5);	
	I2C_BUS_SCL_LOW(i2c_bus[chip]);
	delayus(1);
}


static __inline unsigned char i2c_bus_get_ack(CHIP_LIST_e chip)
{
	u8 ack;
	
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);	
	delayus(5);
	i2c_bus_sda_dir(chip, input);	
	I2C_BUS_SCL_LOW(i2c_bus[chip]);
	delayus(1);

	ack = I2C_BUS_SDA_STATE(chip); 
	i2c_bus_sda_dir(chip, output);	

	return ack;
}

//----------------------------------------------------------------------------------
u8 i2c_bus_write_byte(CHIP_LIST_e chip, u8 value)
{
	u8 index;
	u8 error = 0;
	
	i2c_bus_sda_dir(chip, output);
	for(index = 0x80; index > 0; index >>= 1)			
	{ 
		if (index & value) 
			I2C_BUS_SDA_HIGH(i2c_bus[chip]);		
		else 
			I2C_BUS_SDA_LOW(i2c_bus[chip]);		
		
		delayus(1);	
		I2C_BUS_SCL_HIGH(i2c_bus[chip]);					
		delayus(2);
		I2C_BUS_SCL_LOW(i2c_bus[chip]);					
		delayus(1);
	}					
	
	//error = i2c_bus_get_ack(chip);					
	i2c_bus_sda_dir(chip, input);	
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);				
	delayus(3);
	error = I2C_BUS_SDA_STATE(chip);              
	delayus(1);
	I2C_BUS_SCL_LOW(i2c_bus[chip]);				
	i2c_bus_sda_dir(chip, output);
	delayus(1);
	I2C_BUS_SDA_HIGH(i2c_bus[chip]);
	
	return error;				
}

u8 i2c_bus_read_byte(CHIP_LIST_e chip)
{
	u8 index;
	u8 val = 0;
	
	i2c_bus_sda_dir(chip, input);
	delayus(1);
	
	for(index = 0x80; index > 0; index >>= 1)		
	{ 
		I2C_BUS_SCL_HIGH(i2c_bus[chip]);		
		delayus(3);
		
		if(I2C_BUS_SDA_STATE(chip))					
			val = (val | index);			
		delayus(1);
		I2C_BUS_SCL_LOW(i2c_bus[chip]);					
	}
	i2c_bus_sda_dir(chip, output);
	delayus(1);
							
	I2C_BUS_SDA_LOW(i2c_bus[chip]);
	
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);				
	delayus(3);
	I2C_BUS_SCL_LOW(i2c_bus[chip]);				
	delayus(1);

	I2C_BUS_SDA_HIGH(i2c_bus[chip]);
	
	return val;
}

void i2c_bus_write_data(CHIP_LIST_e chip, u8 addr, u8 reg, u8 value)
{
	i2c_bus_start(chip);
	i2c_bus_write_byte(chip, addr);
	i2c_bus_write_byte(chip, reg);
	i2c_bus_write_byte(chip, value);
	i2c_bus_stop(chip);
}

u8 i2c_bus_read_data(CHIP_LIST_e chip, u8 addr, u8 reg)
{
	u8 data;
	
	i2c_bus_start(chip);
	i2c_bus_write_byte(chip, addr);
	i2c_bus_write_byte(chip, reg);
	i2c_bus_start(chip);
	i2c_bus_write_byte(chip, addr + 1);
	data = i2c_bus_read_byte(chip);
	i2c_bus_send_ack(chip, 1);
	i2c_bus_stop(chip);	

	return data;
}

BitAction I2C_BUS_SDA_STATE(CHIP_LIST_e chip)
{
	return (((i2c_bus[chip].group_sda->IDR & i2c_bus[chip].pin_sda) != (u32)Bit_RESET) ? Bit_SET : Bit_RESET);
}

