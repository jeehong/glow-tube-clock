#include "stm32f10x.h"
#include <stdio.h>
#include "netconfig.h"	
#include "DM9000x.h"
#include "app_serial.h"


#define  EXINIT                   	GPIO_Pin_9   
#define  CMD                      	GPIO_Pin_3 	 
#define	 CS                       	GPIO_Pin_4	 
#define  IOW                      	GPIO_Pin_7   
#define  IOR                      	GPIO_Pin_6   
	
#define  DATA                     	GPIOB
#define  CMD_PORT                 	GPIOA
#define  CS_PORT                  	GPIOC
#define  IOW_PORT                 	GPIOC
#define  IOR_PORT                 	GPIOC
#define  CONTROL_PORT             	GPIOC
#define  EXINT_PORT               	GPIOC
#define  DATA_PORT                	GPIOB       

#define  GPIOB_CRL                	(*(volatile unsigned long *)GPIOB_BASE) 
#define  GPIOB_CRH				  	(*(volatile unsigned long *)(GPIOB_BASE + 8))
#define  DATA_PORT_CRL            	GPIOB_CRL

#define  DM9000_IO                	0    
#define  DM9000_DATA              	1
#define  DM9000_INDEX             	0
	
#define  DM9000X_CMD(dcmd)        	(dcmd == 0x01)                 ?          \
									(CMD_PORT->ODR |= CMD) :          \
									(CMD_PORT->ODR &= ~CMD)           
	
#define  DM9000X_CS(dcs)          	(dcs == 0x01)                  ?          \
									(CS_PORT->ODR |= CS)  :          \
									(CS_PORT->ODR &= ~CS)           
	
#define  DM9000X_IOW(dwrite)      	(dwrite == 0x01)               ?          \
									(IOW_PORT->ODR |= IOW) :          \
									(IOW_PORT->ODR &= ~IOW)   
									
#define  DM9000X_IOR(dread)       	(dread == 0x01)                ?          \
									(IOR_PORT->ODR |= IOR) :          \
									(IOR_PORT->ODR &= ~IOR)   
	
	
#define  DM9000X_DATA_OUT(ddata)  	(DATA_PORT->ODR = ddata)    
#define  DM9000X_DATA_IN()        	(DATA_PORT->IDR)
	
#define DM9000X_BYTE_MODE         	0x01
#define DM9000X_WORD_MODE         	0x00


static void dm9000_delay(u32 delay)
{
	while (delay--);
}

void dm9000x_gpio_inital(void)
{
	GPIO_InitTypeDef GPIO_dm9000x;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

	/* Configure control io as output push-pull */
	GPIO_dm9000x.GPIO_Pin = CMD | CS | IOW | IOR;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(CONTROL_PORT, &GPIO_dm9000x);

	/* Configure data io as inout */
	GPIO_dm9000x.GPIO_Pin = GPIO_Pin_All;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DATA_PORT, &GPIO_dm9000x);
}

/*********************************************************************************************************************/
static __inline void dm9000_out_byte(u16 outdata, u16 reg)
{     
	GPIO_InitTypeDef GPIO_dm9000x;
	
	GPIO_dm9000x.GPIO_Pin = GPIO_Pin_All;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
    portENTER_CRITICAL();
	GPIO_Init(DATA_PORT, &GPIO_dm9000x);
	DM9000X_DATA_OUT(outdata); 	       		
	DM9000X_CMD(reg);			   
	DM9000X_CS(0);				   
	DM9000X_IOR(1);				   
	DM9000X_IOW(0);	
	DM9000X_IOW(1);				   
	DM9000X_CS(1); 
	portEXIT_CRITICAL();
}

static __inline u16 dm9000_in_byte(u16 reg)
{
	u16 indata;
	GPIO_InitTypeDef GPIO_dm9000x;	

	
 	GPIO_dm9000x.GPIO_Pin = GPIO_Pin_All;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_IPU;
    portENTER_CRITICAL();
	GPIO_Init( DATA_PORT, &GPIO_dm9000x );   
	DM9000X_CMD(reg);			  
	DM9000X_CS(0);				  
	DM9000X_IOR(0);				  
	DM9000X_IOW(1);   
	indata = DM9000X_DATA_IN();  			  
	DM9000X_CS(1);
	portEXIT_CRITICAL();
    
	return indata;
}

static u16 ior(u16 reg)
{
	dm9000_out_byte(reg, DM9000_INDEX); 
	
	return dm9000_in_byte(DM9000_DATA); 
}

static void iow(u16 reg, u16 value)
{
	dm9000_out_byte(reg,   DM9000_INDEX);   
	dm9000_out_byte(value, DM9000_DATA); 
}

unsigned int dm9000x_read_id(void)
{
	u32 id = 0x00;								    

	id  = ior( DM9000_VIDL ) &0x00ff;      
	id |= (ior( DM9000_VIDH )&0x00ff) << 8; 
	id |= (ior( DM9000_PIDL )&0x00ff) << 16;
	id |= (ior( DM9000_PIDH )&0x00ff) << 24;
	
	dbg_string("DM9000x ID:0x%08x \r\n",id);

	if(DM9000_ID == id)
	  return  id;
	
	return 0;	 
}

void dm9000x_inital(void)
{
	unsigned char index = 0x00;
	const unsigned char *pmac = netconfig_get_pmac();

	dm9000x_gpio_inital();

	iow(DM9000_IMR, 0x80);		/* 屏蔽网卡中断 */
	
	/* 设置 GPCR(1EH) bit[0]=1，使DM9000的GPIO3为输出 */
	iow(DM9000_GPCR, 0x01);

	/* GPR bit[0]=0 使DM9000的GPIO3输出为低以激活内部PHY */
	iow(DM9000_GPR, 0x00);
	
	/* 延时2ms以上等待PHY上电 */
	dm9000_delay(10000);

	/* 软件复位 */
	iow(DM9000_NCR, 0x03);

	/* 延时20us以上等待软件复位完成 */
	dm9000_delay(5000);

	/* 复位完成，设置正常工作模式 */
	iow(DM9000_NCR, 0x00);

	/* 第二次软件复位，为了确保软件复位完全成功。此步骤是必要的 */
	iow(DM9000_NCR, 0x03);
	dm9000_delay(10000);

	iow(DM9000_NCR, 0x00);

	/*以上完成了DM9000的复位操作*/

	/* 设置 GPCR(1EH) bit[0]=1，使DM9000的GPIO3为输出 */
	iow(DM9000_GPCR, 0x01);

	/* GPR bit[0]=0 使DM9000的GPIO3输出为低以激活内部PHY */
	iow(DM9000_GPR, 0x00);

	dm9000x_read_id();
	
	/*  */
	iow(DM9000_NSR, 0x2c);/* 清除各种状态标志位 */
	
	iow(DM9000_ISR, 0x3f);/* 清除所有中断标志位 */
	
	iow(DM9000_RCR, 0x39);/* 接收控制 */
	
	iow(DM9000_TCR, 0x00);/* 发送控制 */
	
	iow(DM9000_BPTR, 0x3f);
	
	iow(DM9000_FCTR, 0x3a); 
	
	iow(DM9000_FCR, 0xff);
	
	iow(DM9000_SMCR, 0x00);
	
	for(index = 0; index < 6; index++)
	{
		iow(DM9000_PAR + index, pmac[index]);
	}
	
	dbg_string("Mac: ");
	for(index = 0; index < 6; index++)
	{
        dbg_string("%02x", ior(DM9000_PAR + index) & 0x00ff);
        if(index != 5)
            dbg_string(":");
	}
	dbg_string("\r\n");


	iow(DM9000_NSR, 0x2c);
	
	iow(DM9000_ISR, 0x3f);
	
	iow(DM9000_IMR, 0x81);
}

void dm9000x_sendpacket( uint8_t* packet, uint16_t len)
{
	uint16_t length = len;
	uint16_t io_mode;
	u16 i,Temp;		
	
	iow(DM9000_IMR, 0x80);/* 先禁止网卡中断，防止在发送数据时被中断干扰 */

	io_mode = ior(DM9000_ISR) >> 7;  
	
	if(DM9000X_BYTE_MODE == (io_mode & 0x01))
	{
		while(length--)
		{
			iow(DM9000_MWCMD, *(packet++));
		}
		/* printf("dm9000x_sendpacket:dm9000 io mode is 8bit.....!\n"); */
	}
	else 
	{
		for(i = 0; i < length; i += 2)
		{
			Temp = packet[i] | (packet[i + 1] << 8);
			iow(DM9000_MWCMD, Temp);	
		}
		/* printf("dm9000x_sendpacket:dm9000 io mode is 16bit.....!\n"); */
	}
	iow(DM9000_TXPLH, (len >> 8) & 0x0ff);
	iow(DM9000_TXPLL, len & 0x0ff);
		
	iow(DM9000_TCR, 0x01);	/* 发送数据到以太网上 */
	
	while(!(ior(DM9000_NSR) & 0x0C))
		dm9000_delay(5);    
		
	iow(DM9000_NSR, 0x2c);	/* 清除状态寄存器，由于发送数据没有设置中断，因此不必处理中断标志位 */
	
	iow(DM9000_IMR, 0x81);	/* DM9000网卡的接收中断使能 */
}

uint16_t dm9000x_receivepacket(uint8_t* packet, uint16_t maxlen)
{
	unsigned char ready;
	unsigned char rx_io_mode;
	unsigned int  rx_status = 0x00;
	unsigned int  rx_length = 0x00;
	unsigned int  rx_length_bak = 0x00;
	unsigned int  temp = 0x0000;
	unsigned int  i;
	
	ready = 0;/* 希望读取到“01H” */

	/*以上为有效数据包前的4个状态字节*/
	if(ior(DM9000_ISR) & 0x01)
	{
		iow(DM9000_ISR, 0x01);
	}

	/*清除接收中断标志位*/
 
	ready = ior(DM9000_MRCMDX); 			/* 第一次读取，一般读取到的是 00H */
	if((ready & 0x0ff) != 0x01)
	{
		ready = ior(DM9000_MRCMDX); 		/* 第二次读取，总能获取到数据 */
		if((ready & 0x01) != 0x01)
		{
			if((ready & 0x01) != 0x00) 		/* 若第二次读取到的不是 01H 或 00H ，则表示没有初始化成功 */
			{
				dm9000x_inital();			/* 重新初始化 */
			}
			return 0;
		}
	}
 
	rx_io_mode = ior(DM9000_ISR) >> 7;	  

	if(DM9000X_BYTE_MODE == (rx_io_mode & 0x01))
	{
		rx_status = ior(DM9000_MRCMD) + (ior(DM9000_MRCMD) << 8);
		rx_length = ior(DM9000_MRCMD) + (ior(DM9000_MRCMD) << 8);
		
		if(!(rx_status & 0xbf00) && (rx_length < 1522))
		{
			rx_length_bak = rx_length;
			
			while(rx_length_bak--)
			{
			    *(packet++) = ior(DM9000_MRCMD);
			}
		}
		else
		{
			return 0;
		}	
	}
   	else if(DM9000X_WORD_MODE == 0)
	{			  
		/* 前四位 是接受包的状态字节 */
		rx_status = ior(DM9000_MRCMD); 
		rx_length = ior(DM9000_MRCMD); 
	
		if(!(rx_status & 0xbf00) && (rx_length < 1522))
		{
			rx_length_bak = rx_length;
				   
			for(i = 0; i < rx_length; i += 2)
			{
				temp= ior(DM9000_MRCMD);
				packet[i] = temp & 0xff;
				packet[i + 1]=(temp >> 8) & 0xff;
			}
		}
		else
		{  
			rx_length = 0;
			return 0;
		}	
    }
    return rx_length;
}
