/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * httpd.c
 *
 * Author : Adam Dunkels <adam@sics.se>
 *
 * CHANGELOG: this file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es>
 *            Departamento de Informática de Sistemas y Computadores
 *            Universidad Politécnica de Valencia
 *            Valencia (Spain)
 *            Date: March 2003
 *
 */

#include "httpd.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

struct http_state
{
  char *file;
  u32_t left;
};


/*-----------------------------------------------------------------------------------*/
static void
conn_err(void *arg, err_t err)
{
  struct http_state *hs;

  hs = arg;
  mem_free(hs);
}
/*-----------------------------------------------------------------------------------*/
static void
close_conn(struct tcp_pcb *pcb, struct http_state *hs)
{

  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  mem_free(hs);
  tcp_close(pcb);
}
/*-----------------------------------------------------------------------------------*/
static void
send_data(struct tcp_pcb *pcb, struct http_state *hs)
{
  err_t err;
  u16_t len;

  /* We cannot send more data than space avaliable in the send
     buffer. */
  if (tcp_sndbuf(pcb) < hs->left)
  {
    len = tcp_sndbuf(pcb);
  }
  else
  {
    len = hs->left;
  }

  err = tcp_write(pcb, hs->file, len, 0);

  if (err == ERR_OK)
  {
    hs->file += len;
    hs->left -= len;
  }
}

/*-----------------------------------------------------------------------------------*/
static err_t
http_poll(void *arg, struct tcp_pcb *pcb)
{
  if (arg == NULL)
  {
    /*    printf("Null, close\n");*/
    tcp_close(pcb);
  }
  else
  {
    send_data(pcb, (struct http_state *)arg);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct http_state *hs;

  hs = arg;

  if (hs->left > 0)
  {
    send_data(pcb, hs);
  }
  else
  {
    close_conn(pcb, hs);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  int i, j;
  char *data;
  char fname[40];
  struct fs_file file = {0, 0};
  struct app_state *hs;

  hs = arg;

  if (err == ERR_OK && p != NULL)
  {

    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);

    if (hs->file == NULL)
    {
      data = p->payload;
        
      if (strncmp(data, "GET /STM32F107ADC", 17) == 0)
      {
        printf("\n\rhttp_connect");
      }
      else if (strncmp(data, "GET /method=get", 15) == 0)
      {
        printf("\n\rhttp_connect");
      }    
      else if (strncmp(data, "GET ", 4) == 0)
      {
      	printf("\n\rhttp_connect"); 
      }
      else
      {
        close_conn(pcb, hs);
      }
    }
    else
    {
      pbuf_free(p);
    }
  }

  if (err == ERR_OK && p == NULL)
  {

    close_conn(pcb, hs);
  }
  
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct app_state *hs;
  u32_t IPaddress;
  u8_t iptxt[20];
  volatile u8_t iptab[4];
  
  IPaddress = pcb->remote_ip.addr;
  printf("\n\rhttp_accept:%d.%d.%d.%d\n\r", (u8_t)(IPaddress),
                (u8_t)(IPaddress >> 8),(u8_t)(IPaddress >> 16),(u8_t)(IPaddress >> 24));


  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, http_recv);

  tcp_err(pcb, conn_err);

  tcp_poll(pcb, http_poll, 10);
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_init(void)
{
  struct tcp_pcb *pcb;

  /* Create a new TCP control block  */
  pcb = tcp_new();
  
  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, 80);

  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);

  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, http_accept);
}
/*-----------------------------------------------------------------------------------*/
int
fs_open(char *name, struct fs_file *file)
{
  struct fsdata_file_noconst *f;

  return 0;
}
/*-----------------------------------------------------------------------------------*/

