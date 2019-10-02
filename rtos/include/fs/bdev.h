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
 * CVS :        $Id: bdev.h,v 1.2 2003/03/13 13:37:58 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:37:58 $
 */

#ifndef __FS_BDEV_H__
#define __FS_BDEV_H__

#include <fs/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define BDEV_OK    1
#define BDEV_FAIL  0
#define BDEV_ERROR -1

int bdev_trylock(__dev_t device);
int bdev_tryunlock(__dev_t device);

int bdev_read(__dev_t dev, __blkcnt_t blocknum, __uint8_t *buffer);
int bdev_write(__dev_t dev, __blkcnt_t blocknum, __uint8_t *buffer);
int bdev_seek(__dev_t dev, __blkcnt_t blocknum);

__uint8_t bdev_getdeffs(__dev_t device);

__dev_t bdev_find_byname(char *name);
__dev_t bdev_find_byfs(__uint8_t fsind);
int bdev_scan_devices(int(*callback)(__dev_t,__uint8_t));
int bdev_findname(__dev_t device, char **majorname, char **minorname);

/* for debug purpose */
void bdev_dump_names(void);
void bdev_dump_status(void);

__END_DECLS
#endif
