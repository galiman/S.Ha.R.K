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
 CVS :        $Id: eth.h,v 1.1 2004/05/11 15:14:48 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2004/05/11 15:14:48 $
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

/* Project:     HARTIK 3.0 Network Library                      */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        4/12/1997                                       */

/* File:        Eth.H                                           */
/* Revision:    2.0						*/

#ifndef __ETH_H__
#define __ETH_H__

#include <ll/ll.h>
#include <endian.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOOPBACK 2
		
#define ETH_MAX_LEN	1528
#define ETH_ARP_TYPE	0x0806
#define ETH_IP_TYPE	0x0800
#define ETH_HRT_TYPE	0x8FFF
#define ETH_MAX_PROTOCOLS	5

typedef struct eth_addr {
    BYTE ad[6];
} ETH_ADDR;

typedef struct eth_header {
    struct eth_addr dest;
    struct eth_addr source;
    WORD type;
} ETH_HEADER;

typedef struct eth_device {
    DWORD BaseAddress;
    BYTE IntLine;
    struct eth_addr addr;
} ETH_DEVICE;

#define setEthAddr(q,w) memcpy(q.ad,w.ad,sizeof(struct eth_addr))

/*
--- These functions are declared into endian.h that includes bits/endian.h
    that includes ../oslib/libm/machine/endian.h :-(
    Maybe the code needs a BIG cleanup... PJ
unsigned short htons(unsigned short host);
unsigned short ntohs(unsigned short net);
*/

void eth_str2Addr(char *add, struct eth_addr *ds);
void eth_printHeader(struct eth_header *p);
void eth_printPkt(char *pkt,int len);
int eth_setProtocol(WORD type, void (*recv)(void *pkt));
void *eth_setHeader(void *b, struct eth_addr dest, WORD type);
int eth_sendPkt(void *p, int len);
void *eth_getFDB(void *pkt);
int eth_init(int mode, TASK_MODEL *m);
void eth_close(void *a);
void eth_getAddress(struct eth_addr *eth);

#ifdef __cplusplus
};
#endif

#endif
