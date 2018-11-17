#include "string.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "netconfig.h"	
#include "tcpip.h"


#define UDP_SERVER_PORT    	50000   /* define the UDP local connection port */
#define UDP_CLIENT_PORT    	50000   /* define the UDP remote connection port */
#define TCP_SERVER_PORT    	50000	/* define the TCP connection port */

#define IP_ADDRESS0  192
#define IP_ADDRESS1  168
#define IP_ADDRESS2  1
#define IP_ADDRESS3  5

#define MASK_ADDRESS0 255
#define MASK_ADDRESS1 255
#define MASK_ADDRESS2 255
#define MASK_ADDRESS3 0

#define GW_ADDRESS0  192
#define GW_ADDRESS1  168
#define GW_ADDRESS2  1
#define GW_ADDRESS3  1

#define MAC_ADDRESS0  0x54
#define MAC_ADDRESS1  0x55
#define MAC_ADDRESS2  0x58
#define MAC_ADDRESS3  0x10
#define MAC_ADDRESS4  0x00
#define MAC_ADDRESS5  0x88

extern err_t ethernetif_init( struct netif *netif );
extern err_t ethernetif_input(struct netif *netif);

void server_init(void);
static void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

static struct netif lwip_dev;

void LwIP_Init( void )
{
	struct ip_addr ipaddr, netmask, gw;

	IP4_ADDR( &ipaddr, IP_ADDRESS0, IP_ADDRESS1, IP_ADDRESS2, IP_ADDRESS3);	
    IP4_ADDR( &netmask, MASK_ADDRESS0, MASK_ADDRESS1, MASK_ADDRESS2, MASK_ADDRESS3);
	IP4_ADDR( &gw, GW_ADDRESS0, GW_ADDRESS1, GW_ADDRESS2, GW_ADDRESS3);	

	lwip_dev.hwaddr[0] = MAC_ADDRESS0;
	lwip_dev.hwaddr[1] = MAC_ADDRESS1;
	lwip_dev.hwaddr[2] = MAC_ADDRESS2;
	lwip_dev.hwaddr[3] = MAC_ADDRESS3;
	lwip_dev.hwaddr[4] = MAC_ADDRESS4;
	lwip_dev.hwaddr[5] = MAC_ADDRESS5;
	
	lwip_dev.hwaddr_len = NETIF_MAX_HWADDR_LEN;

	tcpip_init(NULL,  NULL);
	netif_add(&lwip_dev, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
	netif_set_default(&lwip_dev);					
	netif_set_up(&lwip_dev);
	/* establish network server of UDP and TCP */
	server_init();
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */
static void server_init(void)
{
	struct tcp_pcb *tpcb;    
	struct udp_pcb *upcb;  	 

	/* Create a new TCP control block  */
	tpcb = tcp_new();

	/* Assign to the new pcb a local IP address and a port number */
	/* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
	tcp_bind(tpcb, IP_ADDR_ANY, TCP_SERVER_PORT);

	/* Set the connection to the LISTEN state */
	tpcb = tcp_listen(tpcb);

	/* Specify the function to be called when a connection is established */	
	tcp_accept(tpcb, tcp_server_accept);	

	/* Create a new UDP control block  */
	upcb = udp_new();

	/* Bind the upcb to the UDP_PORT port */
	/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
	udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);

	/* Set a receive callback for the upcb */
	udp_recv(upcb, udp_server_callback, NULL);  
}

static void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	/* uint8_t iptab[4]; */

	/* We have received the UDP Echo from a client */
	/* read its IP address */
	/* iptab[0] = (uint8_t)((uint32_t)(addr->addr) >> 24);
	iptab[1] = (uint8_t)((uint32_t)(addr->addr) >> 16);
	iptab[2] = (uint8_t)((uint32_t)(addr->addr) >> 8);
	iptab[3] = (uint8_t)((uint32_t)(addr->addr)); */

	/* Connect to the remote client */
	udp_connect(upcb, addr, UDP_CLIENT_PORT);

	/* Tell the client that we have accepted it */
	udp_send(upcb, p);

	/* free the UDP connection, so we can accept new clients */
	udp_disconnect(upcb);

	/* Free the p buffer */
	pbuf_free(p);
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	/* Specify the function that should be called when the TCP connection receives data */
	tcp_recv(pcb, tcp_server_recv);

	return ERR_OK;  
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	struct pbuf *tcp_send_pbuf;
	struct name *name = (struct name *)arg;

	if (p != NULL)
	{
		tcp_recved(pcb, p->tot_len);

		tcp_send_pbuf = p;

		tcp_write(pcb, "\r\n", strlen("\r\n"), 1);

		tcp_write(pcb, tcp_send_pbuf->payload, tcp_send_pbuf->len, 1);

		pbuf_free(p);
	}
	else if (err == ERR_OK)
	{
		mem_free(name);
		return tcp_close(pcb);
	}

	return ERR_OK;
}

