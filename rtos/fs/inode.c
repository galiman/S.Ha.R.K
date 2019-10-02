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
 * CVS :        $Id: inode.c,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

#include <fs/util.h>
#include <fs/magic.h>
#include <fs/assert.h>

#include "fs.h"
#include "dentry.h"
#include "inode.h"
#include "inodeop.h"
#include "mutex.h"
#include "super.h"
#include "superop.h"

#include "debug.h"

#define MAXHASH     64
#define MAXHASHMASK 0x3f

static struct inode *ihash[MAXHASH];
static __fs_mutex_t hashmutex;

/* WARNING: must be greather than MAXDENTRY */
#define MAXINODES 256

static __fs_mutex_t mutex;
static struct inode *freeinode;
static struct inode ino[MAXINODES];

#ifdef FSMUTEXCHECK

void debug_dump_inode_mutex(void)
{
  //printk(KERN_DEBUG "inode mutex: %i",(int)mutex);
  //printk(KERN_DEBUG "inode mutex hasm: %i",(int)hashmutex);
}

static __fs_mutex_t regmutex;
static __fs_mutex_t reghashmutex;

int debug_check_inode_mutex(void)
{
  /*
  if (mutex!=regmutex) {
    printk(KERN_DEBUG "inode mutex CHANGED!");
    return 0;
  }
  if (hashmutex!=reghashmutex) {
    printk(KERN_DEBUG "inode hasmmutex CHANGED!");
    return 0;
  }
  */
  return -1;
}

void debug_save_inode_mutex(void)
{
  regmutex=mutex;
  reghashmutex=hashmutex;
}

#endif

/* WARNIG: if you want to aquire "mutex" and "hashmutex" you MUST
 * aquire first "hashmutex" and then "mutex" (if you do not want
 * a deadlock)
 */

void inode_stats(void)
{
  struct inode *ptr;
  int c,i,l,d;
  
  __fs_mutex_lock(&hashmutex);
  __fs_mutex_lock(&mutex);

  printk(KERN_INFO "max inodes:        %4i",MAXINODES);

  c=0;
  ptr=freeinode;
  while (ptr!=NULL) {
    c++;
    ptr=ptr->i_next;
  }
  printk(KERN_INFO "  free inodes:     %4i",c);

  for (i=0,l=0,c=0,d=0;i<MAXHASH;i++) {
    ptr=ihash[i];
    while (ptr!=NULL) {
      c++;
      if (ptr->i_dlink!=0) l++;
      if (ptr->i_dirty) d++;
      ptr=ptr->i_next;
    }
  }
  printk(KERN_INFO "  cached inodes:   %4i",c);
  printk(KERN_INFO "    linked inodes: %4i",l);
  printk(KERN_INFO "    dirty inodes:  %4i",d);
  
  __fs_mutex_unlock(&mutex);
  __fs_mutex_unlock(&hashmutex);
}

int inode_init(void)
{
  int i;
  
  __fs_mutex_init(&mutex);
  __fs_mutex_init(&hashmutex);

#ifdef FSMUTEXCHECK
  debug_save_inode_mutex();
#endif
  
  for (i=0;i<MAXINODES;i++) {

    /* for safety */
    memset(ino+i,0,sizeof(struct inode));
    magic_clr(ino[i].magic);

    __rwlock_init(&ino[i].i_lock);
    ino[i].i_next=&ino[i+1];    
  }
  freeinode=&ino[0];
  ino[MAXINODES-1].i_next=NULL;

  for (i=0;i<MAXHASH;i++)
    ihash[i]=NULL;
  
  return 0;
}

static struct inode *__get_inode(void)
{
  struct inode *ptr;

  //if (debug_info) cprintf("__get_inode start\n");
  		    
  ptr=freeinode;
  if (freeinode!=NULL) freeinode=freeinode->i_next;

  //if (debug_info) cprintf("__get_inode middle\n");
 
  if (ptr!=NULL) {

    //if (debug_info) cprintf("__get_inode assert\n");
    
    magic_assert_ext(ptr->magic,ZERO_MAGIC,inode_stats,
		 "inode_get: inode MAGIC number failed!\n");
    //magic_assert(ptr->magic,ZERO_MAGIC,
    //    		"inode_get: inode MAGIC number failed!\n");

    //if (debug_info) cprintf("__get_inode assert ok\n");
    
    magic_set(ptr->magic,INODE_MAGIC);

    //if (debug_info) cprintf("__get_inode magic setted\n");
 
  }

  //if (debug_info) cprintf("__get_inode end\n");
  
  return ptr;
}

struct inode *get_inode(void)
{
  struct inode *ptr;

  __fs_mutex_lock(&mutex);
  ptr=__get_inode();
  __fs_mutex_unlock(&mutex);  
  
  return ptr;
}

void free_inode(struct inode *ptr)
{
  magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
	       "inode_free: inode MAGIC number failed!\n");
  magic_clr(ptr->magic);

  __fs_mutex_lock(&mutex);
  ptr->i_next=freeinode;
  freeinode=ptr;
  __fs_mutex_unlock(&mutex);
}

/*-------*/

static int hash_fun(__dev_t device, __ino_t inode)
{
  static __uint32_t table[8]={3,5,7,11,13,17,19,23};
  return (table[device&0x07]*(inode+1))&MAXHASHMASK;
}

void __insert_inode(struct inode *ptr)
{
  int h;

  magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
	       "inode_insert: inode MAGIC number failed!\n");

  /* these could be done with no mutex syncronization */
  h=hash_fun(ptr->i_st.st_dev,ptr->i_st.st_ino);
  ptr->i_hash=h;
  ptr->i_prev=NULL;

  /* these NO */
  ptr->i_next=ihash[h];
  if (ihash[h]!=NULL) ihash[h]->i_prev=ptr;
  ihash[h]=ptr;
  ptr->i_dlink=1;

}

void insert_inode(struct inode *ptr)
{  
  __fs_mutex_lock(&hashmutex);
  __insert_inode(ptr);
  __fs_mutex_unlock(&hashmutex);
}

static void __remove_inode(struct inode *ptr)
{
  magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
	       "inode_remove: inode MAGIC number failed!\n");
  
  if (ptr->i_next!=NULL) ptr->i_next->i_prev=ptr->i_prev;
  if (ptr->i_prev!=NULL) ptr->i_prev->i_next=ptr->i_next;
  else ihash[ptr->i_hash]=ptr->i_next;
}

void remove_inode(struct inode *ptr)
{  
  __fs_mutex_lock(&hashmutex);
  _assert(ptr->i_dlink==0);
  __remove_inode(ptr);
  __fs_mutex_unlock(&hashmutex);
  
}

static struct inode *__purge_inode(void)
{
  struct inode *ptr;
  int i;
  
  /* Well, this can be implemented better; actually a scan for the
   * unused inode into the buckets is done.
   * (for examples the unused inodes can be mantained into an unused
   * queue)
   */
  for (i=0;i<MAXHASH;i++) {
    ptr=ihash[i];
    while (ptr!=NULL) {
      if (ptr->i_dlink==0) {
	__remove_inode(ptr);
	return ptr;
      }
      ptr=ptr->i_next;
    }
  }
  
  return NULL;
}

/* find & remove the oldest dentry from the tree */
/*
static struct inode *purge_inode(void)
{
  struct inode *act;
  __fs_mutex_lock(&hashmutex);
  act=__purge_inode();  
  __fs_mutex_unlock(&hashmutex);
  return act;
}
*/

static struct inode *__catch_inode(void)
{
  struct inode *ptr;


  //if (debug_info) cprintf("get\n");

  
  ptr=get_inode(); /* aquiring "mutex" is required!! don't call __get_inode */
  if (ptr==NULL) {
    //if (debug_info) cprintf("get FAIL\n");
    
    ptr=__purge_inode();

    //if (debug_info) cprintf("catch\n");

    if (ptr==NULL) {
      //if (debug_info) cprintf("catch FAIL\n");
    }
  }

  return ptr;
}

/* get an inode by searching into the free space or purging an old one */
struct inode *catch_inode(void)
{
  struct inode *ptr;

  //if (debug_info) cprintf("catch called\n");
  
  __fs_mutex_lock(&hashmutex);

  //if (debug_info) cprintf("calling __catch...\n");
  
  ptr=__catch_inode();

  //if (debug_info) cprintf("__catch exiting\n");
  
  __fs_mutex_unlock(&hashmutex);
  
  return ptr;
}

struct inode *find_inode(__dev_t device, __ino_t ino)
{
  struct inode *ptr;
  int h;
  
  //h=hash_fun(ptr->i_st.st_dev,ptr->i_st.st_ino);
  h=hash_fun(device,ino);
  __fs_mutex_lock(&hashmutex);
  ptr=ihash[h];
  while (ptr!=NULL) {
    if (ptr->i_st.st_dev==device&&ptr->i_st.st_ino==ino) {
      ptr->i_dlink++;
      break;
    }
    ptr=ptr->i_next;
  }

  if (ptr==NULL) {
    ptr=__catch_inode();
    if (ptr!=NULL) {
      ptr->i_st.st_dev=device;
      ptr->i_st.st_ino=ino;
      __insert_inode(ptr);      
    }
  } else
    magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
		 "find_inode: inode MAGIC number failed!\n");
      
  __fs_mutex_unlock(&hashmutex);

  return ptr;  
}

/* unlock inode */
void unlock_inode(struct inode *ptr)
{
  magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
	       "unlock_inode: inode MAGIC number failed!\n");
  
  __fs_mutex_lock(&hashmutex);
  _assert(ptr->i_dlink>0);  
  ptr->i_dlink--;
  __fs_mutex_unlock(&hashmutex);
}

/* unlock & remove inode from hash table */
int destroy_inode(struct inode *ptr)
{
  int ret;
  magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
		   "unlock_inode: inode MAGIC number failed!\n");
  
  __fs_mutex_lock(&hashmutex);
  _assert(ptr->i_dlink>0);  
  if (ptr->i_dlink!=1) {
    __fs_mutex_unlock(&hashmutex);
    return -1;
  }
  
  if (ptr->i_dirty) {
    ret=ptr->i_sb->sb_op->write_inode(ptr);
    if (ret!=0) {
      __fs_mutex_unlock(&hashmutex);
      return -1;
    }
  }
  
  ptr->i_dlink--;
  __remove_inode(ptr);
  __fs_mutex_unlock(&hashmutex);
  return 0;
}

/* unlock, remove the inode from the hast table
 * erase inode on disk
 */
int erase_inode(struct inode *ptr)
{
  int ret;
  magic_assert_ext(ptr->magic,INODE_MAGIC,inode_stats,
		   "erase_inode: inode MAGIC number failed!\n");
  
  __fs_mutex_lock(&hashmutex);
  _assert(ptr->i_dlink>0);
  
  if (ptr->i_dlink!=1) {
    __fs_mutex_unlock(&hashmutex);
    return -1;
  }

  ptr->i_op->destroy(ptr);
  ret=ptr->i_sb->sb_op->delete_inode(ptr);
  if (ret==0) {
    ptr->i_dlink--;
    __remove_inode(ptr);
  }
  
  __fs_mutex_unlock(&hashmutex);
  return ret;
}
