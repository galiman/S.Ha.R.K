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
 CVS :        $Id: arp.h,v 1.4 2004/05/11 14:30:48 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2004/05/11 14:30:48 $
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

/* File:        ARP.H                                           */
/* Revision:    2.0						*/

#ifndef __ARP_H__
#define __ARP_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define ARP_IP_TYPE     0x0800
#define ARP_ETH_TYPE 	1

#define ARP_REQUEST	1
#define ARP_REPLY	2

#define ARP_MAX_ENTRIES 50
#define ARP_MAX_USED	99

//#define ARP_ERROR_BASE	(ETH_ERROR_BASE + 10)
//#define ARP_TABLE_FULL	(ARP_ERROR_BASE + 0)


typedef struct arp_table{
	IP_ADDR ip;
	struct eth_addr eth;
	int used;
	int time;
	int ntrans;
	void *pkt;
	BYTE valid;
} ARP_TABLE;

void arp_send(void *pkt, IP_ADDR dest, int len);
void arp_init(char *localAddr);
int arp_req(IP_ADDR dest);
void arp_sendRequest(int i);

__BEGIN_DECLS

#endif
