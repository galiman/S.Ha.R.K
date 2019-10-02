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
 CVS :        $Id: arp.c,v 1.5 2005/01/08 14:57:06 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2005/01/08 14:57:06 $
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


/* Author:      Giuseppe Lipari & Luca Abeni                    */
/* Date:        4/12/97                                         */

/* File: 	ArpDrv.C					*/
/* Revision:    2.0						*/

/*
   ARP layer. It associates, the correct ethernet addresses to IP addresses
   this is done using an ARP table, created dinamicaly. This mechanism can
   introduce unpredictability (it is impossible to predict how much time is
   required to obtain an ethernet address from the network). To solve this
   problem, ARP is used only the first time that a computer is addressed.
*/

#include <kernel/kern.h>
#include <semaphore.h>

#include "eth_priv.h"
#include "netbuff.h"
#include <drivers/udpip.h>
#include "arp.h"
#include <signal.h>

//#define __ARP_DBG__
#define ARP_INFO "[ARP] "

#define FALSE	0
#define TRUE 	1
#define PENDING 2
#define ARP_TIMEOUT	5
#define ARP_MAX_RETRANS	4
#define ARP_PRIORITY	10

#define ARP_LEN		70

/*
   Structure used to enqueue the packets destinated to an host whose ethernet
   address is still unknown. This structure overwrites the firsts fields of
   the ehternet header, but it isn't a problem because it is used only
   locally
*/
typedef struct {
	int len;
	void *next;
} PKT_NXT;

typedef struct arp_pkt{
	WORD htype;
	WORD ptype;
	BYTE hlen;
	BYTE plen;
	WORD operation;
	struct eth_addr sha;
	IP_ADDR sip;
	struct eth_addr tha;
	IP_ADDR tip;
} ARP_PKT;

struct netbuff arp_txbuff;
struct eth_addr broadcast,nulladdr;

BYTE reply[ETH_MAX_LEN];
extern struct netbuff udp_txbuff;

ARP_TABLE arpTable[ARP_MAX_ENTRIES];
sem_t arpMutex;

int arpIsInstalled = FALSE;

struct eth_addr myEthAddr;
IP_ADDR myIpAddr;

/*
   ARP Exceptions Handler: in particular, it is called when the ARP table
   is full. The default action is to free the less used entry in the table
   (it's the simpler thing to do!!!). An hard Real-Time task should have
   to disable the handler in this case. (for an hard task, the full table
   condition is an error condition, 'cause it can generate unpredictability).
*/
void arp_exc(int err)
{
	int i, j, minused;

	minused = ARP_MAX_USED; j = -1;
	for (i = 0; i < ARP_MAX_ENTRIES; i++) {
		if ((arpTable[i].valid != PENDING) && (arpTable[i].used <= minused)) {
			j = i;
			minused = arpTable[i].used;
		}
	}
	if (j == -1) {
		cprintf("ARP table overflow.\n");
		exit(AARPFULL);
	}
	arpTable[j].valid = FALSE;
}

/*
   Send an ARP request: if there aren't free buffers, do nothing (there will
   be a retry)
*/
void arp_sendRequest(int i)
{
	ARP_PKT *pkt;
	BYTE *arpBuff;

	if ((arpBuff= netbuff_get(&arp_txbuff, NON_BLOCK)) != NULL) {
		eth_setHeader(arpBuff,broadcast,ETH_ARP_TYPE);
		pkt = (ARP_PKT *)eth_getFDB(arpBuff);
		pkt->htype = htons(ARP_ETH_TYPE);
		pkt->ptype = htons(ARP_IP_TYPE);
		pkt->hlen = sizeof(struct eth_addr);
		pkt->plen = sizeof(IP_ADDR);
		pkt->operation = htons(ARP_REQUEST);
		setEthAddr(pkt->sha,myEthAddr);
		setEthAddr(pkt->tha,nulladdr);
		setIpAddr(pkt->sip,myIpAddr);
		setIpAddr(pkt->tip,arpTable[i].ip);
		eth_sendPkt(arpBuff,sizeof(ARP_PKT));
		netbuff_release(&arp_txbuff, (void *)arpBuff);
	}
}

/* Retry task (periodic) */
TASK arp_retry(void)
{
	int i;
	PKT_NXT *p, *p1;

	while (1) {
		/* mutual exclusion on the ARP table */
		sem_xwait(&arpMutex, 1, BLOCK);
		for (i = 0; i < ARP_MAX_ENTRIES; i++) {
			if (arpTable[i].valid == PENDING) {
				arpTable[i].time--;
				if (arpTable[i].time <= 0) {
					arpTable[i].ntrans++;
					if (arpTable[i].ntrans > ARP_MAX_RETRANS) {
						/* N. Max Retry? If yes, discard all the packets */
						p = (PKT_NXT *)arpTable[i].pkt;
						while (p != NULL) {
							p1 = p->next;
							netbuff_release(&udp_txbuff, (void *)p);
#ifdef __ARP_DBG__
							printk(KERN_DEBUG ARP_INFO "Pacchetto : %lp scartato.\n",p);
#endif
							p = p1;
						}
						arpTable[i].valid = FALSE;
					} else {
						arp_sendRequest(i);
						arpTable[i].time = ARP_TIMEOUT;
					}
				}
			}
		}
		sem_post(&arpMutex);
		task_endcycle();
	}
}

/* Search for a free entry in the ARP table (if there isn't any, return -1 */
int arp_req(IP_ADDR dest)
{
	int j, done;

	done = 0; j = 0;
	while ((j < ARP_MAX_ENTRIES) && !done) {
		if (arpTable[j].valid == FALSE) {
			done = 1;
			arpTable[j].valid = PENDING;
		} else 
			j++;
	}

	if (!done)
		return -1;

	/* Fill the entry */
	setIpAddr(arpTable[j].ip, dest);
#ifdef __ARP_DBG__
	printk(KERN_DEBUG ARP_INFO "Indirizzo : %d.%d.%d.%d\n",dest.ad[0], dest.ad[1], dest.ad[2], dest.ad[3]);
	printk(KERN_DEBUG ARP_INFO "Indirizzo : %d.%d.%d.%d\n",arpTable[j].ip.ad[0], arpTable[j].ip.ad[1], arpTable[j].ip.ad[2], arpTable[j].ip.ad[3]);
#endif
	arpTable[j].time = ARP_TIMEOUT;
	arpTable[j].ntrans = 0;

	return j;
}

/*
   Send an IP packet. If the ethernet address isn't in the ARP table, send
   a request
*/
void arp_send(void *pkt, IP_ADDR dest, int len)
{
	int i,j;
	PKT_NXT *p,*p1 = NULL;
	int caso;

	sem_xwait(&arpMutex, 1, BLOCK);
	caso = 0;
	j = -1;
	for (i = 0; i < ARP_MAX_ENTRIES; i++) {
		if (ip_compAddr(dest,arpTable[i].ip)) {
			/* found: CASE 1 */
			if (arpTable[i].valid == TRUE) {
				caso = 1;
				j = i;
			}
			else if (arpTable[i].valid == PENDING) {
				/* Entry found, but the ethernet address is still unknown: CASE 2 */
				caso = 2;
				j = i;
			}
		}
	}
    
	if (caso == 1) {
		/* Send the IP packet */
		eth_setHeader(pkt,arpTable[j].eth,ETH_IP_TYPE);
		eth_sendPkt(pkt,len);
		netbuff_release(&udp_txbuff, (void *)pkt);
		arpTable[j].used++;
		if (arpTable[j].used > ARP_MAX_USED) arpTable[j].used = ARP_MAX_USED;
	} else if (caso == 2) {
		/* Enqueue the packet until the ethernet address arrives */
		p = arpTable[j].pkt;
		while (p != NULL) {
			p1 = p;
			p = p1->next;
		}
		p1->next = pkt;
		((PKT_NXT *)pkt)->next = NULL;
		((PKT_NXT *)pkt)->len = len;
#ifdef __ARP_DBG__
		printk(KERN_DEBUG ARP_INFO "Pacchetto : %lp accodato.\n", pkt);
#endif
	} else {
		/* Search for a free entry in the ARP table...*/
		j = -1;
		while (j == -1) {
			j = arp_req(dest);
			if (j == -1) {
				cprintf("ARP Table Full.\n");
				kern_raise(XARP_TABLE_FULL,NIL);
			}
		}
		/*...fill it...*/
		arpTable[j].pkt = pkt;
		((PKT_NXT *)arpTable[j].pkt)->next = NULL;
		((PKT_NXT *)pkt)->len = len;
#ifdef __ARP_DBG__
		printk(KERN_DEBUG ARP_INFO "Pacchetto : %lp accodato\n", pkt);
#endif
		/*...and send the request!!! */
		arp_sendRequest(j);
	}
	sem_post(&arpMutex);
}

/* ARP packet received CallBack*/
void arp_server_recv(void *pk)
{
	ARP_PKT *pkt,*rpkt;
	PKT_NXT *p1,*q1;
	int len;
	int i,j = 0;
	BYTE found;
 
	pkt = (ARP_PKT *)eth_getFDB(pk);

#if 0
	{
		int ii;

		cprintf("Arp PKT...\n");
		cprintf ("source...");
		for (ii=0; ii<4; ii++) cprintf("%d ", pkt->sip.ad[ii]);	
		cprintf ("\ndest...");
		for (ii=0; ii<4; ii++) cprintf("%d ", pkt->tip.ad[ii]);	
	}
#endif

	/* Check if the packet is directed to this host...*/
	if (ip_compAddr(pkt->tip,myIpAddr)) {
		sem_xwait(&arpMutex, 1, BLOCK);
		/* 1 : Search an entry with his IP address */
		found = FALSE;
		for (i = 0; (i < ARP_MAX_ENTRIES) && !found; i++) {
			if ((arpTable[i].valid != FALSE) && ip_compAddr(arpTable[i].ip,pkt->sip)) {
				setEthAddr(arpTable[i].eth,pkt->sha);
				found = TRUE;
			}
		}
		/* If there isn't any, fill a new entry (if the table is not full) */
		if (!found) {
			for (i = 0; (i < ARP_MAX_ENTRIES) && !found; i++) 
				if (arpTable[i].valid == FALSE) {
					j = i;
					found = TRUE;
				}
				if (found) {
					setIpAddr(arpTable[j].ip,pkt->sip);
					setEthAddr(arpTable[j].eth,pkt->sha);
					arpTable[j].valid = TRUE;
				}
		}

		/* If it is a request, send the reply */
		if (ntohs(pkt->operation) == ARP_REQUEST) {
			rpkt = (ARP_PKT *)eth_getFDB(reply);
			rpkt->htype = htons(ARP_ETH_TYPE);
			rpkt->ptype = htons(ARP_IP_TYPE);
			rpkt->hlen = sizeof(struct eth_addr);
			rpkt->plen = sizeof(IP_ADDR);
			rpkt->operation = htons(ARP_REPLY);
			setEthAddr(rpkt->sha,myEthAddr);
			setIpAddr(rpkt->sip,myIpAddr);
			setEthAddr(rpkt->tha,pkt->sha);
			setIpAddr(rpkt->tip,pkt->sip);
			eth_setHeader(reply, pkt->sha, ETH_ARP_TYPE);
			eth_sendPkt(reply,sizeof(ARP_PKT));
		}
 
		/* If it is a reply, search for his pending request */
		else {
			for (i = 0; i < ARP_MAX_ENTRIES; i++) {
				if ((arpTable[i].valid == PENDING) && ip_compAddr(arpTable[i].ip,pkt->sip)) {
					/* the eth field in the ARP table was filled previously */
					arpTable[i].valid = TRUE;
					/* Send pending packets */
					p1 = (PKT_NXT *)arpTable[i].pkt;
					while (p1 != NULL) {
						q1 = p1;
						p1 = q1->next;
						len = q1->len;
						eth_setHeader((struct ETH_HEADER *)q1,arpTable[i].eth,ETH_IP_TYPE);
						eth_sendPkt(q1,len);
						netbuff_release(&udp_txbuff, (void *)q1);
#ifdef __ARP_DBG__
						printk(KERN_DEBUG ARP_INFO "Pacchetto : %lp inviato\n", q1);
#endif
					}
				}
			} 
		} 
		sem_post(&arpMutex);
	}
}

void arp_init(char *localAddr)
{
	int i;
	PID s;
	SOFT_TASK_MODEL m;

	struct sigaction action;

	if (!arpIsInstalled) {
		for (i = 0; i < ARP_MAX_ENTRIES; i++) arpTable[i].valid = FALSE;

		/* ARP table mutex semaphore */
		sem_init(&arpMutex, 0, 1);

		netbuff_init(&arp_txbuff, 1, ARP_LEN);

		ip_str2addr(localAddr,&myIpAddr);

		eth_getAddress(&myEthAddr);

		/* Retry task */
		soft_task_default_model(m);
		soft_task_def_wcet(m,1000);
		soft_task_def_period(m,1000000);
		soft_task_def_met(m,1000);
		soft_task_def_periodic(m);
		soft_task_def_system(m);
		soft_task_def_nokill(m);
		s = task_create("ArpRetry",arp_retry,&m,NULL);
		if (s == NIL) {
			kern_printf("Cannot create ArpRetry\n");
			exit(1);
		}

		eth_setProtocol(ETH_ARP_TYPE,arp_server_recv);
		eth_str2Addr("FF:FF:FF:FF:FF:FF",&broadcast);
		eth_str2Addr("00:00:00:00:00:00",&nulladdr);

//		for (i = ARP_ERROR_BASE; i <= XARP_TABLE_FULL; i++)
//			exc_set(i, arp_exc);
		action.sa_flags = 0;
		action.sa_handler = arp_exc;
		sigfillset(&action.sa_mask); /* we block all the other signals... */

		if (sigaction(SIGARPFULL, &action, NULL) == -1) {
			perror("Error initializing ARP signal...");
			exit(AARPFULL);
		}

		task_activate(s);
		arpIsInstalled = TRUE;
	} else 
		cprintf("Arp: already installed.");
}
