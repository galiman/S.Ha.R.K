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
 * CVS :        $Id: super.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 This file contains the struct super_block and the functions to manage super blocks.
***/

#ifndef __SUPER_H__
#define __SUPER_H__

#include <fs/types.h>
#include <fs/fsconf.h>
#include <fs/mount.h>

#ifdef MSDOS_FILESYSTEM
#include "msdos/msdos_s.h"
#endif

struct super_block {

  /* this are used to build a tree of superblock */
     
  /*+ parent of this superblock +*/
  struct super_block        *sb_parent;
  /*+ childs of this superblock +*/
  struct super_block        *sb_childs;
  /*+ to build a list of childs +*/
  struct super_block        *sb_next;
  
  /*+ device on which the super block reside +*/
  __dev_t                   sb_dev;
  
  /*+ inode of the root directory entry +*/
  struct inode              *sb_root;
  /*+ directory entry into the tree of the root directory +*/
  struct dentry             *sb_droot;
  /*+ file-system of the super block +*/
  struct file_system_type   *sb_fs;
  /*+ pointer to a super operations structure (see superop.h) +*/
  struct super_operations   *sb_op;
  /*+ pointer to a dentry operations structure (see dentryop.h) +*/
  struct dentry_operations  *sb_dop;
  /*+ mount options (see fs/mount.h) +*/
  struct mount_opts         sb_mopts;

  /*+ busy counter (for every operation in progress using this super) +*/
  int                       sb_busycount;
  
  /* flags */
  /*+ set if this element is valid and used +*/
  __uint32_t                sb_used:1;
  /*+ set if blocked (if only one operation must be done) +*/
  __uint32_t                sb_blocked:1;
  
  union {    
#ifdef MSDOS_FILESYSTEM
/*+ msdos filesystem specific super block data +*/
    struct msdos_super_block msdos_sb;
#endif    
    /*+ not used +*/
    unsigned                 dummy;
  } u;
};

/*+ module "super" initialization +*/
int super_init(void);

struct super_block *super_getblock(void);
void super_freeblock(struct super_block *ptr);
int super_enumdevice(int(*func)(__dev_t));

struct super_block *get_root_superblock(void);
void set_root_superblock(struct super_block *sb);
static __inline__ __dev_t get_root_device(void) { return get_root_superblock()->sb_dev; }

struct super_block *super_aquire(__dev_t dev);
void super_release(struct super_block *sb);

int super_incbusy(struct super_block *sb);
void super_decbusy(struct super_block *sb);

void super_insertintotree(struct super_block *sb, struct super_block *psb);
void super_removefromtree(struct super_block *sb);

#endif

