/*
    FreeRTOS V6.0.2 - Copyright (C) 2010 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS eBook                                  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/*
	Implements a simplistic WEB server.  Every time a connection is made and
	data is received a dynamic page that shows the current TCP/IP statistics
	is generated and returned.  The connection is then closed.

	This file was adapted from a FreeRTOS lwIP slip demo supplied by a third
	party.
*/

/*
	Changes from V3.2.2

	+ Changed the page returned by the lwIP WEB server demo to display the 
	  task status table rather than the TCP/IP statistics.
*/


/* Standard includes. */ 
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>
#include "config.h"
/* Scheduler includes. */
/* #include "semphr.h" */

/* Demo includes. */
#include "LwIPEntry.h"

/* lwIP includes. */
#include "lwip/api.h" 
#include "lwip/tcpip.h"
#include "lwip/memp.h" 
#include "lwip/stats.h"
#include "netif/loopif.h"
#include "lwip/ip.h"

/*------------------------------------------------------------*/

/*------------------------------------------------------------*/

 void vlwIPInit( void )
{
    /* Initialize lwIP and its interface layer. */
	sys_init();
	mem_init();								
	memp_init();
	pbuf_init(); 
	netif_init();
	ip_init();
	tcpip_init( NULL, NULL );
}
/*------------------------------------------------------------*/
struct netif EMAC_if;
extern   void  ethernetif_input( void *pReserved);
__inline void SetLwIP(void)
{
extern err_t ethernetif_init( struct netif *netif );
struct ip_addr xIpAddr, xNetMast, xGateway;
	/* Create and configure the EMAC interface. */
	IP4_ADDR(&xIpAddr,emacIPADDR0,emacIPADDR1,emacIPADDR2,emacIPADDR3);
	IP4_ADDR(&xNetMast,emacNET_MASK0,emacNET_MASK1,emacNET_MASK2,emacNET_MASK3);
	IP4_ADDR(&xGateway,emacGATEWAY_ADDR0,emacGATEWAY_ADDR1,emacGATEWAY_ADDR2,emacGATEWAY_ADDR3);
	netif_add(&EMAC_if, &xIpAddr, &xNetMast, &xGateway, NULL, ethernetif_init, tcpip_input);
	/* make it the default interface */
    netif_set_default(&EMAC_if);
	/* bring it up */
    netif_set_up(&EMAC_if);

}




void LwIPEntry(void * pvArg)
{
  	struct netconn  *__pstConn, *__pstNewConn;
	struct netbuf	*__pstNetbuf;

	//* 初始化LwIP
	vlwIPInit();	
	//* 设置LwIP，包括添加配置网络接口、建立接收任务等工作
	SetLwIP();
	__pstConn = netconn_new(NETCONN_TCP);
	netconn_bind(__pstConn, NULL, 80);
	netconn_listen(__pstConn);
   /* Initilaize the HelloWorld module */
 	while(1)
	{
		__pstNewConn = netconn_accept(__pstConn);
		
		if(__pstNewConn != NULL)
		{			
			__pstNetbuf = netconn_recv(__pstNewConn);
			if(__pstNetbuf != NULL)
			{
				netconn_write(__pstNewConn, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n", 44, NETCONN_COPY);
				netconn_write(__pstNewConn, "<body><h1>这是LWIP TCP测试！</h1></body>", 40, NETCONN_COPY);
				
				netbuf_delete(__pstNetbuf);	
			}
			
			netconn_close(__pstNewConn);
			while(netconn_delete(__pstNewConn) != ERR_OK)
				vTaskDelay(1);
		}


//	ethernetif_input(NULL);
//  tcp_tmr();	
 // etharp_tmr();  
     }			   
}


