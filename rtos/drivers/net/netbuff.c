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
 CVS :        $Id: netbuff.c,v 1.3 2004/05/11 14:30:50 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2004/05/11 14:30:50 $
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
/* Date:        25/9/1997                                       */

/* File: 	NetBuff.c					*/
/* Revision:    1.00						*/

/*
   This module manages all the buffers used by the network driver:
   the buffers are grouped in netbuff structures (initialized by
   netbuff_init) and can be allocated by netbuff_get or released
   by netbuff_released. A netbuff_sequentialget function is also
   provided for beeing used by tasks that have their own buffer (
   if a buffer can be allocated by only one task, it is useless to
   allocate it in mutual exclusion). It is usefull for the receive
   buffers allocated by the receive process.
*/
#include <kernel/kern.h>
#include <semaphore.h>

#include "netbuff.h"

/* Init the buffer structures */
void netbuff_init(struct netbuff *netb, BYTE nbuffs, WORD buffdim)
{
	int i;

	kern_cli();
	netb->b = kern_alloc(nbuffs * buffdim);
	netb->pb = kern_alloc(nbuffs * sizeof(void *));
	netb->free = kern_alloc(nbuffs * sizeof(char));
	kern_sti();
	if ((netb->b == 0) || (netb->pb ==0) || (netb->free == 0)) {
		kern_raise(XNETBUFF_INIT_EXC,exec_shadow);
	}
	netb->nbuffs = nbuffs;
	for (i = 0; i < nbuffs; i++) {
		netb->pb[i] = netb->b + (i * buffdim);
		netb->free[i] = 1;
	}
	sem_init(&(netb->buffersem), 0, nbuffs);
}

/*
   Get the first free buffer in the netb pool. All the tasks can call this
   function, so a kern_cli is needed
*/
void *netbuff_get(struct netbuff *netb, BYTE to)
{
	int i, done;
	static int mycount = 0;

	if (sem_xwait(&(netb->buffersem), 1, to) != 0) {
		return NULL;
	} else {
		mycount++;
	}

	done = 0; i = 0;
	kern_cli();
	while ((i < netb->nbuffs) && !done) {
		if (netb->free[i]) {
			done = 1;
			netb->free[i] = 0;
		} else i++;
	}
	kern_sti();
	if (!done) {
		kern_raise(XNETBUFF_GET_EXC,exec_shadow);
	}
	return netb->pb[i];
}

/*
   Get the first free buffer in the netb pool. This function can be called
   only if netb is private of the process that wants to get the buffer, so
   kern_cli is not needed
*/
void *netbuff_sequentialget(struct netbuff *netb, BYTE to)
{
	int i, done;

	if (!sem_xwait(&(netb->buffersem), 1, to)) {
		return NULL;
	}

	done = 0; i = 0;
	while ((i < netb->nbuffs) && !done) {
		if (netb->free[i]) {
			done = 1;
			netb->free[i] = 0;
		} else i++;
	}
	if (!done) {
		kern_raise(XNETBUFF_GET_EXC,exec_shadow);
	}
	return netb->pb[i];
}

/* Release the buffer m of the pool netb */
void netbuff_release(struct netbuff *netb, void *m)
{
	int i, done;

	done = 0; i = 0;
	while ((i < netb->nbuffs) && !done) {
		if (netb->pb[i] == m) {
			if (netb->free[i] == 1) {
				cprintf("Trying to free a free buffer :( \n");
				kern_raise(XNETBUFF_ALREADYFREE_EXC,exec_shadow);
			}
			done = 1;
			netb->free[i] = 1;
		} else i++;
	}
	if (!done) {
		cprintf("Trying to free a STRANGE buffer :( \n");
		kern_raise(XNETBUFF_RELEASE_EXC,exec_shadow);
	}
	sem_post(&(netb->buffersem));
}
