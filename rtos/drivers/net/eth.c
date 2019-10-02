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
 CVS :        $Id: eth.c,v 1.12 2005/02/25 11:04:03 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.12 $
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


/* Author:      Luca Abeni                  */
/* Date:        2/12/1997                                       */

/* File:    eth.C                       */
/* Revision:    2.0                     */

/*
   Ethernet layer: it is an inetrface between the low level driver (
   3Com 3c59xx, ...) and the high level protocols (IP, ARP,...).
*/

/* only 4 debug... */
int netlev;

#include <kernel/kern.h>
#include <hartport/hartport/hartport.h>

#include <linux/pci.h>

#include "eth_priv.h"
#include "netbuff.h"

#include <linux/netdevice.h>
/*#include "lowlev.h"
//#include "3com.h" */

//#define DEBUG_ETH

#define ETH_PAGE        5

struct eth_service{
	WORD type;
	void (*rfun)(void *pkt);
} ETH_SERVICE;

int definedprotocols;
struct eth_service eth_table[ETH_MAX_PROTOCOLS];

#define ETH_RX_BUFFERS      4   
#define ETH_TX_BUFFERS      4

#ifndef ETH0_ADDR
# define ETH0_ADDR 0
#endif
#ifndef ETH0_IRQ
# define ETH0_IRQ 0
#endif

#define NONE 0

/*extern void net_handler(void);
//extern PID net_extern_driver(void);*/

PID nettask_pid = NIL;
static PORT NetRxPort;

/* void (*vortex_send)(DWORD BaseAddress, DWORD *txbuff, int len); */

int ethIsInstalled = FALSE;

/* device descriptor */
struct eth_device eth_dev;
struct pci_des pci_devs[5];
/* This is the Linux one!!! */
static struct device device0 = {
	"eth0", 0, 0, 0, 0, ETH0_ADDR, ETH0_IRQ, 0, 0, 0, NULL, NULL};
struct device *dev_base = &device0;

/* received frames buffers */
extern struct netbuff rxbuff; /* from skbuff.c */

/* buffers for the frames to send */
/* struct netbuff txbuff; */

/* Called if an unknown frames arrives */
void eth_nullfun(void *pkt)
{
	kern_raise(ETH_NULLPROTOCOL_EXC,NIL);
}

void dev_tint(struct device *dev)
{
	printk(KERN_WARNING "Warning! dev_tint called. (Why?)\n");
	exit(201);
}

/*
   -----------------------------------------------------------------------
   The extern process calls this function when a frame arrives
   -----------------------------------------------------------------------
*/

void netif_rx(struct sk_buff *skb)
{
	// cprintf("DENTRO netif_rx, skbuf=%p\n",skb->data);
	if (nettask_pid == NIL) {
		printk(KERN_CRIT "Net receives packets, but the driver doesn't exist.\n");
		exit(300);
	}

	port_send(NetRxPort,skb,NON_BLOCK);
	// task_activate(nettask_pid);
}

TASK net_extern_driver(void)
{
	static PORT NetPort;
	struct sk_buff skb;
	void *pkt;
	int len;
	BYTE count;
	int i;

	NetPort = port_connect("NetPort", sizeof(struct sk_buff), STREAM, READ);
	while (1) {
		/* debug... */
		netlev = 1;

		port_receive(NetPort,&skb,BLOCK);   
		pkt = skb.data;
		len = skb.len;

		((struct eth_header *)pkt)->type = ntohs(((struct eth_header *)pkt)->type);
		count = 0;
		/* Search for the frame protocol...*/
		for (i = 0; i < definedprotocols; i++) {
			/* debug... */
			netlev = 10 + i;

			if (eth_table[i].type == (((struct eth_header *)pkt)->type)) {
				count++;
				/*...and call the protocol CallBack!!! */
				eth_table[i].rfun(pkt);
			}
		}

		/* debug... */
		netlev = 20;

		// cprintf("ETH: releasing %p\n", pkt);

		// NOTE changed by PJ because skb.data not always point to the
		// buffer start!!!... it is skb.head that always points there!
		netbuff_release(&rxbuff, skb.head); 

		/* debug... */
		netlev = 30;
	}
}

/*
   -------------------- 
   Interface functions
   --------------------
*/
/* formatted print of an ethernet header */
void eth_printHeader(struct eth_header *p)
{
        cprintf("Dest   : %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x \n",p->dest.ad[0],
							p->dest.ad[1],
							p->dest.ad[2],
							p->dest.ad[3],
							p->dest.ad[4],
							p->dest.ad[5]);
	cprintf("Source : %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x \n",p->source.ad[0],
							p->source.ad[1],
							p->source.ad[2],
							p->source.ad[3],
							p->source.ad[4],
							p->source.ad[5]);
	cprintf("Type : %x\n",p->type);
}

void eth_showinfo(struct eth_device *d)
{
	cprintf("IntLine		: %d\n",d->IntLine);
	cprintf("BaseAddress	: %lx\n",d->BaseAddress);
	cprintf("Address		: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
					d->addr.ad[0],d->addr.ad[1],d->addr.ad[2],
					d->addr.ad[3],d->addr.ad[4],d->addr.ad[5]);
}

/* formatted print of an ethernet frame*/
void eth_printPkt(char *pkt,int len)
{
	int i,j,offset;
	
	eth_printHeader((struct eth_header *)pkt);
	offset = sizeof(struct eth_header);
	len -= offset;
	for (i = 0; i < len; i += 10) {
	for (j = 0; j < 10; j++)
		cprintf("%2.2x  ", pkt[offset+i+j]);
	for (j = 0; j < 10; j++)
		cputc(pkt[offset+i+j]);
		cprintf("\n");
	}
	cprintf("\n");
}

void eth_copy_and_sum(struct sk_buff *dest, unsigned char *src, int length, int base)
{
	memcpy(dest->data, src, length);
}

#if 0									
/*-------------------- User Interface -----------------------------*/
unsigned short htons(unsigned short host)
{
	return ((host & 0xff00) >> 8) + ((host & 0x00ff) << 8);
}

unsigned short ntohs(unsigned short host)
{
	return ((host & 0xff00) >> 8) + ((host & 0x00ff) << 8);
}
#endif

/*
   Translate an ethernet address from a text string to an eth_addr
   structure
*/
void eth_str2Addr(char *add, struct eth_addr *ds)
{
	int ad[6];
	int i,j;
	char c;

	i = 0;
	for(j = 0; j < 6; j++) {
		ad[j] = 0;
			while((add[i] != ':') && (add[i] != 0)) {
				c = add[i++];
				if (c <= '9') c = c - '0';
				else c = c - 'A' + 10;
				ad[j] = ad[j] * 16 + c;
			}
		i++;
	}
	for (i=0; i<6; i++) ds->ad[i] = ad[i];
}

/* Set a higher level protocol's CallBack */
int eth_setProtocol(WORD type, void (*recv)(void *pkt))
{
	int i;

	if (definedprotocols == ETH_MAX_PROTOCOLS) return FALSE;
	for(i = 0; i < definedprotocols; i++) {
		if (eth_table[i].type == type) return FALSE;
	}
	eth_table[definedprotocols].type = type;
	eth_table[definedprotocols++].rfun = recv;

	return TRUE;
}

/* Fill an ethernet frame's header and return a pointer to the frame's body */
void *eth_setHeader(void *b,struct eth_addr dest, WORD type)
{
	setEthAddr(((struct eth_header *)b)->dest,dest);
	setEthAddr(((struct eth_header *)b)->source,eth_dev.addr);
	/* the type field is in big-endian format */
	((struct eth_header *) b)->type = htons(type);

	return((BYTE *)b + sizeof(struct eth_header));
}

/* getFirstDataByte : Return a pointer to the body of an ethernet frame */
void *eth_getFDB(void *p)
{
	return ((void *)((BYTE *)p + sizeof(struct eth_header)));
}

/* eth_getAddress : return the local ethernet address */
void eth_getAddress(struct eth_addr *eth)
{
	memcpy(eth->ad,&(device0.dev_addr),sizeof(struct eth_addr));
}

/* Send an ethernet frame */
int eth_sendPkt(void *p, int len)
{
	int i;
	int l;
	struct sk_buff buff;

	l = len + sizeof(struct eth_header);
	if (l < 60) {
		for (i = l; i <= 60; i++) *((BYTE *)p + i) = 0;
		l = 60;
	}
	buff.len = l;
	buff.data = p;
	device0.hard_start_xmit(&buff, &device0);
	/* lowlev_send(eth_dev.BaseAddress, p, l); */

	return TRUE;
}

int eth_exc(int err)
{
	int p;

	 if (err != ETH_BUFFERS_FULL) printk(KERN_ERR "Ethernet : ");
	switch (err) {
	case ETH_DRIVER_NOT_FOUND :
		printk(KERN_ERR "NET PANIC --> Etherlink not found.\n");
		return 0;
	case ETH_RXERROR :
		printk(KERN_ERR "Receive error.\n");
		return 0;
	case ETH_TXERROR :
		printk(KERN_ERR "Transimit error: N. Max Retry.\n");
		return 0;
	case ETH_PROTOCOL_ERROR :
		printk(KERN_ERR "Too much protocols.\n");
		return 0;
	case ETH_BUFFERS_FULL:
		printk(KERN_ERR "Buffers full: frame lost!\n");
		return 1;
	case ETH_NULLPROTOCOL_EXC:
		printk(KERN_ERR "Null protocol called!\n");
		for (p = 0; p < ETH_MAX_PROTOCOLS; p++) {
		printk(KERN_ERR "%d:   %d\n", p, eth_table[p].type);
		}
		return 0;
	default :
		return 1;
	}
}

void skb_init(void);
void linuxpci_init(void);
int rtl8139_probe(struct device *dev);
int tc59x_probe(struct device *dev);
int eepro100_probe(struct device *dev);
int el3_probe(struct device *dev);
int ne_probe(struct device *dev);

int eth_init(int mode, TASK_MODEL *m)
{
	SOFT_TASK_MODEL m_soft;
	int i;
#if 0
        ndev;
	WORD Class;
	struct pci_regs *r;
	PID p;

#endif

	BYTE cardtype;
	int linux_found = 0;

	if (!ethIsInstalled) {
		/* Scan the devices connected to the PCI bus */
		cardtype = NONE;

		skb_init();
		NetRxPort = port_create("NetPort",sizeof(struct sk_buff),50,STREAM,WRITE);
		if (!m) {
			soft_task_default_model(m_soft);
			soft_task_def_wcet(m_soft, 1000);
			soft_task_def_period(m_soft,20000);
			soft_task_def_met(m_soft, 1000);
			soft_task_def_aperiodic(m_soft);
			soft_task_def_system(m_soft);
			soft_task_def_nokill(m_soft);
			m = (TASK_MODEL *)&m_soft;
		}

		nettask_pid = task_create("rxProc", net_extern_driver, m, NULL);
		if (nettask_pid == NIL) {
			printk(KERN_ERR "Can't create extern driver.\n");
			return 0;
		}
		task_activate(nettask_pid);

		if (1) { //!!!if (pci_init() == 1) {
			linuxpci_init();
#ifdef DEBUG_ETH
			printk(KERN_DEBUG "LF %d\n", linux_found);
#endif
			linux_found += (rtl8139_probe(&device0) == 0);
#ifdef DEBUG_ETH
                        printk(KERN_DEBUG "LF %d\n", linux_found);
#endif
			linux_found += (tc59x_probe(&device0) == 0);
#ifdef DEBUG_ETH
			printk(KERN_DEBUG "LF %d\n", linux_found);
#endif
 			linux_found += (eepro100_probe(&device0) == 0);
#ifdef DEBUG_ETH
			printk(KERN_DEBUG "LF %d\n", linux_found);
#endif


#if 0
		ndev = pci_scan_bus(pci_devs);
#ifdef __ETH_DBG__
		pci_show(pci_devs, ndev);
#endif
		for (i = 0; i < ndev; i++) {
		r = (struct pci_regs *) pci_devs[i].mem;
		Class = r->ClassCode;
		/* Is there a network card? */
		if (Class == 0x0200) {
			if (cardtype == NONE) {
			cardtype = UNKNOWN;
			}
		/* is it a 3COM card? */
			if (r->VendorId == 0x10b7) {
			/* YES!!!!!! */
			lowlev_info = vortex_info;
				lowlev_readregs = vortex_readregs;
				lowlev_open = vortex_open;
				lowlev_close = vortex_close;
				if (mode == TXTASK) {
				lowlev_send = vortex_send_msg;
			} else {
				lowlev_send = vortex_send_mem;
			}
		        printk(KERN_INFO "PCI Ethlink card found:\n");
			lowlev_info(r);
			cardtype = VORTEX;
			}
		}
		}
	}
	if ((cardtype == NONE) || (cardtype == UNKNOWN)) {
		exc_raise(ETH_DRIVER_NOT_FOUND);
	}
	/*PUT HERE THE PFUN INIT!!!*/
	if (cardtype == VORTEX) {
	}
	/*
	   Use it if you want to see the value of the internal
	   registers of the card
	*/
	/*vortex_readregs(eth_dev.BaseAddress);*/

	p = lowlev_open(eth_dev.BaseAddress, mode);
	
	/* Set the Fast Handler and the external process */
	handler_set(eth_dev.IntLine, net_handler, p);
#else
		}
		if (linux_found == 0) {
		  linux_found += (el3_probe(&device0) == 0);
#ifdef DEBUG_ETH
		  printk(KERN_DEBUG "LF %d\n", linux_found);
#endif
		  linux_found += (ne_probe(&device0) == 0);
#ifdef DEBUG_ETH
		  printk(KERN_DEBUG "LF %d\n", linux_found);
#endif
		  linux_found += (NS8390_init(&device0, 1) == 0);
#ifdef DEBUG_ETH
		  printk(KERN_DEBUG "LF %d\n", linux_found);
#endif
		}

/*
		if (mode & LOOPBACK) {
			cprintf("Installing loopback device (forced)\n");
			loopback_init(&device0);
		}
*/
		if (linux_found) {
			device0.open(&device0);
			printk(KERN_INFO "Network card found.\n");
		} else {
			printk(KERN_INFO "No network card found.\n");
/*			cprintf("No network card found. Installing loopback device.\n");
			loopback_init(&device0);
			device0.open(&device0);*/
			return 0;
		}
#endif

		//netbuff_init(&rxbuff, ETH_RX_BUFFERS, ETH_MAX_LEN);
		//netbuff_init(&txbuff, ETH_TX_BUFFERS, ETH_MAX_LEN);

		definedprotocols = 0;
		for (i = 0; i < ETH_MAX_PROTOCOLS; i++) {
			eth_table[i].type = 0;
			eth_table[i].rfun = eth_nullfun;
		}
		ethIsInstalled = TRUE;
		/* Don't crash the system at the exit, please :) */
		sys_atrunlevel(eth_close,NULL,RUNLEVEL_BEFORE_EXIT);

	} else {
		printk(KERN_INFO "Ethernet already installed.\n");
		return 0;
	}
	return 1;
}

void eth_close(void *a)
{
#ifdef DEBUG_ETH
	printk(KERN_DEBUG "Network Closing.\n");
#endif
	if (ethIsInstalled == TRUE) {
		device0.stop(&device0);
		/* This seems to break everithing... */
		// lowlev_close(eth_dev.BaseAddress);
		ethIsInstalled = FALSE;
	}
#ifdef DEBUG_ETH
	printk(KERN_DEBUG "Network Closed.\n");
#endif
}
