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
 CVS :        $Id: netbuff.h,v 1.5 2005/03/28 17:54:27 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2005/03/28 17:54:27 $
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
                           
/* File:        NetBuff.H                                       */
/* Revision:    1.00                                            */

#ifndef __NETBUFF_H__
#define __NETBUFF_H__

#include "ll/sys/cdefs.h"
#include <sem/sem/sem.h>

__BEGIN_DECLS

typedef struct netbuff{
	BYTE *b;
	BYTE **pb;
	BYTE *free;
	BYTE nbuffs;
	sem_t buffersem;
} NETBUFF;

void netbuff_init(struct netbuff *netb, BYTE nbuffs, WORD buffdim);
void *netbuff_get(struct netbuff *netb, BYTE to);
void *netbuff_sequentialget(struct netbuff *netb, BYTE to);
void netbuff_release(struct netbuff *netb, void *m);

__END_DECLS

#endif
