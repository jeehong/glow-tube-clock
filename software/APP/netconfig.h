#ifndef __LWIP_INIT_H
#define __LWIP_INIT_H

#include "stm32f10x.h"

#include "err.h"  
#include "lwip/init.h"
#include "udp.h"
#include "tcp.h"


void LwIP_Init( void );
const unsigned char *netconfig_get_pmac(void);


#endif






