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
 * CVS :        $Id: inode.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 This file defines an inode structure and prototypes the functions used to manipulate that structure.
 ***/

#ifndef __INODE_H__
#define __INODE_H__

#include <fs/fsconf.h>
#include <fs/types.h>
#include <fs/stat.h>

#include "rwlock.h"

#ifdef MSDOS_FILESYSTEM
#include "msdos/msdos_i.h"
#endif

struct inode {

#ifdef _PARANOIA
  __uint32_t  magic;   /*+ magic number +*/  
#endif

  struct stat i_st;
  
  struct inode *i_next;
  struct inode *i_prev;
  int           i_hash;

  __rwlock_t i_lock;     /* for read/write on inode's file */
  
  __uint16_t i_dlink;   /* quanti dentry puntano a questo? */
  __uint32_t i_dirty:1;
  
  struct inode_operations *i_op;
  struct super_block	  *i_sb;
  
  union {
#ifdef MSDOS_FILESYSTEM
    struct msdos_inode_info msdos_i;
#endif
    unsigned                dummy; 
  } u;
  
};

int inode_init(void);

struct inode *catch_inode(void);
void free_inode(struct inode *);

void insert_inode(struct inode *ptr);
void unlock_inode(struct inode *ptr);
int destroy_inode(struct inode *ptr);

int erase_inode(struct inode *prt);

#endif
