/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
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
 CVS :        $Id: addregio.c,v 1.1.1.1 2002/03/29 14:12:52 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1.1.1 $
 Last update: $Date: 2002/03/29 14:12:52 $
 ------------

this code is from:
List-based Memory Management, from OsKit.

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

/*
 * Copyright (c) 1995, 1998 University of Utah and the Flux Group.
 * All rights reserved.
 *
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 *
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

#include <kernel/lmm.h>
#define assert(test) assertk(test)

void lmm_add_region(lmm_t *lmm, lmm_region_t *reg,
		    void *addr, size_t size,
		    lmm_flags_t flags, lmm_pri_t pri)
{
        DWORD min = (DWORD)addr;
	DWORD max = min + size;
	struct lmm_region **rp, *r;

	/* Align the start and end addresses appropriately.  */
	min = (min + ALIGN_MASK) & ~ALIGN_MASK;
	max &= ~ALIGN_MASK;

	/* If there's not enough memory to do anything with,
	   then just drop the region on the floor.
	   Since we haven't put it on the lmm's list,
	   we'll never see it again.  */
	if (max <= min)
		return;

	/* Initialize the new region header.  */
	reg->nodes = 0;
	reg->min = min;
	reg->max = max;
	reg->flags = flags;
	reg->pri = pri;
	reg->free = 0;

	/* Add the region to the lmm's region list in descending priority order.
	   For regions with the same priority, sort from largest to smallest
	   to reduce the average amount of list traversing we need to do.  */
	for (rp = &lmm->regions;
	     (r = *rp) && ((r->pri > pri) ||
	     		   ((r->pri == pri) &&
			    (r->max - r->min > reg->max - reg->min)));
	     rp = &r->next)
	{
		assert(r != reg);
		assert((reg->max <= r->min) || (reg->min >= r->max));
	}
	reg->next = r;
	*rp = reg;
}

