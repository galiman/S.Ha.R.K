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
 CVS :        $Id: lmm.h,v 1.2 2003/03/13 13:36:28 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:36:28 $
 ------------

this code is from:
List-based Memory Management, from OsKit.

it is used in kernel/mem.c to implement the memory allocators...

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
 * Copyright (c) 1995-1996, 1998 University of Utah and the Flux Group.
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
 * Public header file for the List Memory Manager.
 */
#ifndef __KERNEL_LMM_H__
#define __KERNEL_LMM_H__

// this part was oskit/lmm.h

#include <sys/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS
typedef DWORD lmm_flags_t;
typedef int   lmm_pri_t;

/* The contents of these structures are opaque to users.  */
typedef struct lmm_region
{
	struct lmm_region *next;

	/* List of free memory blocks in this region.  */
	struct lmm_node *nodes;

	/* Virtual addresses of the start and end of the memory region.  */
	DWORD min;
	DWORD max;

	/* Attributes of this memory.  */
	lmm_flags_t flags;

	/* Allocation priority of this region with respect to other regions.  */
	lmm_pri_t pri;

	/* Current amount of free space in this region in bytes.  */
	size_t free;
} lmm_region_t;

typedef struct lmm
{
	struct lmm_region *regions;
} lmm_t;

#define LMM_INITIALIZER { 0 }

// OSKIT_BEGIN_DECLS

void lmm_init(lmm_t *lmm);
void lmm_add_region(lmm_t *lmm, lmm_region_t *lmm_region,
		    void *addr, size_t size,
		    lmm_flags_t flags, lmm_pri_t pri);
void lmm_add_free(lmm_t *lmm, void *block, size_t size);
void lmm_remove_free(lmm_t *lmm, void *block, size_t size);
void *lmm_alloc(lmm_t *lmm, size_t size, lmm_flags_t flags);
void *lmm_alloc_aligned(lmm_t *lmm, size_t size, lmm_flags_t flags,
			int align_bits, DWORD align_ofs);
void *lmm_alloc_page(lmm_t *lmm, lmm_flags_t flags);
void *lmm_alloc_gen(lmm_t *lmm, size_t size, lmm_flags_t flags,
		    int align_bits, DWORD align_ofs,
		    DWORD bounds_min, DWORD bounds_max);
size_t lmm_avail(lmm_t *lmm, lmm_flags_t flags);
void lmm_find_free(lmm_t *lmm, DWORD *inout_addr,
		   size_t *out_size, lmm_flags_t *out_flags);
void lmm_free(lmm_t *lmm, void *block, size_t size);
void lmm_free_page(lmm_t *lmm, void *block);

/* Only available if debugging turned on.  */
void lmm_dump(lmm_t *lmm);
void lmm_stats(lmm_t *lmm);
// OSKIT_END_DECLS


// this part was llm.h

#include <kernel/assert.h>

struct lmm_node
{
	struct lmm_node *next;
	size_t size;
};

#define ALIGN_SIZE	sizeof(struct lmm_node)
#define ALIGN_MASK	(ALIGN_SIZE - 1)

#define CHECKREGPTR(p)	{ \
	assertk((reg->nodes == 0 && reg->free == 0) \
	       || (DWORD)reg->nodes >= reg->min); \
	assertk(reg->nodes == 0 || (DWORD)reg->nodes < reg->max); \
	assertk(reg->free >= 0); \
	assertk(reg->free <= reg->max - reg->min); \
}

__END_DECLS
#endif
