#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "netconfig.h"	
#include "tcpip.h"
#include "clock-arch.h"

#define	localtime		sys_ticks

struct netif netif;
__IO uint32_t TCPTimer = 0;
__IO uint32_t DNSTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
	__IO uint32_t DHCPfineTimer   = 0;
	__IO uint32_t DHCPcoarseTimer = 0;

#endif 
 	
struct netif DM9000AEP;	   		                        		 /* 网络硬件接口结构特性 */
//uint8_t macaddress[6]={0x54,0x55,0x58,0x10,0x00,0x88};		/* 设置硬件网络接口的mac地址*/

uint8_t m_mac[6],serverIP[4],maskIP[4],gateIP[4];

extern err_t ethernetif_init( struct netif *netif );    /* 以太网初始化函数 */
extern err_t ethernetif_input( struct netif *netif );   /* 以太网输入函数 */

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(void *pvParameters)
{     
	while(1)
	{
		if(ethernetif_input(&DM9000AEP) == ERR_OK) 	  //ÂÖÑ¯ÊÇ·ñ½ÓÊÕµ½Êý¾Ý
		{
			/* TCP periodic process every 250 ms */
			if ((localtime - TCPTimer) >= TCP_TMR_INTERVAL)
			{
				TCPTimer =  localtime;
				tcp_tmr();
			}
			
			/* DNS periodic process every 1000 ms */
			if ((localtime - DNSTimer) >= DNS_TMR_INTERVAL)
			{
				DNSTimer =  localtime;
				dns_tmr();
			}
			
			/* ARP periodic process every 5s */
			if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
			{
				ARPTimer =  localtime;
				etharp_tmr();
			}

		#if LWIP_DHCP
			/* Fine DHCP periodic process every 500ms */
			if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
			{
				DHCPfineTimer =  localtime;
				dhcp_fine_tmr();
			}
			/* DHCP Coarse periodic process every 60s */
			if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
			{
				DHCPcoarseTimer =  localtime;
				dhcp_coarse_tmr();
			}
		#endif
		}
	}
}

 

/*
 * 函数名：LWIP_Init
 * 描述  ：初始化LWIP协议栈，主要是把DM9000AEP与LWIP连接起来。包括IP、MAC地址，接口函数 			
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
void LwIP_Init( void )
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	
	serverIP[0] = 172;
	serverIP[1] = 10;
	serverIP[2] = 11;
	serverIP[3] = 157;
	
	maskIP[0] = 255;
	maskIP[1] = 255;
	maskIP[2] = 0;
	maskIP[3] = 0;

	gateIP[0] = 172;
	gateIP[1] = 10;
	gateIP[2] = 11;
	gateIP[3] = 1;
	
	lwip_init();								    /* 调用LWIP初始化函数初始化网络接口结构体链表、内存池、pbuf结构体 */
												    
	#if LWIP_DHCP			   					    /* 若使用DHCP协议 */
		ipaddr.addr = 0;
		netmask.addr = 0;
		gw.addr = 0; 
	#else										
		IP4_ADDR( &ipaddr, serverIP[0], serverIP[1], serverIP[2], serverIP[3]);	
    IP4_ADDR( &netmask, maskIP[0], maskIP[1], maskIP[2], maskIP[3]);
		IP4_ADDR( &gw, gateIP[0], gateIP[1], gateIP[2], 1);	
	#endif
	
	/* 初始化DM9000AEP与LWIP的接口，参数为网络接口结构体、ip地址、子网掩码、网关、网卡信息指针、初始化函数、输入函数 */
	netif_add(&DM9000AEP, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
	//netif_add(&DM9000AEP, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
	
	netif_set_default(&DM9000AEP);					/* 把DM9000AEP设置为默认网卡 */
	
	
	#if LWIP_DHCP	   		                    /* 若使用了DHCP */
	/*  Creates a new DHCP client for this interface on the first call.
	Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	the predefined regular intervals after starting the client.
	You can peek in the netif->dhcp struct for the actual DHCP status.*/
		dhcp_start(&DM9000AEP);               /* 启动DHCP */
	#endif
	
	netif_set_up(&DM9000AEP);               /* 使能硬件网络芯片接口驱动DM9000AEP */
}
