#include "stm32f10x.h"
#include <stdio.h>

#include "DM9000x.h"
#include "netif.h"

#include "mid_dbg.h"

#define  EXINIT                    GPIO_Pin_0    // PB0
#define  CMD                       GPIO_Pin_3 	 // 
#define	 CS                        GPIO_Pin_4	 // 
#define  IOW                       GPIO_Pin_7   // 
#define  IOR                       GPIO_Pin_6   // 

#define  DATA                      GPIOB
#define  CMD_PORT                  GPIOA
#define  CS_PORT                   GPIOC
#define  IOW_PORT                  GPIOC
#define  IOR_PORT                  GPIOC
#define  EXINT_PORT                GPIOB
#define  CONTROL_PORT              GPIOC
#define  DATA_PORT                 GPIOB         // PB0~15  

#define  GPIOB_CRL                 (*(volatile unsigned long *)GPIOB_BASE) //GPIOB = 0x40011000
#define  GPIOB_CRH				   (*(volatile unsigned long *)(GPIOB_BASE + 8))
#define  DATA_PORT_CRL             GPIOB_CRL

#define  DM9000_IO                 0    
#define  DM9000_DATA               1
#define  DM9000_INDEX              0

#define  DM9000X_CMD(dcmd)         (dcmd == 0x01)                 ?          \
                                   (CMD_PORT->ODR |= GPIO_Pin_3) :          \
                                   (CMD_PORT->ODR &= ~GPIO_Pin_3)           

#define  DM9000X_CS(dcs)           (dcs == 0x01)                  ?          \
                                   (CS_PORT->ODR |= GPIO_Pin_4)  :          \
                                   (CS_PORT->ODR &= ~GPIO_Pin_4)           

#define  DM9000X_IOW(dwrite)       (dwrite == 0x01)               ?          \
                                   (IOW_PORT->ODR |= GPIO_Pin_7) :          \
                                   (IOW_PORT->ODR &= ~GPIO_Pin_7)   
                                   
#define  DM9000X_IOR(dread)        (dread == 0x01)                ?          \
                                   (IOR_PORT->ODR |= GPIO_Pin_6) :          \
                                   (IOR_PORT->ODR &= ~GPIO_Pin_6)   

#define  DM9000X_DATA_OUT(ddata)   (DATA_PORT->ODR = ddata)    
#define  DM9000X_DATA_IN()         (DATA_PORT->IDR)

#define DM9000X_BYTE_MODE          0x01
#define DM9000X_WORD_MODE          0x00


static dm9000_delay(u32 delay)
{
       while (delay--);
}

void dm9000x_gpio_inital(void)
{
    GPIO_InitTypeDef GPIO_dm9000x;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE); 	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE); 

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

	/* Configure control io as output push-pull */
	GPIO_dm9000x.GPIO_Pin = CMD | CS | IOW | IOR;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( CONTROL_PORT, &GPIO_dm9000x );


	/* Configure control io as output push-pull */
	GPIO_dm9000x.GPIO_Pin = CMD ;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( CMD_PORT, &GPIO_dm9000x );
	/* Configure data io as inout */
	GPIO_dm9000x.GPIO_Pin = GPIO_Pin_All;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( DATA_PORT, &GPIO_dm9000x );

	GPIO_dm9000x.GPIO_Pin = EXINIT;
    GPIO_dm9000x.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_dm9000x.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(EXINT_PORT,&GPIO_dm9000x);
}

static __inline void dm9000_out_byte(u16 outdata, u16 reg)
{
	GPIO_InitTypeDef GPIO_dm9000x;

	GPIO_dm9000x.GPIO_Pin = GPIO_Pin_All;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init( DATA_PORT, &GPIO_dm9000x );
	DM9000X_DATA_OUT(outdata);
	DM9000X_CMD(reg);
	DM9000X_CS(0);
	DM9000X_IOR(1);
	DM9000X_IOW(0);

	DM9000X_IOW(1);
	DM9000X_CS(1);
}

static __inline u16 dm9000_in_byte(u16 reg)
{
	u16 indata;
    GPIO_InitTypeDef GPIO_dm9000x;

 	GPIO_dm9000x.GPIO_Pin = GPIO_Pin_All;
	GPIO_dm9000x.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_dm9000x.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init( DATA_PORT, &GPIO_dm9000x );
	  
    DM9000X_CMD(reg);
    DM9000X_CS(0);
    DM9000X_IOR(0);
    DM9000X_IOW(1);
	indata = DM9000X_DATA_IN();
    DM9000X_IOW(1);
    DM9000X_CS(1);

    return indata;
}

static u16 ior(u16 reg)
{
    u16 data = 0; 
	dm9000_out_byte(reg, DM9000_INDEX); 
	data=dm9000_in_byte(DM9000_DATA); 
	   
    return data;
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
	
//	printf("DM9000x ID:0x%x \n",id); 

	if(DM9000_ID == id)
	     return  id;
	
	return 0;	 
}

static uint8_t dm9000_mac[NETIF_MAX_HWADDR_LEN] =  {0};

void dm9000x_inital(uint8_t *macaddr)
{
	unsigned char i = 0x00;

	dm9000x_gpio_inital();
	dm9000_delay (10000);
	iow(DM9000_GPCR, 0x01);
    iow(DM9000_GPR, 0x00);
    dm9000_delay(30000);
    iow(DM9000_NCR, 0x03);
	dm9000_delay(10000);
    iow(DM9000_NCR, 0x00);
    iow(DM9000_NCR, 0x03);
	dm9000_delay(10000);
    iow(DM9000_NCR, 0x00);
	iow(DM9000_GPCR, 0x01);
    iow(DM9000_GPR, 0x00);
	dm9000x_read_id();
    iow(DM9000_NSR, 0x2c);
    iow(DM9000_ISR, 0x3f);
    iow(DM9000_RCR, 0x39);
    iow(DM9000_TCR, 0x00);
    iow(DM9000_BPTR, 0x3f);
    iow(DM9000_FCTR, 0x3a); 
    iow(DM9000_FCR, 0xff);
    iow(DM9000_SMCR, 0x00);

    for(i = 0; i < NETIF_MAX_HWADDR_LEN; i++)
    {
		dm9000_mac[i] = macaddr[i];
    	iow(DM9000_PAR + i, macaddr[i]);
	}

    iow(DM9000_NSR, 0x2c);
    iow(DM9000_ISR, 0x3f);
    iow(DM9000_IMR, 0x81);
}

void dm9000x_sendpacket( uint8_t* packet, uint16_t len)
{
    uint16_t length = len;
	uint16_t io_mode;
	u16 i,Temp;		
    iow(DM9000_IMR, 0x80);

	io_mode = ior(DM9000_ISR) >> 7;
	
	if(DM9000X_BYTE_MODE == (io_mode & 0x01))
	{
		while(length--)
		{
			iow(DM9000_MWCMD, *(packet++));
		}
		dbg_string("dm9000x_sendpacket:dm9000 io mode is 8bit.....!\r\n");
	}
    else 
	{
        for(i = 0; i < length; i += 2)
		{
			Temp = packet[i]|( packet[i+1]<< 8);
			iow(DM9000_MWCMD, Temp);	
		}
		//dbg_string("dm9000x_sendpacket:dm9000 io mode is 16bit.....!\r\n");
	}
    iow(DM9000_TXPLH, (len>>8) & 0x0ff);
    iow(DM9000_TXPLL, len & 0x0ff);
	
    iow(DM9000_TCR, 0x01);

	while(!(ior(DM9000_NSR)&0x0C)) ;

    iow(DM9000_NSR, 0x2c);
    iow(DM9000_IMR, 0x81);
}

uint16_t dm9000x_receivepacket(uint8_t* packet, uint16_t maxlen)
{
    unsigned char ready;
	unsigned char rx_io_mode;
	unsigned int  rx_status = 0x00;
	unsigned int  rx_length = 0x00;
	unsigned int  rx_length_bak = 0x00;
	unsigned int  temp=0x0000;
	unsigned int  i;

    ready = 0;
    if(ior(DM9000_ISR) & 0x01)
    {
        iow(DM9000_ISR, 0x01);
    }
    ready = ior(DM9000_MRCMDX);
    if((ready & 0x0ff) != 0x01)
	{
        ready = ior(DM9000_MRCMDX);
        if((ready & 0x01) != 0x01)
        {
			if((ready & 0x01) != 0x00) 
            {
                 iow(DM9000_IMR, 0x80);
                 dm9000x_inital(dm9000_mac);
				 dbg_string("System is restart\r\n");
                 iow(DM9000_IMR, 0x81);
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
   	else if(DM9000X_WORD_MODE == (rx_io_mode & 0x00))
	{
		rx_status = ior(DM9000_MRCMD);
		rx_length = ior(DM9000_MRCMD);
	
		if(!(rx_status & 0xbf00) && (rx_length < 1522))
		{
			rx_length_bak = rx_length;
			for(i = 0; i < rx_length; i += 2)
			{
				temp= ior(DM9000_MRCMD);
				packet[i]=temp & 0xff;
				packet[i+1]=(temp>>8) & 0xff;
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


