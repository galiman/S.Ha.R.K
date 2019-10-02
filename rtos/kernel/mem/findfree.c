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
 CVS :        $Id: findfree.c,v 1.1.1.1 2002/03/29 14:12:52 pj Exp $

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

void lmm_find_free(lmm_t *lmm, DWORD *inout_addr,
		   size_t *out_size, lmm_flags_t *out_flags)
{
	struct lmm_region *reg;
	DWORD start_addr = (*inout_addr + ALIGN_MASK) & ~ALIGN_MASK;
	DWORD lowest_addr = (DWORD)-1;
	size_t lowest_size = 0;
	unsigned lowest_flags = 0;

	for (reg = lmm->regions; reg; reg = reg->next)
	{
		struct lmm_node *node;

		if ((reg->nodes == 0)
		    || (reg->max <= start_addr)
		    || (reg->min > lowest_addr))
			continue;

		for (node = reg->nodes; node; node = node->next)
		{
			assert((DWORD)node >= reg->min);
			assert((DWORD)node < reg->max);

			if ((DWORD)node >= lowest_addr)
				break;
			if ((DWORD)node + node->size > start_addr)
			{
				if ((DWORD)node > start_addr)
				{
					lowest_addr = (DWORD)node;
					lowest_size = node->size;
				}
				else
				{
					lowest_addr = start_addr;
					lowest_size = node->size
						- (lowest_addr - (DWORD)node);
				}
				lowest_flags = reg->flags;
				break;
			}
		}
	}

	*inout_addr = lowest_addr;
	*out_size = lowest_size;
	*out_flags = lowest_flags;
}

