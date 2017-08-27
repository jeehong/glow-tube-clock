#include "string.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "netconfig.h"	
#include "tcpip.h"

#ifdef LWIP_DHCP
	__IO uint32_t DHCPfineTimer   = 0;
	__IO uint32_t DHCPcoarseTimer = 0;
#endif 
 	
static struct netif DM9000AEP;	   		                        		 	

static uint8_t m_mac[6];
static uint8_t serverIP[4], maskIP[4], gateIP[4];

err_t ethernetif_init( struct netif *netif );    
err_t ethernetif_input( struct netif *netif );   

/*
 * 
 * 
 */
void LwIP_Init( void )
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	
	serverIP[0] = 192;
	serverIP[1] = 168;
	serverIP[2] = 1;
	serverIP[3] = 156;
	
	maskIP[0] = 255;
	maskIP[1] = 255;
	maskIP[2] = 255;
	maskIP[3] = 0;

	gateIP[0] = 192;
	gateIP[1] = 168;
	gateIP[2] = 1;
	gateIP[3] = 1;
	
	m_mac[0] = 0x54;	
	m_mac[1] = 0x55;
	m_mac[2] = 0x58;
	m_mac[3] = 0x10;
	m_mac[4] = 0x00;
	m_mac[5] = 0x88;							    
												    
	#if LWIP_DHCP			   					   
		ipaddr.addr = 0;
		netmask.addr = 0;
		gw.addr = 0; 
	#else										
		IP4_ADDR( &ipaddr, serverIP[0], serverIP[1], serverIP[2], serverIP[3]);	
    	IP4_ADDR( &netmask, maskIP[0], maskIP[1], maskIP[2], maskIP[3]);
		IP4_ADDR( &gw, gateIP[0], gateIP[1], gateIP[2], gateIP[3]);	
	#endif
	memcpy(&DM9000AEP.hwaddr, m_mac, NETIF_MAX_HWADDR_LEN);
	DM9000AEP.hwaddr_len = NETIF_MAX_HWADDR_LEN;
	
	netif_add(&DM9000AEP, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
	
	netif_set_default(&DM9000AEP);					
	
	
	#if  LWIP_DHCP  	   		                 
		dhcp_start(&DM9000AEP);               
	#endif
	
	netif_set_up(&DM9000AEP);               
}

const unsigned char *netconfig_get_pmac(void)
{
	return m_mac;
}

