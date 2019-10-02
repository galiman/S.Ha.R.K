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
 * CVS :        $Id: bdev.h,v 1.2 2003/03/13 13:43:13 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:43:13 $
 */

#ifndef __BDEV_H__
#define __BDEV_H__

#include "glue.h"
#include <fs/bdevinit.h>
#include <fs/bdev.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define MAXBLOCKDEVICE 8

struct block_device {
  
#ifdef _PARANOIA
  __uint32_t magic;
#endif

  char       *bd_name;
  int        bd_sectorsize;
  
  __uint32_t bd_flag_used:1;
  
  struct block_device_operations *bd_op;
};

#define MAXBLOCKDEVICEMINOR 16

struct block_device_minor {

#ifdef _PARANOIA
  __uint32_t magic;
#endif

  __dev_t              bdm_device;
  __uint8_t            bdm_fsind;
#define MAXDEVICENAME 16
  char                 bdm_name[MAXDEVICENAME+1];
};

struct block_device_operations {
  int (*_trylock)(__dev_t device);
  int (*_tryunlock)(__dev_t device);
  int (*read)(__dev_t device, __blkcnt_t blocknum, __uint8_t *buffer);
  int (*seek)(__dev_t device, __blkcnt_t blocknum);
  int (*write)(__dev_t device, __blkcnt_t blocknum, __uint8_t *buffer);
};

#define TRYLOCK_OK     BDEV_OK
#define TRYLOCK_CANT   BDEV_FAIL
#define TRYLOCK_ERR    BDEV_ERROR
#define TRYUNLOCK_OK   TRYLOCK_OK 
#define TRYUNLOCK_CANT TRYLOCK_CANT
#define TRYUNLOCK_ERR  TRYLOCK_ERR

int bdev_register(__dev_t major,char *name,struct block_device *);
int bdev_register_minor(__dev_t device, char *name,__uint8_t fsind);

int bdev_read(__dev_t dev, __blkcnt_t blocknum, __uint8_t *buffer);
int bdev_seek(__dev_t dev, __blkcnt_t blocknum);
int bdev_write(__dev_t dev, __blkcnt_t blocknum, __uint8_t *buffer);

__END_DECLS
#endif
