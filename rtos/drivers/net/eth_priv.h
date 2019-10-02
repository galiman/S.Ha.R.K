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
 CVS :        $Id: eth_priv.h,v 1.4 2004/05/11 14:30:49 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2004/05/11 14:30:49 $
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


/* Author:      Luca Abeni					*/
/* Date:        4/12/1997                                       */

/* File:        Eth.H                                           */
/* Revision:    2.0						*/

#ifndef __ETH_H__
#define __ETH_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

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

#define ETH_ERROR_BASE		150
#define ETH_DRIVER_NOT_FOUND	(ETH_ERROR_BASE+0)
#define ETH_RXERROR	 	(ETH_ERROR_BASE+1)
#define ETH_DRIVER_RELEASE	(ETH_ERROR_BASE+2)
#define ETH_DRIVER_ADDRESS	(ETH_ERROR_BASE+3)
#define ETH_TXERROR		(ETH_ERROR_BASE+4)
#define ETH_PROTOCOL_ERROR	(ETH_ERROR_BASE+5)
#define ETH_BUFFERS_FULL	(ETH_ERROR_BASE+6)
#define ETH_NULLPROTOCOL_EXC	(ETH_ERROR_BASE+7)

/*unsigned short htons(unsigned short host);
unsigned short ntohs(unsigned short net);*/
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


__END_DECLS
#endif
