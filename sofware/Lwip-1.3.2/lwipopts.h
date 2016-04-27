/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 * Modifications: Christian Walter <wolti@sil.at>
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
/* ------------------------ Generic options ------------------------------- */
//#undef  LWIP_NOASSERT
#define LWIP_NOASSERT
#define LWIP_NOERROR

/*********************************************************************************************************
**-------------------------------Socket参数配置：Socket options----------------------------
*********************************************************************************************************/
#define LWIP_SOCKET                     1
#define LWIP_COMPAT_SOCKETS             1
//#define NO_SYS    											0	 		/* 使用操作系统 */
#define LWIP_DNS												1
/*********************************************************************************************************
**-------------------------------Sequential layer options----------------------------
*********************************************************************************************************/
#define LWIP_NETCONN                    1

#define NO_SYS_NO_TIMERS   1

#define LWIP_DHCP     0

/*************************************************************************************************************************************/
#define ETHARP_TMR_INTERVAL             5000  /* Time in milliseconds to perform ARP processing */

/*********************************************************************************************************/

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4		/* Align memory on 4 byte boundery (32-bit) */

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                2000

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           20
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        4
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 8
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        8
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    6


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          16

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       512
/*********************************************************************************************************
**-------------------------------TCP参数配置：TCP options----------------------------
*********************************************************************************************************/

 #define MEMP_NUM_REASSDATA              5 
/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (512)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (512)

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (6 * TCP_SND_BUF)/TCP_MSS

/* TCP receive window. */
#define TCP_WND                 (512)


/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1

/*******************************************************************************************************************/
//#define LWIP_DEBUG                      0
//#define LWIP_DBG_TYPES_ON               LWIP_DBG_OFF

/**
 * ETHARP_DEBUG: Enable debugging in etharp.c.
 */
#define ETHARP_DEBUG                    LWIP_DBG_OFF

/**
 * NETIF_DEBUG: Enable debugging in netif.c.
 */
#define NETIF_DEBUG                     LWIP_DBG_OFF

/**
 * PBUF_DEBUG: Enable debugging in pbuf.c.
 */
#define PBUF_DEBUG                      LWIP_DBG_OFF

/**
 * API_LIB_DEBUG: Enable debugging in api_lib.c.
 */
#define API_LIB_DEBUG                   LWIP_DBG_OFF

/**
 * API_MSG_DEBUG: Enable debugging in api_msg.c.
 */
#define API_MSG_DEBUG                   LWIP_DBG_OFF

/**
 * SOCKETS_DEBUG: Enable debugging in sockets.c.
 */
#define SOCKETS_DEBUG                   LWIP_DBG_OFF

/**
 * ICMP_DEBUG: Enable debugging in icmp.c.
 */
#define ICMP_DEBUG                      LWIP_DBG_OFF

/**
 * IGMP_DEBUG: Enable debugging in igmp.c.
 */
#define IGMP_DEBUG                      LWIP_DBG_OFF

/**
 * INET_DEBUG: Enable debugging in inet.c.
 */
#define INET_DEBUG                      LWIP_DBG_OFF

/**
 * IP_DEBUG: Enable debugging for IP.
 */
#define IP_DEBUG                        LWIP_DBG_OFF

/**
 * IP_REASS_DEBUG: Enable debugging in ip_frag.c for both frag & reass.
 */
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
/**
 * RAW_DEBUG: Enable debugging in raw.c.
 */
#define RAW_DEBUG                       LWIP_DBG_OFF

/**
 * MEM_DEBUG: Enable debugging in mem.c.
 */
#define MEM_DEBUG                       LWIP_DBG_OFF

/**
 * MEMP_DEBUG: Enable debugging in memp.c.
 */
#define MEMP_DEBUG                      LWIP_DBG_OFF

/**
 * SYS_DEBUG: Enable debugging in sys.c.
 */
#define SYS_DEBUG                       LWIP_DBG_OFF

/**
 * TCP_DEBUG: Enable debugging for TCP.
 */
#define TCP_DEBUG                       LWIP_DBG_OFF

/**
 * TCP_INPUT_DEBUG: Enable debugging in tcp_in.c for incoming debug.
 */
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
/**
 * TCP_FR_DEBUG: Enable debugging in tcp_in.c for fast retransmit.
 */
#define TCP_FR_DEBUG                    LWIP_DBG_OFF

/**
 * TCP_RTO_DEBUG: Enable debugging in TCP for retransmit
 * timeout.
 */
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF

/**
 * TCP_CWND_DEBUG: Enable debugging for TCP congestion window.
 */
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF

/**
 * TCP_WND_DEBUG: Enable debugging in tcp_in.c for window updating.
 */
#define TCP_WND_DEBUG                   LWIP_DBG_OFF

/**
 * TCP_OUTPUT_DEBUG: Enable debugging in tcp_out.c output functions.
 */
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
/**
 * TCP_RST_DEBUG: Enable debugging for TCP with the RST message.
 */
#define TCP_RST_DEBUG                   LWIP_DBG_OFF

/**
 * TCP_QLEN_DEBUG: Enable debugging for TCP queue lengths.
 */
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF

/**
 * UDP_DEBUG: Enable debugging in UDP.
 */
#define UDP_DEBUG                       LWIP_DBG_OFF

/**
 * TCPIP_DEBUG: Enable debugging in tcpip.c.
 */
#define TCPIP_DEBUG                     LWIP_DBG_OFF

/**
 * PPP_DEBUG: Enable debugging for PPP.
 */
#define PPP_DEBUG                       LWIP_DBG_OFF

/**
 * SLIP_DEBUG: Enable debugging in slipif.c.
 */
#define SLIP_DEBUG                      LWIP_DBG_OFF

/**
 * DHCP_DEBUG: Enable debugging in dhcp.c.
 */
#define DHCP_DEBUG                      LWIP_DBG_OFF

/**
 * AUTOIP_DEBUG: Enable debugging in autoip.c.
 */
#define AUTOIP_DEBUG                    LWIP_DBG_OFF

/**
 * SNMP_MSG_DEBUG: Enable debugging for SNMP messages.
 */
#define SNMP_MSG_DEBUG                  LWIP_DBG_OFF

/**
 * SNMP_MIB_DEBUG: Enable debugging for SNMP MIBs.
 */
#define SNMP_MIB_DEBUG                  LWIP_DBG_OFF

/**
 * DNS_DEBUG: Enable debugging for DNS.
 */
#define DNS_DEBUG                       LWIP_DBG_OFF

#define LWIP_PROVIDE_ERRNO      1

#endif



