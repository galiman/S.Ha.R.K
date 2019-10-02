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
 CVS :        $Id: dump.c,v 1.1.1.1 2002/03/29 14:12:52 pj Exp $

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

/*
 * Debugging routine:
 * dump an LMM memory pool and do a thorough sanity check on it.
 */

#ifndef DEBUG
#define DEBUG
#endif

#include <ll/stdio.h>
#include <kernel/func.h>
#include <kernel/lmm.h>
#define assert(test) assertk(test)

void lmm_dump(lmm_t *lmm)
{
	struct lmm_region *reg;

	kern_printf("lmm_dump(lmm=%p)\n", lmm);

	for (reg = lmm->regions; reg; reg = reg->next)
	{
		struct lmm_node *node;
		size_t free_check;

		kern_printf(" region %08lx-%08lx size=%08lx flags=%08lx pri=%d free=%08lx\n",
			reg->min, reg->max, reg->max - reg->min,
			reg->flags, reg->pri, reg->free);

		CHECKREGPTR(reg);

		free_check = 0;
		for (node = reg->nodes; node; node = node->next)
		{
			kern_printf("  node %p-%08lx size=%08lx next=%p\n", 
				node, (DWORD)node + node->size, node->size, node->next);

			assert(((DWORD)node & ALIGN_MASK) == 0);
			assert((node->size & ALIGN_MASK) == 0);
			assert(node->size >= sizeof(*node));
			assert((node->next == 0) || (node->next > node));
			assert((DWORD)node < reg->max);

			free_check += node->size;
		}

		kern_printf(" free_check=%08lx\n", free_check);
		assert(reg->free == free_check);
	}

	kern_printf("lmm_dump done\n");
}

