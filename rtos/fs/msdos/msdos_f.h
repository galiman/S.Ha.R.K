
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


#ifndef __MSDOS_F_H__
#define __MSDOS_F_H__

#include <fs/types.h>

struct msdos_file_info {
  /* these informations are redundant but are used to increase performance */
  __uint16_t  cluster; /* actual cluster */
  __uint32_t  lsector; /* actual logical sector */
  __uint16_t  sectnum; /* sector number (relative to start of cluster) */
  __uint16_t  bpos;    /* byte position (relative to start of sector) */
  /**/
  __uint16_t  bakcluster; /* see msdos_i.c at __advance() & __increase() */
};

#define MSDOS_F(ptr) ((ptr)->u.msdos_f)

#endif
