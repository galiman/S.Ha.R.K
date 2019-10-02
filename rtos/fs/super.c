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
 * CVS :        $Id: super.c,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

#include <fs/sysmacro.h>
#include <fs/util.h>
#include <fs/major.h>
#include <fs/fsinit.h>
#include <fs/bdev.h>
#include <fs/fsind.h>
#include <fs/mount.h>
#include <fs/assert.h>

#include "mutex.h"
#include "fs.h"
#include "super.h"

#define MAXSUPERBLOCKS 8
static __fs_mutex_t mutex;
static __fs_mutex_t mutexsb;
static struct super_block sbs[MAXSUPERBLOCKS];

static struct super_block *root_superblock=NULL;

int super_init(void)
{
  int i;
  
  __fs_mutex_init(&mutex);
  __fs_mutex_init(&mutexsb);

  for (i=0;i<MAXSUPERBLOCKS;i++) {
    memset(sbs+i,0,sizeof(struct super_block));
    mount_std_parms(sbs[i].sb_mopts);
    sbs[i].sb_busycount=0;
    sbs[i].sb_used=0;
    sbs[i].sb_blocked=0;
  }

  return 0;
}

struct super_block *super_getblock(void)
{
  struct super_block *ptr=NULL;
  int i;

  __fs_mutex_lock(&mutexsb);
  for (i=0;i<MAXSUPERBLOCKS;i++) 
    if (!sbs[i].sb_used) {
      sbs[i].sb_used=1;
      sbs[i].sb_blocked=1;
      sbs[i].sb_busycount=0;
      ptr=sbs+i;
      break;
    }
  __fs_mutex_unlock(&mutexsb);

  return ptr;
}

void super_freeblock(struct super_block *ptr)
{
  __fs_mutex_lock(&mutexsb);
  _assert(ptr->sb_used==1);
  _assert(ptr->sb_blocked==1);
  _assert(ptr->sb_busycount==0);
  ptr->sb_used=0;
  __fs_mutex_unlock(&mutexsb);
}

/* --- */

struct super_block *get_root_superblock(void)
{
  _assert(root_superblock!=NULL);
  return root_superblock;
}

void set_root_superblock(struct super_block *sb)
{
  __fs_mutex_lock(&mutex);
  if (sb==NULL) {
    _assert(root_superblock!=NULL);
    root_superblock=NULL;
    __fs_mutex_unlock(&mutex);
    return;
  }
  _assert(root_superblock==NULL);
  __fs_mutex_unlock(&mutex);
  root_superblock=sb;
}

/* --- */

int super_incbusy(struct super_block *sb)
{
  int ret=-1;
  
  _assert(sb>=sbs&&sb<sbs+MAXSUPERBLOCKS);
  
  __fs_mutex_lock(&mutex);
  
  if (sb->sb_blocked==0) {
    ret=0;
    sb->sb_busycount++;
  }
  
  __fs_mutex_unlock(&mutex);
  
  return ret;
}

void super_decbusy(struct super_block *sb)
{
  _assert(sb>=sbs&&sb<sbs+MAXSUPERBLOCKS);
  
  __fs_mutex_lock(&mutex);
  
  _assert(sb->sb_busycount>0);
  _assert(sb->sb_blocked==0);

  sb->sb_busycount--;
  
  __fs_mutex_unlock(&mutex);
}

struct super_block *super_aquire(__dev_t dev)
{
  struct super_block *ptr=NULL;
  int i;
  
  __fs_mutex_lock(&mutex);
  
  for (i=0;i<MAXSUPERBLOCKS;i++) {
    if (sbs[i].sb_used==1&&sbs[i].sb_dev==dev) {
      if (sbs[i].sb_blocked==1||sbs[i].sb_busycount!=0) break;      
      sbs[i].sb_blocked=1;
      ptr=sbs+i;
      break;
    }
  }
  __fs_mutex_unlock(&mutex);
  return ptr;
}

void super_release(struct super_block *sb)
{
  _assert(sb>=sbs&&sb<sbs+MAXSUPERBLOCKS);
  __fs_mutex_lock(&mutex);
  _assert(sb->sb_used==1&&sb->sb_blocked==1);
  sb->sb_blocked=0;
  __fs_mutex_unlock(&mutex);
}

/* -- */

void super_insertintotree(struct super_block *sb, struct super_block *psb)
{
  __fs_mutex_lock(&mutex);
  sb->sb_parent=psb;
  sb->sb_next=psb->sb_childs;
  sb->sb_childs=NULL;
  psb->sb_childs=sb; 
  __fs_mutex_unlock(&mutex);
}

void super_removefromtree(struct super_block *sb)
{
  struct super_block *ptr,*prev;
  ptr=sb->sb_parent->sb_childs;
  prev=NULL;
  __fs_mutex_lock(&mutex);
  while (ptr!=sb) {
    prev=ptr;
    ptr=ptr->sb_next;
    _assert(ptr!=NULL);
  }
  if (prev!=NULL) prev->sb_next=sb->sb_next;
  else sb->sb_parent->sb_childs=sb->sb_next;
  __fs_mutex_unlock(&mutex);
}

static int visittree(struct super_block *sb,int(*func)(__dev_t))
{
  struct super_block *ptr;
  ptr=sb->sb_childs;
  while (ptr!=NULL) {
    if (visittree(ptr,func)) return -1;
    ptr=ptr->sb_next;
  }
  return (*func)(sb->sb_dev);
}

/* can be called only during shutdown!
 * (there must be no lock on superblocks tree during shutdown)
 * (the shutdown function is responsable not to call the
 * insert/remove_to/from tree functions)
 */

int super_enumdevice(int(*func)(__dev_t))
{
  int res;
  res=visittree(root_superblock,func);
  return res;
}
