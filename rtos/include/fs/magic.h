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

/*
 * Copyright (C) 1999 Massimiliano Giorgi
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
 * CVS :        $Id: magic.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:51 $
 */

#ifndef _FS_MAGIC_H
#define _FS_MAGIC_H

#ifdef _PARANOIA

/*
 * _PARANOIA defined
 */

#define ZERO_MAGIC         0x00000000

/* drivers/block/ */ 
#define BLOCK_DEVICE_MAGIC 0xaa1b1234
#define PHDSK_MAGIC        0x129a7412
#define IDE_MAGIC          0x30afc781
#define IDE_MAGIC_MINOR    0x1290aff3

/* fs/ */
#define DCACHE_MAGIC       0x77881299
#define INODE_MAGIC        0x9978100a
#define DENTRY_MAGIC       0x119aaf45

/* log level */
#include <fs/util.h>
#define PARANOIALOG KERN_EMERG

/* assert macros */
#include <fs/assert.h>

#define magic_set(value,magic) \
  { (value)=(magic); }

#include <fs/types.h>
#define DECLARE_MAGIC(x)  __uint32_t x

#else

/*
 * _PARANOIA not defined
 */

#define DECLARE_MAGIC(x)  __uint32_t x
#define magic_set(value,magic) {}

#endif

#define magic_clr(value) magic_set(value,ZERO_MAGIC)

#endif
