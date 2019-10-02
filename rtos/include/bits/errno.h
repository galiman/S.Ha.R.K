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
 CVS :        $Id: errno.h,v 1.5 2005/01/08 14:53:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2005/01/08 14:53:17 $
 ------------

 - error codes used as values for errno

**/

/*
 * Copyright (C) 2000 Paolo Gai
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

/* Copyright (C) 1991, 1994, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
#define ETH_ERROR_BASE		150
#define ETH_DRIVER_NOT_FOUND	(ETH_ERROR_BASE+0)
#define ETH_RXERROR	 	(ETH_ERROR_BASE+1)
#define ETH_DRIVER_RELEASE	(ETH_ERROR_BASE+2)
#define ETH_DRIVER_ADDRESS	(ETH_ERROR_BASE+3)
#define ETH_TXERROR		(ETH_ERROR_BASE+4)
#define ETH_PROTOCOL_ERROR	(ETH_ERROR_BASE+5)
#define ETH_BUFFERS_FULL	(ETH_ERROR_BASE+6)
#define ETH_NULLPROTOCOL_EXC	(ETH_ERROR_BASE+7)
*/

#ifndef __BITS_ERRNO_H__
#define __BITS_ERRNO_H__

#include <ll/errno.h>

/*---------------------------------------------------------------------*/
/* Abort values                                                        */
/*---------------------------------------------------------------------*/

/* note: when changing these numbers, please also change their description 
   into kernel/kern.c */

#define LAST_ABORT_NUMBER        5

// the number 1 is a generic error of the lowlevel
#define ASIG_DEFAULT_ACTION      2   /* really_deliver-signal */
#define ASIGINIT                 3   /* Error in signal_init */
#define AHEXC                    4   /* Default Hartik exception handler*/
#define AARPFULL                 5   /* ARP table Full */

/*---------------------------------------------------------------------*/
/* S.Ha.R.K. exception values                                          */
/*---------------------------------------------------------------------*/

/* note: when changing these numbers, please also change their description 
   into kernel/exchand.c */

#define LAST_EXC_NUMBER         19

#define XNOMORE_EVENTS           1 /* too many events posted... */
#define XINVALID_KILL_SHADOW     2 /* task_makefree */
#define XNOMORE_CLEANUPS         3 /* task_cleanup_push */
#define XINVALID_TASK            4 /* invalid operation for a task  */
// 5 not used
// 6 not used
#define XDEADLINE_MISS           7 
#define XWCET_VIOLATION          8 
#define XACTIVATION              9 
#define XPANIC_INSIDE_IRQ       10

#define XMUTEX_OWNER_KILLED     11 /* Mutex */
#define XSRP_INVALID_LOCK       12 /* SRP */

#define XINVALID_DUMMY_OP       13 /* dummy.h hope it will never called... */
#define XINVALID_SS_REPLENISH	14 /* kernel/modules/ss.c */

#define XUNSPECIFIED_EXCEPTION  15 /* use this when you have to raise
				      an exception in your module but
				      you do not want to put too many
				      exceptions in this file */

#define XARP_TABLE_FULL	        16 /* drivers/net/arp.c */

#define NETBUFF_EXC_BASE	(XARP_TABLE_FULL+1)
#define	XNETBUFF_INIT_EXC	(NETBUFF_EXC_BASE + 0)
#define	XNETBUFF_GET_EXC	(NETBUFF_EXC_BASE + 1)
#define	XNETBUFF_ALREADYFREE_EXC (NETBUFF_EXC_BASE + 2)
#define	XNETBUFF_RELEASE_EXC	(NETBUFF_EXC_BASE + 3)

#define UDP_ERROR_BASE	(NETBUFF_EXC_BASE+4)
#define XUDP_BADCHK_EXC	(0 + UDP_ERROR_BASE)




/*---------------------------------------------------------------------*/
/* errno values                                                        */
/*---------------------------------------------------------------------*/

/* this is the maximun errno offset used by this file */
#define LAST_ERR_NUMBER         (LAST_STDERRNO+30)

#define EWRONG_INT_NO		(1 + LAST_STDERRNO)  /* in handler_set & handler_remove */
#define EUSED_INT_NO		(2 + LAST_STDERRNO)  /* in handler_set */
#define EUNUSED_INT_NO		(3 + LAST_STDERRNO)  /* handler_remove */
#define ETOOMUCH_INITFUNC	(4 + LAST_STDERRNO)  /* sys_atinit */
#define ETOOMUCH_EXITFUNC	(5 + LAST_STDERRNO)  /* sys_atexit */
#define ENO_AVAIL_TASK		(6 + LAST_STDERRNO)  /* task_create */
#define ENO_AVAIL_SCHEDLEVEL	(7 + LAST_STDERRNO)  /* task_create */
/* NOW UNUSED: ETASK_CREATE 	(8 + LAST_STDERRNO)   task_create */
#define ENO_AVAIL_RESLEVEL	(9 + LAST_STDERRNO)  /* task_create */
#define ENO_GUARANTEE          (10 + LAST_STDERRNO)  /* task_create */
#define ENO_AVAIL_STACK_MEM    (11 + LAST_STDERRNO)  /* task_create */
#define ENO_AVAIL_TSS          (12 + LAST_STDERRNO)  /* task_create */
#define EINVALID_KILL	       (13 + LAST_STDERRNO)  /* task_kill */
#define EINVALID_TASK_ID       (14 + LAST_STDERRNO)  /* task_activate */
#define EINVALID_GROUP         (15 + LAST_STDERRNO)  /* group_activate e group_kill */

#define EPORT_NO_MORE_DESCR    (16 + LAST_STDERRNO)
#define EPORT_NO_MORE_INTERF   (17 + LAST_STDERRNO)
#define EPORT_INCOMPAT_MESSAGE (18 + LAST_STDERRNO)
#define EPORT_ALREADY_OPEN     (19 + LAST_STDERRNO)
#define EPORT_NO_MORE_HASHENTRY (20+ LAST_STDERRNO)
#define EPORT_2_CONNECT        (21 + LAST_STDERRNO)
#define EPORT_UNSUPPORTED_ACC  (22 + LAST_STDERRNO)
#define EPORT_WRONG_OP         (23 + LAST_STDERRNO)
#define EPORT_WRONG_TYPE       (24 + LAST_STDERRNO)
#define EPORT_INVALID_DESCR    (25 + LAST_STDERRNO)

#define ECAB_INVALID_ID        (26 + LAST_STDERRNO)
#define ECAB_CLOSED            (27 + LAST_STDERRNO)
#define ECAB_INVALID_MSG_NUM   (28 + LAST_STDERRNO)
#define ECAB_NO_MORE_ENTRY     (29 + LAST_STDERRNO)
#define ECAB_TOO_MUCH_MSG      (30 + LAST_STDERRNO)


#endif

