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
 CVS :        $Id: net.h,v 1.1 2004/05/11 15:14:48 giacomo Exp $

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

/* File:        Net.H                                           */
/* Revision:    1.00                                            */

#ifndef __NET_H__
#define __NET_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/*									   */
/*				NetInit interface			   */
/*									   */
/***************************************************************************/
#define NET_MAX_PROTOCOLS	10
#define TXTASK 1
#define TXMEM  0
#define LOOPBACK 2

#define net_setmode(m,mode) m.lowlevparm=mode

typedef struct protocol {
    void (*initfun)(void *parm);
    void *initparms;
} PROTOCOL;

typedef struct net_model {
    int lowlevparm;
    int numprotocol;
    struct protocol protocol[NET_MAX_PROTOCOLS];
} NET_MODEL;

extern struct net_model net_base /*= {0, 0, {NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    				  NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    				  NULL, NULL, NULL, NULL, NULL, NULL}}*/;

/*void set(struct net_model *m);*/
void net_setprotocol(struct net_model *m, void (*initfun)(void *parm), void *initparms);
int net_init(struct net_model *m);

/* The UDP/IP stack definition now are in the exported header "UDPIP.H" */

#ifdef __cplusplus
};
#endif

#endif

