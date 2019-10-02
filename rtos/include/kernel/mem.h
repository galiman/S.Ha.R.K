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
 CVS :        $Id: mem.h,v 1.2 2003/03/13 13:36:28 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:36:28 $
 ------------

- Memory Management functions

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

/*---------------------------------------------------------------------*/
/* Memory Management                                                   */
/*---------------------------------------------------------------------*/

#ifndef __KERNEL_MEM_H__
#define __KERNEL_MEM_H__

#include <kernel/lmm.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* Memory regions to be passed as flags... */
#define MEMORY_UNDER_1M       1
#define MEMORY_FROM_1M_TO_16M 2
#define MEMORY_UNDER_16M      3

/* init the memory allocator */
void kern_mem_init(struct multiboot_info *multiboot);

/* alloc memory without requirements on the address... */
void *kern_alloc(DWORD s);

/* alloc an aligned block of memory */
void *kern_alloc_aligned(size_t size, lmm_flags_t flags,
			int align_bits, DWORD align_ofs);

/* alloc memory with general constraints */
void *kern_alloc_gen(size_t size, lmm_flags_t flags,
		    int align_bits, DWORD align_ofs,
		    DWORD bounds_min, DWORD bounds_max);

/* free a block */
void kern_free(void *block, size_t size);

/* alloc and free a page */
void *kern_alloc_page(lmm_flags_t flags);
void kern_free_page(void *block);

/* alloc and free memory <1Mb */
void *DOS_alloc(DWORD size);
void DOS_free(void *ptr,DWORD size);

/* statistical stuffs */
void kern_mem_dump(void);

__END_DECLS
#endif
