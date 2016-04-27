#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "netconfig.h"	

//#include "anbus.h"

struct netif netif;
__IO uint32_t TCPTimer = 0;
__IO uint32_t DNSTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
	__IO uint32_t DHCPfineTimer   = 0;
	__IO uint32_t DHCPcoarseTimer = 0;

#endif 
 	
struct netif DM9000AEP;	   		                        	//����Ӳ���ӿڽṹ����
//uint8_t macaddress[6]={0x54,0x55,0x58,0x10,0x00,0x88};	//����Ӳ������ӿڵ�mac��ַ

uint8_t m_mac[6],serverIP[4],maskIP[4],gateIP[4];

extern err_t ethernetif_init( struct netif *netif );    //��̫����ʼ������
extern err_t ethernetif_input( struct netif *netif );   //��̫�����뺯��

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
void LwIP_Periodic_Handle(__IO uint32_t localtime)
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

 

/*
 * ��������LWIP_Init
 * ����  ����ʼ��LWIPЭ��ջ����Ҫ�ǰ�DM9000AEP��LWIP��������������IP��MAC��ַ���ӿں��� 			
 * ����  ����
 * ���  : ��
 * ����  ���ⲿ����
 */
void LwIP_Init( void )
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	
	serverIP[0] = 172;
	serverIP[1] = 10;
	serverIP[2] = 11;
	serverIP[3] = 73;
	
	maskIP[0] = 255;
	maskIP[1] = 255;
	maskIP[2] = 0;
	maskIP[3] = 0;

	serverIP[0] = 172;
	serverIP[1] = 10;
	serverIP[2] = 11;
	serverIP[3] = 1;
	
	lwip_init();								    //����LWIP��ʼ��������ʼ������ӿڽṹ�������ڴ�ء�pbuf�ṹ�� 
												    
	#if LWIP_DHCP			   					    //��ʹ��DHCPЭ��
		ipaddr.addr = 0;
		netmask.addr = 0;
		gw.addr = 0; 
	#else										
		IP4_ADDR( &ipaddr, serverIP[0], serverIP[1], serverIP[2], serverIP[3]);	
    IP4_ADDR( &netmask, maskIP[0], maskIP[1], maskIP[2], maskIP[3]);
		IP4_ADDR( &gw, gateIP[0], gateIP[1], gateIP[2], 1);	
	#endif
	
	//��ʼ��DM9000AEP��LWIP�Ľӿڣ�����Ϊ����ӿڽṹ�塢ip��ַ���������롢���ء�������Ϣָ�롢��ʼ�����������뺯��  
	netif_add(&DM9000AEP, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
		
	netif_set_default(&DM9000AEP);					//��DM9000AEP����ΪĬ������ 
	
	
	#if LWIP_DHCP	   		                     	//��ʹ����DHCP
	/*  Creates a new DHCP client for this interface on the first call.
	Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	the predefined regular intervals after starting the client.
	You can peek in the netif->dhcp struct for the actual DHCP status.*/
		dhcp_start(&DM9000AEP);                      //����DHCP
	#endif
	
	netif_set_up(&DM9000AEP);                        //ʹ��Ӳ������оƬ�ӿ�����DM9000AEP
}
