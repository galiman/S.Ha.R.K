/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: udpip.h,v 1.2 2005/02/25 11:04:03 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2005/02/25 11:04:03 $
 ------------

**/

/*
 * Copyright (C) 2000 Luca Abeni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* Project:     HARTIK 3.0                                      */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        25/9/96						*/

/* File:        UDPIP.h                                         */
/* Revision:    4.0						*/



/* First, include the net general definitions */

#include <hartport/hartport/hartport.h>
#include <drivers/net.h>

#ifndef __UDPIP_H__
#define __UDPIP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*#define net_setudpip(m,localaddr) {net_setprotocol(&m, ip_init, localaddr); net_setprotocol(&m, udp_init, NULL);}*/

#define net_setudpip(m,localaddr,broadcastaddr) {struct ip_params p={localaddr,broadcastaddr};net_setprotocol(&m, ip_init, &p); net_setprotocol(&m, udp_init, NULL);}

struct ip_params{
    char*localAddr;
    char*broadcastAddr;
};

typedef struct ip_addr{
    BYTE ad[4];
} IP_ADDR;

#define IP_ERROR_BASE	(ETH_ERROR_BASE + 20)
#define IP_INIT_ERROR 	(0 + IP_ERROR_BASE)

#define IP_UDP_TYPE     17
#define IP_TCP_TYPE     6

#define setIpAddr(q,w) memcpy(q.ad,w.ad,sizeof(IP_ADDR))
#define getVers(b)      ((b) >> 4)
#define getHlen(b)      (b & 0x0f)
#define getFlags(w)     ((w) >> 13)
#define getFrOffset(w)  (w & 0x1fff)

typedef struct ip_header {
    BYTE vers_hlen;
    BYTE servType;
    WORD lenght;
    WORD ident;
    WORD flags_frOffset;
    BYTE ttl;
    BYTE protocol;
    WORD headChecksum;
    IP_ADDR source;
    IP_ADDR dest;
} IP_HEADER;

#define IP_MAX_ENTRIES 2
typedef struct ip_table{
    BYTE protocol;
    void (*rfun)(void *m);
} IP_TABLE;

void ip_print_header(IP_HEADER *p);
int ip_compAddr(IP_ADDR ip1, IP_ADDR ip2);
int ip_str2addr(char *a, IP_ADDR *ip);
void *ip_getFDB(void *pkt);
int ip_setProtocol(BYTE p, void (*recv)(void *m));
void ip_send(IP_ADDR dest, void *pkt, WORD len);
void ip_init(void *p);/*pass a struct ip_params* */
IP_ADDR *ip_getAddr();


/***************************************************************************/
/*									   */
/*				UDP					   */
/*									   */
/***************************************************************************/

typedef struct udp_addr{
    IP_ADDR s_addr;
    WORD s_port;
} UDP_ADDR;


typedef struct udp_header{
    WORD s_port;
    WORD d_port;
    WORD mlen;
    WORD checksum;
} UDP_HEADER;

typedef struct pseudo_hd {
    IP_ADDR source;
    IP_ADDR dest;
    BYTE zero;
    BYTE protocoll;
    WORD len;
} PSEUDO_HD;

typedef struct udp_msg{
    WORD mlen;
    UDP_ADDR addr;
    void *buff;
} UDP_MSG;

typedef struct udp_table{
    WORD port;
    BYTE valid;
    PORT hport;
    PORT pport;
    BYTE notify;
    int (*notify_fun)(int l, BYTE *buff, void *p);
    void *notify_par;
} UDP_TABLE;

void udp_init(void *dummy);
int udp_recvfrom(int s, void *buff, UDP_ADDR *from);
int udp_sendto(int s, void *buff, int nbytes, UDP_ADDR *to);
int udp_bind(UDP_ADDR *a, IP_ADDR *bindlist);
int udp_notify(int s, int (*f)(int len, BYTE *buff, void *p), void *p);

#ifdef __cplusplus
};
#endif


#endif

