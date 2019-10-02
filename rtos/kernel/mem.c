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
 CVS :        $Id: mem.c,v 1.3 2004/02/10 17:50:04 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2004/02/10 17:50:04 $
 ------------

 Basic Memory Manager Functions:

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


#include <kernel/func.h>
#include <kernel/lmm.h>

static lmm_t lmm;
static lmm_region_t region1M;
static lmm_region_t region16M;
static lmm_region_t region;

void kern_mem_init(struct multiboot_info *multiboot)
{
  DWORD lowsize, highsize;
  LIN_ADDR lowp, highp;

  X_meminfo(&highp, &highsize, &lowp, &lowsize);

  lmm_init(&lmm);

  /* Three memory regions:
     - under 1Mb
     - from 1Mb to 16Mb
     - all the other memory...
  */
  lmm_add_region(&lmm, &region1M , (void*)0x00000000, 0x000FFFFF,
                                   MEMORY_UNDER_1M,       0);
  lmm_add_region(&lmm, &region16M, (void*)0x00100000, 0x00FFFFFF,
                                   MEMORY_FROM_1M_TO_16M, 1);
  lmm_add_region(&lmm, &region   , (void*)0x01000000, 0xFEFFFFFF,
                                   0              ,       2);

  if (lowsize)
    lmm_add_free(&lmm, (void *)lowp, lowsize-1);
  if (highsize)
    lmm_add_free(&lmm, (void *)highp+0x100000, highsize-0x100000);
}

void *kern_alloc(DWORD s)
{
  if (s==0) return NULL;
  return lmm_alloc(&lmm, s, 0);
}

void *kern_alloc_aligned(size_t size, lmm_flags_t flags,
			int align_bits, DWORD align_ofs)
{
  return lmm_alloc_aligned(&lmm, size, flags, align_bits, align_ofs);
}

void *kern_alloc_page(lmm_flags_t flags)
{
  return lmm_alloc_page(&lmm, flags);
}

void *kern_alloc_gen(size_t size, lmm_flags_t flags,
		    int align_bits, DWORD align_ofs,
		    DWORD bounds_min, DWORD bounds_max)
{
  return lmm_alloc_gen(&lmm, size, flags, align_bits, align_ofs,
		       bounds_min, bounds_max);
}

void kern_free(void *block, size_t size)
{
  lmm_free(&lmm, block, size);
}

void kern_free_page(void *block)
{
  lmm_free_page(&lmm, block);
}

void *DOS_alloc(DWORD size)
{
  return lmm_alloc(&lmm, size, MEMORY_UNDER_1M);
}

void DOS_free(void *ptr,DWORD size)
{
  lmm_free(&lmm, ptr, size);
}


void kern_mem_dump(void)
{
  lmm_dump(&lmm);
  lmm_stats(&lmm);
}
