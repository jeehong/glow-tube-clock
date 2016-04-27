#ifndef __LWIPCONFIG_H
#define __LWIPCONFIG_H


//定义的MAC地址   
#define emacETHADDR0   0x54
#define emacETHADDR1   0x55
#define emacETHADDR2   0x58
#define emacETHADDR3   0x10
#define emacETHADDR4   0x00
#define emacETHADDR5   0x24
 
//定义的IP地址
#define	 emacIPADDR0  192
#define	 emacIPADDR1  168
#define	 emacIPADDR2  0
#define	 emacIPADDR3  100

//定义的网关地址
#define emacGATEWAY_ADDR0  192
#define emacGATEWAY_ADDR1  168
#define emacGATEWAY_ADDR2  0
#define emacGATEWAY_ADDR3  1

//定义的掩码地址
#define emacNET_MASK0  255
#define emacNET_MASK1  255
#define emacNET_MASK2  255
#define emacNET_MASK3  0


//#define lwipTCP_STACK_SIZE			600
//#define lwipBASIC_SERVER_STACK_SIZE	250

//#define LWIP_NOASSERT
#endif
