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
 CVS :        $Id: addfree.c,v 1.1.1.1 2002/03/29 14:12:52 pj Exp $

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

void lmm_add_free(lmm_t *lmm, void *block, size_t size)
{
	struct lmm_region *reg;
	DWORD min = (DWORD)block;
	DWORD max = min + size;

	/* Restrict the min and max further to be properly aligned.
	   Note that this is the opposite of what lmm_free() does,
	   because lmm_free() assumes the block was allocated with lmm_alloc()
	   and thus would be a subset of a larger, already-aligned free block.
	   Here we can assume no such thing.  */
	min = (min + ALIGN_MASK) & ~ALIGN_MASK;
	max &= ~ALIGN_MASK;
	assert(max >= min);

	/* If after alignment we have nothing left, we're done.  */
	if (max == min)
		return;

	/* Add the block to the free list(s) of whatever region(s) it overlaps.
	   If some or all of the block doesn't fall into any existing region,
	   then that memory is simply dropped on the floor.  */
	for (reg = lmm->regions; reg; reg = reg->next)
	{
		assert(reg->min < reg->max);
		assert((reg->min & ALIGN_MASK) == 0);
		assert((reg->max & ALIGN_MASK) == 0);

		if ((max > reg->min) && (min < reg->max))
		{
			DWORD new_min = min, new_max = max;

			/* Only add the part of the block
			   that actually falls within this region.  */
			if (new_min < reg->min)
				new_min = reg->min;
			if (new_max > reg->max)
				new_max = reg->max;
			assert(new_max > new_min);

			/* Add the block.  */
			lmm_free(lmm, (void*)new_min, new_max - new_min);
		}
	}
}

