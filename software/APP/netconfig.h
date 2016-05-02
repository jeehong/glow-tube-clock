#ifndef __LWIP_INIT_H
#define __LWIP_INIT_H

#include "stm32f10x.h"

#include "err.h"  
#include "lwip/init.h"
#include "etharp.h"
#include "udp.h"
#include "tcp.h"


void LwIP_Init( void );
void Time_Update(void);
void LwIP_Periodic_Handle(void *pvParameters);
void app_init(void);
void Udp_Receive(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);

#endif






