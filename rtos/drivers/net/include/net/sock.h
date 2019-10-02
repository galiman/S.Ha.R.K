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
 CVS :        $Id: sock.h,v 1.1 2004/05/11 14:32:02 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2004/05/11 14:32:02 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai, Luca Abeni
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


#ifndef __SOCK__
#define __SOCK__

#include <linux/timer.h>

#include <linux/netdevice.h>
#include <linux/skbuff.h>	/* struct sk_buff */

#include "ll/sys/cdefs.h"

__BEGIN_DECLS


/*
 * This structure really needs to be cleaned up.
 * Most of it is for TCP, and not used by any of
 * the other protocols.
 */
struct sock 
{
	/* This must be first. */
	struct sock		*sklist_next;
	struct sock		*sklist_prev;

	struct options		*opt;
	atomic_t		wmem_alloc;
	atomic_t		rmem_alloc;
	unsigned long		allocation;		/* Allocation mode */
	__u32			write_seq;
	__u32			sent_seq;
	__u32			acked_seq;
	__u32			copied_seq;
	__u32			rcv_ack_seq;
	unsigned short		rcv_ack_cnt;		/* count of same ack */
	__u32			window_seq;
	__u32			fin_seq;
	__u32			urg_seq;
	__u32			urg_data;
	__u32			syn_seq;
	int			users;			/* user count */
  /*
   *	Not all are volatile, but some are, so we
   * 	might as well say they all are.
   */
	volatile char		dead,
				urginline,
				intr,
				blog,
				done,
				reuse,
				keepopen,
				linger,
				delay_acks,
				destroy,
				ack_timed,
				no_check,
				zapped,	/* In ax25 & ipx means not linked */
				broadcast,
				nonagle,
				bsdism;
	unsigned long	        lingertime;
	int			proc;

	struct sock		*next;
	struct sock		**pprev;
	struct sock		*bind_next;
	struct sock		**bind_pprev;
	struct sock		*pair;
	int			hashent;
	struct sock		*prev;
	struct sk_buff		* volatile send_head;
	struct sk_buff		* volatile send_next;
	struct sk_buff		* volatile send_tail;
	struct sk_buff_head	back_log;
	struct sk_buff		*partial;
	struct timer_list	partial_timer;
	long			retransmits;
	struct sk_buff_head	write_queue,
				receive_queue;
	struct proto		*prot;
	struct wait_queue	**sleep;
	__u32			daddr;
	__u32			saddr;		/* Sending source */
	__u32			rcv_saddr;	/* Bound address */
	unsigned short		max_unacked;
	unsigned short		window;
	__u32                   lastwin_seq;    /* sequence number when we last updated the window we offer */
	__u32			high_seq;	/* sequence number when we did current fast retransmit */
	volatile unsigned long  ato;            /* ack timeout */
	volatile unsigned long  lrcvtime;       /* jiffies at last data rcv */
	volatile unsigned long  idletime;       /* jiffies at last rcv */
	unsigned int		bytes_rcv;
/*
 *	mss is min(mtu, max_window) 
 */
	unsigned short		mtu;       /* mss negotiated in the syn's */
	volatile unsigned short	mss;       /* current eff. mss - can change */
	volatile unsigned short	user_mss;  /* mss requested by user in ioctl */
	volatile unsigned short	max_window;
	unsigned long 		window_clamp;
	unsigned int		ssthresh;
	unsigned short		num;
	volatile unsigned short	cong_window;
	volatile unsigned short	cong_count;
	volatile unsigned short	packets_out;
	volatile unsigned short	shutdown;
	volatile unsigned long	rtt;
	volatile unsigned long	mdev;
	volatile unsigned long	rto;

/*
 *	currently backoff isn't used, but I'm maintaining it in case
 *	we want to go back to a backoff formula that needs it
 */
 
	volatile unsigned short	backoff;
	int			err, err_soft;	/* Soft holds errors that don't
						   cause failure but are the cause
						   of a persistent failure not just
						   'timed out' */
	unsigned char		protocol;
	volatile unsigned char	state;
	unsigned char		ack_backlog;
	unsigned char		max_ack_backlog;
	unsigned char		priority;
	unsigned char		debug;
	int			rcvbuf;
	int			sndbuf;
	unsigned short		type;
	unsigned char		localroute;	/* Route locally only */
  
#if 0
/*
 *	This is where all the private (optional) areas that don't
 *	overlap will eventually live. 
 */
	union
	{
	  	struct unix_opt	af_unix;
#ifdef CONFIG_INET
		struct inet_packet_opt  af_packet;
#ifdef CONFIG_NUTCP		
		struct tcp_opt		af_tcp;
#endif		
#endif
	} protinfo;  		
#endif
	
/* 
 *	IP 'private area' or will be eventually 
 */
	int			ip_ttl;			/* TTL setting */
	int			ip_tos;			/* TOS */
//	struct tcphdr		dummy_th;
	struct timer_list	keepalive_timer;	/* TCP keepalive hack */
	struct timer_list	retransmit_timer;	/* TCP retransmit timer */
	struct timer_list	delack_timer;		/* TCP delayed ack timer */
	int			ip_xmit_timeout;	/* Why the timeout is running */
	struct rtable		*ip_route_cache;	/* Cached output route */
	unsigned char		ip_hdrincl;		/* Include headers ? */
#ifdef CONFIG_IP_MULTICAST  
	int			ip_mc_ttl;		/* Multicasting TTL */
	int			ip_mc_loop;		/* Loopback */
	char			ip_mc_name[MAX_ADDR_LEN];/* Multicast device name */
	struct ip_mc_socklist	*ip_mc_list;		/* Group array */
#endif  

/*
 *	This part is used for the timeout functions (timer.c). 
 */
 
	int			timeout;	/* What are we waiting for? */
	struct timer_list	timer;		/* This is the TIME_WAIT/receive timer
					 * when we are doing IP
					 */
//	struct timeval		stamp;

 /*
  *	Identd 
  */
  
	struct socket		*socket;
  
  /*
   *	Callbacks 
   */
   
	void			(*state_change)(struct sock *sk);
	void			(*data_ready)(struct sock *sk,int bytes);
	void			(*write_space)(struct sock *sk);
	void			(*error_report)(struct sock *sk);
  
};

__END_DECLS

#endif
