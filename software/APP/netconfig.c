#include "string.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "netconfig.h"	
#include "tcpip.h"

__IO uint32_t TCPTimer = 0;
__IO uint32_t DNSTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
	__IO uint32_t DHCPfineTimer   = 0;
	__IO uint32_t DHCPcoarseTimer = 0;

#endif 
 	
struct netif DM9000AEP;	   		                        		 	

uint8_t m_mac[6],serverIP[4],maskIP[4],gateIP[4];
uint16_t serverPORT;
extern err_t ethernetif_init( struct netif *netif );    
extern err_t ethernetif_input( struct netif *netif );   


char udpstr[50]= {'\0'};								
struct udp_pcb *global_udppcb;

 

/*
 * 
 * 
 */
void LwIP_Init( void )
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	
	serverIP[0] = 172;
	serverIP[1] = 10;
	serverIP[2] = 11;
	serverIP[3] = 156;
	
	maskIP[0] = 255;
	maskIP[1] = 255;
	maskIP[2] = 0;
	maskIP[3] = 0;

	gateIP[0] = 172;
	gateIP[1] = 10;
	gateIP[2] = 11;
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





/*void app_init(void) 
{
	struct tcp_pcb *tcppcb;
	
	tcppcb = tcp_new();
	

	tcp_bind(tcppcb, IP_ADDR_ANY, serverPORT);
	
	tcppcb = tcp_listen(tcppcb);
		
	tcp_accept(tcppcb, tcp_appcall);				                			
}*/
