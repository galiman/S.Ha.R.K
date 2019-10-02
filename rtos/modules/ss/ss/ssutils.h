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
 CVS :        $Id: ssutils.h,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains utility functions used into
 aperiodic server SS (Sporadic Server)

 Title:
   SS (Sporadic Server) utilities 

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


#ifndef __SSUTILS_H__
#define __SSUTILS_H__

#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* Max size of replenish queue */
#define SS_MAX_REPLENISH MAX_EVENT

/*+ Used to store replenishment events +*/
/* Now we use static allocated array. In this way, no more then
   SS_MAX_REPLENISH replenishments can be posted.
typedef struct {
	struct timespec rtime;
	int ramount;
	replenishq *next;
} replenishq;
*/

/*+ SS local memory allocator.
    Can be used for performance optimization. +*/
void *ss_alloc(DWORD b);

/*+ Insert an element at tail of replenish queue
 	LEVEL l			module level
 	int   amount		element to insert

	RETURNS:
 	  0	seccesfull insertion
 	  NIL	no more space for insertion +*/
int ssq_inslast(LEVEL l, int amount);

/*+ Get first element from replenish queue
	LEVEL l		module level

	RETURS: extracted element +*/
int ssq_getfirst(LEVEL l);

/*+ Enquire for empty queue
	LEVEL l		module level

	RETURS:
	  0	queue is not empty
	  1	queue is empty +*/
int ssq_isempty(LEVEL l);

__END_DECLS
#endif
