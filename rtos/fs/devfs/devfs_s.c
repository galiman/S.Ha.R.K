/*
 * Project: HARTIK (HA-rd R-eal TI-me K-ernel)
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *               Gerardo Lamastra <gerardo@sssup.it>
 *
 * Authors     : Massimiliano Giorgi <massy@hartik.sssup.it>
 * (see authors.txt for full list of hartik's authors)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it 
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
 * CVS :        $Id: devfs_s.c,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

#include <fs/util.h>
#include <fs/types.h>
#include <fs/stat.h>
#include <fs/fsinit.h>
#include <fs/fsind.h>

#include "dcache.h"
#include "fs.h"

static struct super_block *devfs_read_super(__dev_t dev, __uint8_t fs_ind);

static struct file_system_type devfs=
  {"devfs",0,FS_MSDOS,devfs_read_super};

int devfs_fs_init(FILESYSTEM_PARMS *ptr)
{
  filesystem_register(&devfs);
  return 0;
}


/*-----------------------------------------*/

/* 
 *
 * Super operations
 *
 */

static int devfs_read_inode(struct inode *in);
static int devfsrite_inode(struct inode *in);

static struct super_operations devfs_super_operations={
  devfs_read_inode,
  devfs_write_inode
};

static int msdos_read_inode(struct inode *in)
{
  struct directoryentry *den;
  dcache_t *ptr;
  __uint16_t  cluster;
  __uint32_t lsector;
  __uint16_t  deoffs,offs;
  
  /*i_sb gia' messo*/
  /*i_ino gia' messo*/

  den=(struct directoryentry *)(ptr->buffer)+offs;
		   
  in->i_dev=in->i_sb->sb_dev;
  in->i_mode=__S_IRWXO|__S_IRWXG|__S_IRWXU;
  in->i_mode|=__S_IFREG);
  in->i_nlink=1;
  in->i_uid=0;
  in->i_gid=0;
  in->i_rdev=0;
  in->i_size=den->size;  
  in->i_atime=in->i_mtime=in->i_ctime=0;
  in->i_op=&devfs_inode_operations;

  return 0;
}

static int msdos_write_inode(struct inode *in)
{
  return -1;
}

/*
 *
 * Mount operation
 *
 */

static struct super_block *devfs_read_super(__dev_t dev, __uint8_t fs_ind)
{
  struct inode       *in;
  struct bootrecord  *br;
  struct super_block *sb;
  dcache_t           *dc;
  int                res;
  
  sb=super_getblock();
  if (sb==NULL) {
    dcache_unlock(dc);
    return NULL;
  }

  sb->sb_dev=dev;
  //sb->sb_op=&devfs_super_operations;
  //sb->sb_dop=&devfs_dentry_operations;
  
  in=inode_get();
  if (in==NULL) {
    super_freeblock(sb);
    return NULL;    
  }

  sb->sb_root=in;
  in->i_sb=sb;
  in->i_ino=0; /* ??? */

  return sb;
}
