#ifndef __LWIP_INIT_H
#define __LWIP_INIT_H

#include "lwip/ip_addr.h"

void LwIP_Init( void );
void LwIP_get_network_info(struct ip_addr *ipaddr, 
                            struct ip_addr *netmask, 
                            struct ip_addr *gw, 
                            u8_t *mac);
void mqtt_do_publish(char const *data, u16_t length);

#endif

