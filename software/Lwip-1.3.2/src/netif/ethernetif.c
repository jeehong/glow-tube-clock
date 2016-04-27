/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "stm32f10x.h"
#include <string.h>
#include "err.h"
#include "netif.h"

#include "etharp.h"

#include "mem.h"
#include "sys.h" 

#include "DM9000x.h"              //网卡芯片DM9000AEP底层驱动函数

#define IFNAME0 'S'
#define IFNAME1 'T'

extern  uint8_t m_mac[6];	        //存放硬件网卡地址值大小

unsigned char Data_Buf[1516];	    //存放接收数据缓冲器
unsigned char Tx_Data_Buf[1516]; //存放发送数据缓冲器


struct ethernetif
{
	struct eth_addr *ethaddr;
};

static void low_level_init( struct netif *netif )	  //底层硬件驱动网卡初始化函数
{
	
	netif->hwaddr_len = 6;						//设置硬件网卡地址值长度 

	netif->hwaddr[0] = m_mac[0];			//硬件网卡地址值大小
	netif->hwaddr[1] = m_mac[1];
	netif->hwaddr[2] = m_mac[2];
	netif->hwaddr[3] = m_mac[3];
	netif->hwaddr[4] = m_mac[4];
	netif->hwaddr[5] = m_mac[5];

	netif->mtu = 1500;		            // 最大传输单元

	/* broadcast capability */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	/* Initialise the EMAC.  This routine contains code that polls status bits.  
	If the Ethernet cable is not plugged in then this can take a considerable 
	time.  To prevent this starving lower priority tasks of processing time we
	lower our priority prior to the call, then raise it back again once the
	initialisation is complete. */
                 
  dm9000x_inital(netif->hwaddr);     //底层硬件网卡芯片DM9000AEP初始化函数
}		   
/*-----------------------------------------------------------*/

/*
 * low_level_output(): Should do the actual transmission of the packet. The 
 * packet is contained in the pbuf that is passed to the function. This pbuf 
 * might be chained.
 */
static err_t low_level_output( struct netif *netif, struct pbuf *p )  //底层硬件网卡DM9000AEP发送数据函数
{     
	struct pbuf *q;
	int len = 0;   		                 //发送数据长度

	for(q = p; q != NULL; q = q->next) 
	{
		memcpy(&Tx_Data_Buf[len], (u8_t*)q->payload, q->len);  //建立链表指针存放发送的数据
		len = len + q->len;
	}		
														
	dm9000x_sendpacket(Tx_Data_Buf,len);//通过底层硬件网卡DM9000AEP发送数据包

	return ERR_OK;
}


/*
 * low_level_input(): Should allocate a pbuf and transfer the bytes of the 
 * incoming packet from the interface into the pbuf. 
 */

static struct pbuf *low_level_input( struct netif *netif )	//底层硬件网卡DM9000AEP接收数据函数
{	                                                   

	struct pbuf *q,*p = NULL;
	u16 Len = 0; 
	int i =0;                  	
	
	Len =dm9000x_receivepacket(Data_Buf,1516);   //返回接收到的数据包长度	

	if ( Len == 0 ) return 0;
		
	p = pbuf_alloc(PBUF_RAW, Len, PBUF_POOL);

	if (p != NULL) 
	{
		for (q = p; q != NULL; q = q->next) 
		{	 
			memcpy((u8_t*)q->payload, (u8_t*)&Data_Buf[i], q->len);
			i = i + q->len;
		}
		if( i != p->tot_len )                      //相等时表明到了数据尾，发送完成
		{ 
			return 0;
    }                                 
	}

	return p;
}

/*
 * ethernetif_input(): This function should be called when a packet is ready to 
 * be read from the interface. It uses the function low_level_input() that 
 * should handle the actual reception of bytes from the network interface.
 */	
err_t  ethernetif_input(struct netif *netif)  
{  	
	err_t err = ERR_OK;		
	struct pbuf *p = NULL;
						
	/* move received packet into a new pbuf */
	p = low_level_input(netif);

	if (p == NULL) 
		return ERR_MEM;   //-1

	err = netif->input(p, netif);
	if (err != ERR_OK)
	{
		//LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
		pbuf_free(p);
		p = NULL;
	}

	return err;
}
  
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init( struct netif *netif )
{
	struct ethernetif   *ethernetif;

	ethernetif = mem_malloc( sizeof(struct ethernetif) );

	if( ethernetif == NULL )
	{
		LWIP_DEBUGF( NETIF_DEBUG, ("ethernetif_init: out of memory\n\r") );
		return ERR_MEM;
	}

	netif->state = ethernetif;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;

	netif->output = etharp_output;

	netif->linkoutput = low_level_output;

	ethernetif->ethaddr = ( struct eth_addr * ) &( netif->hwaddr[0] );

	low_level_init( netif );

	return ERR_OK;
}
/*************************************************************************************************************/


