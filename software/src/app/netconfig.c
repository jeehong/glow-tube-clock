#include <string.h>
#include <stdio.h>

#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/sockets.h"

#include "netif/etharp.h"

#include "netconfig.h"
#include "mid_rtc.h"
#include "mid_dbg.h"
#include "app_inc.h"

#define UDP_SERVER_PORT    	50000   /* define the UDP local connection port */
#define UDP_CLIENT_PORT    	50000   /* define the UDP remote connection port */
#define TCP_SERVER_PORT    	50000	/* define the TCP connection port */

#define IP_ADDRESS0  192
#define IP_ADDRESS1  168
#define IP_ADDRESS2  1
#define IP_ADDRESS3  10

#if (1)
#define SVR_MQTT_IPADDR0  94
#define SVR_MQTT_IPADDR1  191
#define SVR_MQTT_IPADDR2  29
#define SVR_MQTT_IPADDR3  119
#else
#define SVR_MQTT_IPADDR0  192
#define SVR_MQTT_IPADDR1  168
#define SVR_MQTT_IPADDR2  1
#define SVR_MQTT_IPADDR3  8
#endif

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


static struct netif lwip_dev;

static void server_init(int *fd)
{
    struct sockaddr_in server;
    int opt =1;

    /*
     * Get a socket for accepting connections.
     */
    if ((*fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        dbg_string("Socket()\r\n");
        close(*fd);
        return;
    }

    setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt));

    /*
     * Bind the socket to the server address.
     */
    server.sin_family = AF_INET;
    server.sin_port   = htons(TCP_SERVER_PORT);
    server.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(*fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        dbg_string("Bind()\r\n");
        close(*fd);
        return;
    }

    /*
     * Listen for connections. Specify the backlog as 1.
     */
    if (listen(*fd, 5) != 0) {
        dbg_string("Listen()\r\n");
        close(*fd);
        return;
    }
}

static void network_monitor(void *parame)
{
    int server_fd = -1;
    
	struct sockaddr_in from;
	socklen_t len;
	U32 length = 0;
	int client_fd = 0;
    char buffer[400];
    struct rtc_time pt;

    /* waiting for the network to establish */
    while(lwip_dev.ip_addr.addr == 0) {
        vTaskDelay(100);
    }

    server_init(&server_fd);
    
    
	for(;;)	{
        len = sizeof(from);
		client_fd = accept(server_fd, (struct sockaddr*)&from, &len);
		dbg_string("fd:%d Connect from %s:%d\r\n", client_fd, inet_ntoa(from.sin_addr), from.sin_port);
        while(client_fd >= 0) {
    	    length = recv(client_fd, buffer, 100, 0);
			//dbg_string("fd:%d Receive[%d] %s\r\n", client_fd, length, buffer);
            if(length > 0) {
                mid_rtc_get_time(&pt);
                sprintf(buffer, "Date: 20%02d-%02d-%02d %d %02d:%02d:%02d Temperature: %3.2fC Humidity: %3.2f%%",
                pt.year, pt.mon, pt.mday, pt.wday, pt.hour, pt.min, pt.sec,
                app_th_get_data(TEMP), app_th_get_data(HUM));
                send(client_fd, buffer, strlen(buffer), 0);
            } else {
				dbg_string("fd:%d leave from %s:%d\r\n", client_fd, inet_ntoa(from.sin_addr), from.sin_port);
                close(client_fd);
                client_fd = -1;
            }
        }
	}	
}

void LwIP_Init( void )
{
    struct ip_addr ipaddr, netmask, gw;
    
#if LWIP_DHCP
	IP4_ADDR( &ipaddr, 0, 0, 0, 0);	
    IP4_ADDR( &netmask, 0, 0, 0, 0);
	IP4_ADDR( &gw, 0, 0, 0, 0);	
#else
	IP4_ADDR( &ipaddr, IP_ADDRESS0, IP_ADDRESS1, IP_ADDRESS2, IP_ADDRESS3);	
    IP4_ADDR( &netmask, MASK_ADDRESS0, MASK_ADDRESS1, MASK_ADDRESS2, MASK_ADDRESS3);
	IP4_ADDR( &gw, GW_ADDRESS0, GW_ADDRESS1, GW_ADDRESS2, GW_ADDRESS3);	
#endif

	lwip_dev.hwaddr[0] = MAC_ADDRESS0;
	lwip_dev.hwaddr[1] = MAC_ADDRESS1;
	lwip_dev.hwaddr[2] = MAC_ADDRESS2;
	lwip_dev.hwaddr[3] = MAC_ADDRESS3;
	lwip_dev.hwaddr[4] = MAC_ADDRESS4;
	lwip_dev.hwaddr[5] = MAC_ADDRESS5;
	
	lwip_dev.hwaddr_len = NETIF_MAX_HWADDR_LEN;

    tcpip_init(NULL, NULL);
	netif_add(&lwip_dev, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
	netif_set_default(&lwip_dev);
    if (netif_is_up(&lwip_dev)) {
        /* When the netif is fully configured this function must be called.*/
        netif_set_up(&lwip_dev);
    } else {
        /* When the netif link is down this function must be called */
        netif_set_down(&lwip_dev);
    }
#if LWIP_DHCP
    dhcp_start(&lwip_dev );
#endif
    xTaskCreate((pdTASK_CODE)network_monitor, "network_monitor", 400, NULL, tskIDLE_PRIORITY + 3, NULL);
}

void LwIP_get_network_info(struct ip_addr *ipaddr, 
                                    struct ip_addr *netmask, 
                                    struct ip_addr *gw, 
                                    u8_t *mac)
{
    if(ipaddr != NULL) {
        ipaddr->addr = lwip_dev.ip_addr.addr;
    }
    if(netmask != NULL) {
        netmask->addr = lwip_dev.netmask.addr;
    }
    if(gw != NULL) {
        gw->addr = lwip_dev.gw.addr;
    }
    if(mac != NULL) {
        memcpy(mac, &(lwip_dev.hwaddr[0]), NETIF_MAX_HWADDR_LEN);
    }
}

