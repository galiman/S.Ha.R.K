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
 CVS :        $Id: net.c,v 1.5 2004/05/11 14:30:49 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
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

/* File: 	net.C						*/
/* Revision:    1.00						*/

/*
   This module provides an interface function eth_init that initialize
   all the network library layers (LowLev, Ethrnet, IP,...) according
   to the values passed in a net_model structure
*/

#include <kernel/kern.h>
#include <drivers/net.h>
#include "eth_priv.h"

//#define DEBUG_NET

/* OKKIO!!!!! net_base must change if you change NET_MAX_PROTOCOLS!!!! */
struct net_model net_base = {0, 0, {{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL},
				{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}}};

void net_setprotocol(struct net_model *m, void (*initfun)(void *parm), void *initparms)
{
	int i;

	i = m->numprotocol++;
	m->protocol[i].initfun = initfun;
	m->protocol[i].initparms = initparms;
}

int net_init(struct net_model *m)
{
	int i;
	int app;

	/* First, initialize the base layers: eth & lowlev */
	if ((app = eth_init(m->lowlevparm,NULL)) != 1) {
		return app;
	}

	/* Then, the high level layers */
	for(i = 0; i < m->numprotocol; i++) {
#ifdef DEBUG_NET
		printk(KERN_DEBUG "Protocol %d init.\n", i);
#endif
		m->protocol[i].initfun(m->protocol[i].initparms);
	}

	return 1;
}
