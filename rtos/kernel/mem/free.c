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
 CVS :        $Id: free.c,v 1.1.1.1 2002/03/29 14:12:52 pj Exp $

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
 * Copyright (c) 1995, 1998-1999 University of Utah and the Flux Group.
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

void lmm_free(lmm_t *lmm, void *block, size_t size)
{
	struct lmm_region *reg;
	struct lmm_node *node = (struct lmm_node*)
				((DWORD)block & ~ALIGN_MASK);
	struct lmm_node *prevnode, *nextnode;

	assert(lmm != 0);
	assert(block != 0);
	assert(size > 0);

	size = (((DWORD)block & ALIGN_MASK) + size + ALIGN_MASK)
		& ~ALIGN_MASK;

	/* First find the region to add this block to.  */
	for (reg = lmm->regions; ; reg = reg->next)
	{
		assert(reg != 0);
		CHECKREGPTR(reg);

		if (((DWORD)node >= reg->min)
		    && ((DWORD)node < reg->max))
			break;
	}

	/* Record the newly freed space in the region's free space counter.  */
	reg->free += size;
	assert(reg->free <= reg->max - reg->min);

	/* Now find the location in that region's free list
	   at which to add the node.  */
	for (prevnode = 0, nextnode = reg->nodes;
	     (nextnode != 0) && (nextnode < node);
	     prevnode = nextnode, nextnode = nextnode->next);

	/* Coalesce the new free chunk into the previous chunk if possible.  */
	if ((prevnode) &&
	    ((DWORD)prevnode + prevnode->size >= (DWORD)node))
	{
		assert((DWORD)prevnode + prevnode->size
		       == (DWORD)node);

		/* Coalesce prevnode with nextnode if possible.  */
		if (((DWORD)nextnode)
		    && ((DWORD)node + size >= (DWORD)nextnode))
		{
			assert((DWORD)node + size
			       == (DWORD)nextnode);

			prevnode->size += size + nextnode->size;
			prevnode->next = nextnode->next;
		}
		else
		{
			/* Not possible -
			   just grow prevnode around newly freed memory.  */
			prevnode->size += size;
		}
	}
	else
	{
		/* Insert the new node into the free list.  */
		if (prevnode)
			prevnode->next = node;
		else
			reg->nodes = node;

		/* Try coalescing the new node with the nextnode.  */
		if ((nextnode) &&
		    (DWORD)node + size >= (DWORD)nextnode)
		{
			node->size = size + nextnode->size;
			node->next = nextnode->next;
		}
		else
		{
			node->size = size;
			node->next = nextnode;
		}
	}
}

