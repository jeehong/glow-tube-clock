#include "i2c_bus.h"
#include "stm32f10x.h"

typedef enum  {
	input = 0,
	output = 1,
} GPIO_DIR_e;

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

typedef struct {
	u32 rcc_scl;
	u32 rcc_sda;
	GPIO_TypeDef* group_scl;
	GPIO_TypeDef* group_sda;
	u16 pin_scl;
	u16 pin_sda;
	u32 mask;
	u32 nudity;
} I2C_RESOURCE_t;

#define	I2C_BUS_SDA_HIGH(chip)		(chip.group_sda->BSRR = chip.pin_sda)
#define	I2C_BUS_SDA_LOW(chip)		(chip.group_sda->BRR = chip.pin_sda)
#define	I2C_BUS_SCL_HIGH(chip)		(chip.group_scl->BSRR = chip.pin_scl)
#define	I2C_BUS_SCL_LOW(chip)		(chip.group_scl->BRR = chip.pin_scl)
#define	I2C_BUS_SDA_STATE(chip)		((chip.group_sda->ODR & chip.pin_sda != (u32)Bit_RESET) ? (u8)Bit_SET : (u8)Bit_RESET)

static void delayus(u16 us);
static void i2c_bus_sda_dir(I2C_RESOURCE_t *chip, GPIO_DIR_e dir);
static void i2c_bus_start(CHIP_LIST_e chip);
static void i2c_bus_chip_reset(CHIP_LIST_e chip);

static I2C_RESOURCE_t i2c_bus[2];


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

	i2c_bus[sht].rcc_scl = I2C_SHT_RCC_SCL;
	i2c_bus[sht].rcc_sda = I2C_SHT_RCC_SDA;
	i2c_bus[sht].group_scl = I2C_SHT_GROUP_SCL;
	i2c_bus[sht].group_sda = I2C_SHT_GROUP_SDA;
	i2c_bus[sht].pin_scl = I2C_SHT_SCL;
	i2c_bus[sht].pin_sda = I2C_SHT_SDA;

	for(index = ds; index < chip_all; index++)
	{
		RCC_APB2PeriphClockCmd(i2c_bus[index].rcc_scl | i2c_bus[index].rcc_sda, ENABLE); 
		
		GPIO_InitStructure.GPIO_Pin = i2c_bus[index].pin_scl | i2c_bus[index].rcc_sda;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(i2c_bus[index].group_scl, &GPIO_InitStructure);			  
		GPIO_ResetBits(i2c_bus[index].group_scl, i2c_bus[index].pin_scl);
	}	
	i2c_bus_chip_reset(ds);
	i2c_bus_chip_reset(sht);
}

/*
 * 1us/72000000=1.38e-8;
 * 计算得出大概75个时钟周期
 */
static void delayus(u16 us) 
{
	us *= 20;
	while(us--) ;
}

static void i2c_bus_sda_dir(I2C_RESOURCE_t *chip, GPIO_DIR_e dir)
{
	u16 index;

	chip->mask = 0x000F;
	if(dir == input)
		chip->nudity = 0x0008;
	else
		chip->nudity = 0x0003;
	if(chip->pin_sda <= 0x0080) 
	{							
		for(index = 1; index <= 0x80; index <<= 1) 
		{ 
			if(index != chip->pin_sda) 	
			{
				chip->mask <<= 1;
				chip->nudity <<= 1;
			}
			else
				break;
		}
		chip->group_sda->CRL &= ~chip->mask;
		chip->group_sda->CRL |= chip->nudity;
	}
	else
	{
		for(index = 0x100; index <= 0x8000; index <<= 1) 
		{ 
			if(index != chip->pin_sda) 	
			{
				chip->mask <<= 1;
				chip->nudity <<= 1;
			}
			else
				break;
		}
		chip->group_sda->CRH &= ~chip->mask;
		chip->group_sda->CRH |= chip->nudity;	
	}
	chip->group_sda->ODR |= chip->pin_sda;	/* IPU */
}

/*
 * 启动传输
 *       _____         _____
 * DATA:      |_______|
 *           ___     ___
 * SCK : ___|   |___|   |___
 */
static __inline void i2c_bus_start(CHIP_LIST_e chip)
{
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

//----------------------------------------------------------------------------------
__inline u8 i2c_bus_write_byte(CHIP_LIST_e chip, u8 addr, u8 value)
{
	u8 index;
	u8 error = 0;
	
	i2c_bus_sda_dir( &i2c_bus[chip], output);
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
	
	i2c_bus_sda_dir( &i2c_bus[chip], input);	
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);				
	delayus(3);
	error = I2C_BUS_SDA_STATE(i2c_bus[chip]);              
	delayus(1);
	I2C_BUS_SCL_LOW(i2c_bus[chip]);				
	i2c_bus_sda_dir( &i2c_bus[chip], output);
	delayus(1);
	I2C_BUS_SDA_HIGH(i2c_bus[chip]);						
	
	return error;				
}

__inline u8 i2c_bus_read_byte(CHIP_LIST_e chip, u8 addr, u8 ack)
{
	u8 index;
	u8 val = 0;
	
	i2c_bus_sda_dir( &i2c_bus[chip], input);
	delayus(1);
	
	for(index = 0x80; index > 0; index >>= 1)		
	{ 
		I2C_BUS_SCL_HIGH(i2c_bus[chip]);		
		delayus(3);
		
		if(I2C_BUS_SDA_STATE(i2c_bus[chip]))					
			val = (val | index);			
		delayus(1);
		I2C_BUS_SCL_LOW(i2c_bus[chip]);					
	}
	
	i2c_bus_sda_dir( &i2c_bus[chip], output);
	delayus(1);
	
	if(ack)								
		I2C_BUS_SDA_LOW(i2c_bus[chip]);
	else
		I2C_BUS_SDA_HIGH(i2c_bus[chip]);
	
	I2C_BUS_SCL_HIGH(i2c_bus[chip]);				
	delayus(3);
	I2C_BUS_SCL_LOW(i2c_bus[chip]);				
	delayus(1);

	I2C_BUS_SDA_HIGH(i2c_bus[chip]);
	
	return val;
}

