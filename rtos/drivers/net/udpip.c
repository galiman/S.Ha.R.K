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
 CVS :        $Id: udpip.c,v 1.5 2004/05/11 14:30:51 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2004/05/11 14:30:51 $
 ------------
**/

/*
 * Copyright (C) 2000 Luca Abeni and Giuseppe Lipari
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
/* Date:        1/12/97						*/

/* File: 	UDPIPDrv.C					*/
/* Revision:    2.0						*/

/*
   UDP and IP layers. IP doesn't support fragmented packets (fragmentation
   isn't Real-Time!!!). UDP allows to fill some entries of the ARP table
   at the moment of the creation of a socket (see udp_bind).
*/

#include <kernel/kern.h>
#include <semaphore.h>

#include "eth_priv.h"
#include "netbuff.h"
#include <drivers/udpip.h>
#include "arp.h"

//#define __UDP_DBG__
//#define __IP_DBG__
#define UDP_INFO "[UDP] "
#define IP_INFO  "[IP] "

/*
   UDP-buffers number and dimension (UDP-buffers are provided by NetBuff
   module
*/
#define UDP_RX_BUFFERS  4
#define UDP_TX_BUFFERS	4
#define UDP_MAX_LEN     1000
#define UDP_MAX_HANDLES 4

/* Only 4 debug */
extern int netlev;

int ipIsInstalled = FALSE;
extern struct ip_addr myIpAddr;
struct ip_table ipTable[IP_MAX_ENTRIES];
extern ARP_TABLE arpTable[ARP_MAX_ENTRIES];
IP_ADDR IPbroadcastaddress;

int udpIsInstalled = FALSE;
struct netbuff udp_txbuff;
struct netbuff udp_rxbuff;
struct udp_table udpTable[UDP_MAX_HANDLES];
struct eth_addr broadcast;

IP_ADDR *ip_getAddr()
{
    return &myIpAddr;
}

/* TRUE if the IP addresses ip1 and ip2 are equal, FALSE otherwise */
int ip_compAddr(IP_ADDR ip1, IP_ADDR ip2)
{
	int i;
    
	for (i=0; i < 4; i++)
		if (ip1.ad[i] != ip2.ad[i]) return FALSE;

	return TRUE;
}

/* traslate an IP address from text string to 4 bytes */
int ip_str2addr(char *a, IP_ADDR *ip)
{
	int ad[6];
	int i,j;
	char c;

	i = 0;
	for(j = 0; j < 4; j++) {
		ad[j] = 0;
		while((a[i] != '.') && (a[i] != 0)) {
			c = a[i++];
			if (c <= '9') c = c - '0';
			else c = c - 'A' + 10;
			ad[j] = ad[j] * 10 + c;
		}
		i++;
	}

	for (i=0; i<4; i++)
		ip->ad[i] = ad[i];

	return 1;
}

/* give the body of an IP packet */
void *ip_getFDB(void *pkt)
{
	return((void *)(((BYTE *)eth_getFDB(pkt)) + sizeof(IP_HEADER)));
}

/* received IP packet CallBack */
void ip_server_recv(void *pkt)
{
	IP_HEADER *iphd;
	WORD *ptcheck;
	WORD checksum,oldChk;
	int hlen,i;
	int done;
	char dummystr[50];
	BYTE flag;

	UDP_HEADER *udphd;
	struct pseudo_hd ph;
	WORD sum, old;
	WORD *p;
	UDP_MSG usermsg;
	void *b, *s;

	/* debug...*/
	netlev = 2;

	sprintf(dummystr, "Packet arrived!\n");
	iphd = (IP_HEADER *)eth_getFDB(pkt);

	/* compute the checksum */
	ptcheck = (WORD *)iphd;
#ifdef __IP_DBG__
	//!!!ip_print_header(iphd);
#endif
	hlen = getHlen(iphd->vers_hlen) * 2;
	checksum = oldChk = *ptcheck;
	ptcheck++;
	for (i=1; i<hlen; i++) {
		checksum += *ptcheck;
		if (checksum < oldChk) checksum ++;
		oldChk = checksum;
		ptcheck++;
	}
	if (checksum != 0xffff) {
		/* wrong ChkSum */
#ifdef __IP_DBG__
		printk(KERN_DEBUG IP_INFO "Wrong checksum: %x.\n", checksum);
#endif
	} else if (!ip_compAddr(iphd->dest,myIpAddr)) {
#ifdef __IP_DBG__
		printk(KERN_DEBUG IP_INFO "Packet not addressed to this host.\n");
#endif
	} else if (getFlags(ntohs(iphd->flags_frOffset)) & 0x01) {
#ifdef __IP_DBG__
		/* fragment? */
		printk(KERN_DEBUG IP_INFO "Gotta a fragment!\n");
#endif
	} else if (getFrOffset(ntohs(iphd->flags_frOffset)) != 0) {
#ifdef __IP_DBG__
		printk(KERN_DEBUG IP_INFO "Gotta a fragment again!\n");
#endif
	} else {
		/* OK: the packet is good... */
		if (iphd->protocol != IP_UDP_TYPE) {
			/* It isn't an UDP packet */
			done = FALSE;
			i = 0;
			while ((i < IP_MAX_ENTRIES) && !done) {
				if (ipTable[i].protocol == iphd->protocol) done = TRUE;
				else i++;
			}
			if (!done) {
#ifdef __IP_DBG__
				/* Unknown transport protocol */
				printk(KERN_DEBUG IP_INFO "Wrong protocol.\n");
#endif
			} else {
				/* Call the correct transport protocol CallBack */
			ipTable[i].rfun((void *)((BYTE *)iphd + 4*getHlen(iphd->vers_hlen)));
			}
		} else {
			/* UDP packet */
			netlev = 4;

			udphd = (UDP_HEADER *)(void *)((BYTE *)iphd + 4*getHlen(iphd->vers_hlen));
			/* compute the UDP checksum */
			for (i = 0; i < 4; i++) {
				ph.source.ad[i]  = iphd->source.ad[i];
				ph.dest.ad[i]  = iphd->dest.ad[i];
			}
			ph.zero = 0;
			ph.protocoll = IP_UDP_TYPE;
			ph.len = udphd->mlen;
			sum = 0;
			old = 0;
			p = (WORD *)&ph;
			for (i = 0; i < (sizeof(ph) >> 1); i++) {
				sum += p[i];
				if (sum < old) sum ++;
				old = sum;
			}
#ifdef __UDP_DBG__
			printk(KERN_DEBUG UDP_INFO "Half sum: %x\n",sum);
#endif
			p = (WORD *)udphd;
			((BYTE *)udphd)[ntohs(udphd->mlen)] = 0;
			for (i = 0; i < ((ntohs(udphd->mlen) + 1) >> 1); i++) {
				sum += p[i];
				if (sum < old) sum ++;
				old = sum;
			}
#ifdef __UDP_DBG__
			printk(KERN_DEBUG UDP_INFO "Packet received.\n");
			printk(KERN_DEBUG UDP_INFO "s_port: %x\n",udphd->s_port);
			printk(KERN_DEBUG UDP_INFO "d_port: %x\n",udphd->d_port);
			printk(KERN_DEBUG UDP_INFO "m_len: %x %d\n",udphd->mlen, ntohs(udphd->mlen));
			printk(KERN_DEBUG UDP_INFO "checksum: %x\n",udphd->checksum);
			printk(KERN_DEBUG UDP_INFO "mysum: %x \n", sum);
#endif
			if(sum != 0xFFFF) {
				/* Wrong UDP ChkSum */
				cprintf("Error %x!\n", sum);
				kern_raise(XUDP_BADCHK_EXC,exec_shadow);
			} else {
				done = FALSE;
				i = 0;
				/* searching for the destination socket...*/
				while((i < UDP_MAX_HANDLES) && !done) {
					if ((udpTable[i].valid == TRUE) && (udpTable[i].port == ntohs(udphd->d_port)))
						done = TRUE;
					else i++;
				}
				if (done) {
					/*...found! */
					s = (void *)(((BYTE *)udphd) + sizeof(UDP_HEADER));
					if (udpTable[i].notify == TRUE) {
						/* notify function associated to the socket: call it */
						udpTable[i].notify_fun(ntohs(udphd->mlen) - sizeof(UDP_HEADER),
							s, udpTable[i].notify_par);
					} else {
						/* otherwise, send the packet to the correct port */
						if((b = netbuff_get(&udp_rxbuff, NON_BLOCK)) != 0) {
							memcpy(b,s,ntohs(udphd->mlen) - sizeof(UDP_HEADER) + 1);
							setIpAddr(usermsg.addr.s_addr, iphd->source);
							usermsg.addr.s_port = ntohs(udphd->s_port);
							usermsg.mlen = ntohs(udphd->mlen) - sizeof(UDP_HEADER);
							usermsg.buff = b;
							flag = port_send(udpTable[i].hport,&usermsg,NON_BLOCK);
							if (!flag) {
								netbuff_release(&udp_rxbuff, b);
#ifdef __UDP_DBG__
								printk(KERN_DEBUG "Port is filled up.\n");
#endif
							}
						}
					}
				} else {
#ifdef __UDP_DBG__
					printk(KERN_DEBUG UDP_INFO "Port not found.\n");
#endif
				}
			}
		}
	}
}

/* Send an IP packet */
void ip_send(IP_ADDR dest, void *pkt, WORD len)
{
	static WORD ip_ident = 0;
	IP_HEADER *iphd;
	WORD check = 0, oldCheck = 0;
	int i, done;
	WORD *pt;

	iphd = (IP_HEADER *)eth_getFDB(pkt);
	iphd->vers_hlen = (4 << 4) + 5;
	iphd->servType = 8;
	iphd->lenght = htons(len + sizeof(IP_HEADER));
	iphd->ident = htons(ip_ident++);
	iphd->flags_frOffset = 0;
	iphd->ttl = 10;
	iphd->protocol = IP_UDP_TYPE;
	iphd->headChecksum = 0;
	iphd->source = myIpAddr;
	iphd->dest = dest;

	/* Compute the checksum */
	pt = (WORD *)iphd;
	check = oldCheck = *pt;
	pt++;
	for (i = 1; i < 10; i++) {
		check += *pt;
		if (oldCheck > check) check++;
		oldCheck = check;
		pt++;
	}
	check = ~check;
	iphd->headChecksum = check;
#ifdef __IP_DBG__
	printk(KERN_DEBUG IP_INFO "Serv type : %d\n", iphd->servType);
#endif
	/* Is the destination IP address the broadcast address?*/
	if (ip_compAddr(dest,IPbroadcastaddress)) {
		/* Send the packet*/
		eth_setHeader(pkt,broadcast,ETH_IP_TYPE);
		eth_sendPkt(pkt,len + sizeof(IP_HEADER));
		netbuff_release(&udp_txbuff, (void *)pkt);
	} else {
		/* Is the destination ethernet address in the ARP table? */
		i = 0; done = 0;
		while (i < ARP_MAX_ENTRIES && !done)
			if (arpTable[i].valid == TRUE) {
				if (ip_compAddr(dest,arpTable[i].ip)) {
					done = TRUE;
				} else i++;
			} else i++;
		if (done == FALSE) {
			/* No: call ARP to get the ethernet address */
			arp_send(pkt, dest, len + sizeof(IP_HEADER));
		} else {
			/* Yes: directly send the packet */
			eth_setHeader(pkt,arpTable[i].eth,ETH_IP_TYPE);
			eth_sendPkt(pkt, len + sizeof(IP_HEADER));
			netbuff_release(&udp_txbuff, (void *)pkt);
			arpTable[i].used++;
			if (arpTable[i].used > ARP_MAX_USED) arpTable[i].used = ARP_MAX_USED;
		}
	}
}

/* let IP manage a new transport protocol */
int ip_setProtocol(BYTE proto, void (*recv)(void *m))
{
	BYTE done, i;

	i = 0; done = 0;
	while (i < IP_MAX_ENTRIES && !done)
		if (ipTable[i].rfun == NULL) done = TRUE;
		else i++;
	if (!done)
		return FALSE;
	else {
		ipTable[i].protocol = proto;
		ipTable[i].rfun = recv;
	}
	return TRUE;
}

/*int ip_error(int code)
{
	cprintf("IP error\n");
	cprintf("Code [%d]\nCause : %s",code,ip_error_msg[code-IP_ERROR_BASE]);
	return(0);
}*/

/* Initialize the IP layer: it also call the ARP initialization function, pass a struct ip_params* */
void ip_init(void *p)
{
	int i;

	if (!ipIsInstalled) {
		arp_init(((struct ip_params*)p)->localAddr);
		//exc_set(IP_INIT_ERROR,ip_error);
		for (i=0; i < IP_MAX_ENTRIES; i++)
			ipTable[i].rfun = NULL;

		eth_setProtocol(ETH_IP_TYPE, ip_server_recv);
		ip_str2addr(((struct ip_params*)p)->broadcastAddr,&IPbroadcastaddress);
		ipIsInstalled = TRUE;
		eth_str2Addr("FF:FF:FF:FF:FF:FF",&broadcast);
	} else 
		cprintf("IP: already installed.\n");
}

/* Receive an UDP packet from a socket */
int udp_recvfrom(int s, void *buff, UDP_ADDR *from)
{
	UDP_MSG u;

	port_receive(udpTable[s].pport,&u,BLOCK);
	memcpy(buff,u.buff,u.mlen);
	netbuff_release(&udp_rxbuff, u.buff);
	*from = u.addr;

	return (u.mlen);
}

/* Associate a notify function to a socket */
int udp_notify(int s, int (*f)(int len, BYTE *buff, void *p), void *p)
{
	if (f == NULL) {
		udpTable[s].notify = FALSE;
		return 1;
	}
	
	if (udpTable[s].valid != TRUE)
		return -1;
	udpTable[s].notify = TRUE;
	udpTable[s].notify_fun = f;
	udpTable[s].notify_par = p;

	return 1;
}

/* Create a new socket binding it to a specified IP port */
int udp_bind(UDP_ADDR *local, IP_ADDR *bindlist)
{
	int i, j;
	BYTE done;
	char str[30];

	/* Search for a free entry in the socket table */
	i = 0; done = FALSE;
	while ((i < UDP_MAX_HANDLES) && !done) {
		kern_cli();
		if ((udpTable[i].valid == FALSE)) {
			done = TRUE;
			udpTable[i].valid = 2;
		} else i++;
		kern_sti();
	}

	/* No free entries: bind fail! */
	if (!done)
		return -1;

	/* Create a receive port for the socket */
	udpTable[i].port = local->s_port;
	sprintf(str,"UDP%d",i);
	udpTable[i].hport = port_create(str,sizeof(UDP_MSG),4,STREAM,WRITE);
	udpTable[i].pport = port_connect(str,sizeof(UDP_MSG),STREAM,READ);
	udpTable[i].valid = TRUE;

	/*
	 * Request for the ethernet addresses associated to the IP addressed
	 * given in the bindlist.
	 */
	if (bindlist != NULL) {
		while (*(int*)bindlist != 0) {
			/* Ignore broadcast IP address */
			if (!ip_compAddr(*bindlist,IPbroadcastaddress)) {
				j = arp_req(*bindlist);
				arp_sendRequest(j);
			}
			bindlist ++;
		}
	}

	return i;
}

/* Send an UDP packet */
int udp_sendto(int s, void *buff, int nbytes, UDP_ADDR *to)
{
	void *pkt;
	UDP_HEADER *udphd;
	char *msg;

#ifdef __UDP_DBG__
	static int num_pack = 0;
#endif

	WORD sum, old;
	int i;
	struct pseudo_hd ph;
	WORD *p;
	IP_ADDR *source;

	pkt = netbuff_get(&udp_txbuff, BLOCK);
	udphd = (UDP_HEADER *)ip_getFDB(pkt);
	udphd->s_port = htons(udpTable[s].port);
	udphd->d_port = htons(to->s_port);
	udphd->mlen = htons((WORD)nbytes + sizeof(UDP_HEADER));
	msg = (char *)(((BYTE *)udphd) + sizeof(UDP_HEADER));
	if (nbytes > UDP_MAX_LEN) nbytes = UDP_MAX_LEN;
	memcpy(msg,buff,nbytes);

	source = ip_getAddr();
	/* Compute the CheckSum */
	udphd->checksum = 0;
	for (i = 0; i < 4; i++) {
		ph.source.ad[i]  = source->ad[i];
		ph.dest.ad[i]  = to->s_addr.ad[i];
	}
	ph.zero = 0;
	ph.protocoll = 17;
	ph.len = udphd->mlen;
	sum = 0; old = 0;
	p = (WORD *)&ph;
	for (i = 0; i < (sizeof(ph) >> 1); i++) {
		sum += p[i];
		if (sum < old) sum ++;
		old = sum;
	}
	p = (WORD *)udphd;
	((BYTE *)udphd)[ntohs(udphd->mlen)] = 0;
	for (i = 0; i < ((ntohs(udphd->mlen) + 1) >> 1); i++) {
		sum += p[i];
		if (sum < old) sum++;
		old = sum;
	}
	udphd->checksum = ~sum;
     
	ip_send(to->s_addr, pkt, ((WORD)nbytes + sizeof(UDP_HEADER)));
#ifdef __UDP_DBG__
	printk(KERN_DEBUG UDP_INFO "Packets sent: %d.\n", num_pack++);
#endif
    
	return nbytes;
}

void udp_init(void *dummy)
{
	int i;

	if (!udpIsInstalled) {
		netbuff_init(&udp_rxbuff, UDP_RX_BUFFERS, UDP_MAX_LEN);
		netbuff_init(&udp_txbuff, UDP_TX_BUFFERS, ETH_MAX_LEN);

		for (i = 0; i < UDP_MAX_HANDLES; i++) {
			udpTable[i].valid = FALSE;
			udpTable[i].notify = FALSE;
		}
		udpIsInstalled = TRUE;
	} else
		printk(KERN_WARNING UDP_INFO ": Already installed.\n");
}
