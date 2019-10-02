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
 CVS :        $Id: mount.h,v 1.2 2003/03/13 13:37:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:37:58 $
 ------------
**/

/*
 * Copyright (C) 2000 Massimiliano Giorgi
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
 *
 *
 *
 *
 */

#ifndef __FS_MOUNT_H__
#define __FS_MOUNT_H__

#include <fs/types.h>
#include <fs/fsconf.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*
 *
 * SPECIFIC FILESYSTEM PART
 *
 */

#define mount_std_parms(obj) {(obj).flags=0;}

/*
 * msdos filesystem
 */

#ifdef MSDOS_FILESYSTEM

#define MSDOS_FLAG_ONORCASE 0x01

struct msdosfs_parms {
  __uint32_t flags;
};

#define msdosfs_std_parms(o) {mount_std_parms(o);(o).x.msdos.flags=0;}

#else

struct msdosfs_parms {
  __uint32_t dummy;
};

#endif

/*
 * iso9660 filesystem
 */

#ifdef ISO9660_FILESYSTEM

struct iso9660fs_parms {
  __uint32_t dummy;
};

#define iso9660fs_std_parms(o) {mount_std_parms(o);(o).x.iso9660.dummy=0;}

#else

struct iso9660fs_parms {
  __uint32_t dummy;
};

#endif

/*
 *
 * GENERAL FILESYSTEM PART
 *
 */

#define MOUNT_FLAG_RW 0x01

struct mount_opts {
  __uint32_t flags;
  union {
    struct msdosfs_parms   msdos;
    struct iso9660fs_parms iso9660;
  } x;
};

typedef struct mount_opts MOUNT_PARMS;

__END_DECLS
#endif
