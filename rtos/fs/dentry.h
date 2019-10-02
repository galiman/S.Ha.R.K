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
 * CVS :        $Id: dentry.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 The "dentry" structure: it contains a directory entry (only the filename).
 ***/

#ifndef __DENTRY_H__
#define __DENTRY_H__

#include <fs/limits.h>
#include <fs/const.h>
#include <fs/types.h>
#include <fs/magic.h>

struct qstr {
  char name[MAXFILENAMELEN+1];
  char *nameptr;
};

#define QSTRNAME(ptr) ((ptr)->nameptr!=NULL?(ptr)->nameptr:(ptr)->name)

struct dentry {

  DECLARE_MAGIC(magic);
  
  struct dentry *d_next;
  struct dentry *d_prev;
  struct dentry *d_parent;
  struct dentry *d_child;

  __time_t       d_acc;
  struct qstr    d_name;

  short          d_lock;
  
  struct dentry_operations  *d_op;
  struct inode              *d_inode;
  struct super_block        *d_sb; 
};

int dentry_init(void);
int set_root_dentry(struct super_block *sb);
struct dentry *get_root_dentry(void);

struct dentry *get_dentry(void);
void free_dentry(struct dentry *);
void insert_dentry(struct dentry *den, struct dentry *parent);
void remove_dentry(struct dentry *den);
struct dentry *purge_dentry(void);

int unlink_dentry(struct dentry *den);

int mount_dentry(struct super_block *sb, struct dentry *de);
int umount_dentry(struct super_block *sb);

void getfullname_dentry(struct dentry *act, char *buffer, int size);


/* flags for createflags */
#define DENTRY_NOCREATE    0x00
#define DENTRY_CANCREATE   0x01
#define DENTRY_MUSTCREATE  0x03

#define DENTRY_CREATEMASK  0x03

/* return flags for createflags */
#define DENTRY_CREATED     0x01
#define DENTRY_EXIST       0x02

struct dentry *find_dentry_from_ext(struct dentry *,
				    char *pathname,
				    int *createflags);
void unlock_dentry(struct dentry *den);

extern __inline__ struct dentry *find_dentry_from(struct dentry *den,
						  char *pathname)
{
  return find_dentry_from_ext(den,pathname,NULL);
}

#endif
