
/*
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


#ifndef __MSDOS_S_H__
#define __MSDOS_S_H__

#include <fs/types.h>

struct msdos_super_block {
  __uint32_t lfat;      /* fat start at? (logical sector) */
  __uint32_t lroot;     /* root directory start at? (logical sector) */
  __uint32_t ldata;     /* data start at? (logical sector) */
  __uint32_t lsize;     /* numbers of sectors in volume */
  
  __uint16_t spc;       /* sectors per cluster */
  __uint32_t spf;       /* sectors per fat */
  __uint32_t rootsect;  /* root directory size in sectors */
  __uint16_t nfat;      /* number of fats */
  
  __uint16_t stclust;   /* cluster on which begin search for free cluster */
};

#define MSDOS_SB(ptr) (ptr->u.msdos_sb)

#endif
