#ifndef __LWIPCONFIG_H
#define __LWIPCONFIG_H


//�����MAC��ַ   
#define emacETHADDR0   0x54
#define emacETHADDR1   0x55
#define emacETHADDR2   0x58
#define emacETHADDR3   0x10
#define emacETHADDR4   0x00
#define emacETHADDR5   0x24
 
//�����IP��ַ
#define	 emacIPADDR0  192
#define	 emacIPADDR1  168
#define	 emacIPADDR2  0
#define	 emacIPADDR3  100

//��������ص�ַ
#define emacGATEWAY_ADDR0  192
#define emacGATEWAY_ADDR1  168
#define emacGATEWAY_ADDR2  0
#define emacGATEWAY_ADDR3  1

//����������ַ
#define emacNET_MASK0  255
#define emacNET_MASK1  255
#define emacNET_MASK2  255
#define emacNET_MASK3  0


//#define lwipTCP_STACK_SIZE			600
//#define lwipBASIC_SERVER_STACK_SIZE	250

//#define LWIP_NOASSERT
#endif
