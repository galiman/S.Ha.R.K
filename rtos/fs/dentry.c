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
 * CVS :        $Id: dentry.c,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

#include <fs/util.h>
#include <fs/types.h>
#include <fs/assert.h>
#include "fs.h"
#include "super.h"
#include "superop.h"
#include "dentry.h"
#include "dentryop.h"
#include "inode.h"
#include "inodeop.h"
#include "fsconst.h"
#include "mutex.h"

#include "debug.h"

/*
 * DEBUG
 */

/* if defined:
 * a short message is printed on console when a task go in/out of
 * functions dentry_find() & dentry_unlock()
 * (after  and before the lock/unlock of the mutex!)
 */
#define DEBUGFINDUNLOCK KERN_DEBUG
#undef DEBUGFINDUNLOCK

#ifdef DEBUGFINDUNLOCK
#define printdd(fmt,pid) kern_printf(fmt,pid)
#else
#define printdd(fmt,pid)
#endif

/* if you modify this you must modify MAXINODE */
#define MAXDENTRY 128

static __fs_mutex_t mutex;
static __fs_fastmutex_t mutexreq;
static struct dentry *freelist;
//static struct dentry *leaflist;
static struct dentry direntry[MAXDENTRY];

static struct dentry *root_direntry=NULL;

#ifdef FSMUTEXCHECK
void debug_dump_dentry_mutex(void)
{
  //printk(KERN_DEBUG "dentry mutex: %i",(int)mutex);
  //printk(KERN_DEBUG "dentry mutex req: %i",(int)mutexreq);
}

//static __fs_mutex_t regmutex;
//static __fs_mutex_t regmutexreq;

int debug_check_dentry_mutex(void)
{
  /*
  if (mutex!=regmutex) {
    printk(KERN_DEBUG "dentry mutex CHANGED! (prev: %i)",regmutex);
    return 0;
  }
  if (mutexreq!=regmutexreq) {
    printk(KERN_DEBUG "dentry mutexreq CHANGED! (prev: %i)",regmutexreq);
    return 0;
  }
  */
  return -1;
}

void debug_save_dentry_mutex(void)
{
  //regmutex=mutex;
  //regmutexreq=mutexreq;
}
#endif

int dentry_init(void)
{
  int i;
  
  __fs_mutex_init(&mutex);
  __fs_fastmutex_init(&mutexreq);

#ifdef FSMUTEXCHECK
  debug_save_dentry_mutex();
#endif
  
  freelist=direntry;
  for (i=0;i<MAXDENTRY;i++) {
    memset(direntry+i,0,sizeof(struct dentry));
    magic_set(direntry[i].magic,DENTRY_MAGIC);
    direntry[i].d_next=direntry+i+1;
    direntry[i].d_lock=-1;
  }
  direntry[MAXDENTRY-1].d_next=NULL;
  return 0;
}

static __inline__ struct dentry *__get_dentry(void)
{
  struct dentry *den;
  if (freelist==NULL) return NULL;    
  den=freelist;
  freelist=den->d_next;
  
  _assert(den->d_lock==-1);
  magic_assert(den->magic,DENTRY_MAGIC,"get_dentry: dentry MAGIC failed!");
  
  den->d_lock=0;
  return den;
}

/* get a dentry from the free queue */
struct dentry *get_dentry(void)
{
  struct dentry *den;
  __fs_fastmutex_lock(&mutexreq);
  den=__get_dentry();
  __fs_fastmutex_unlock(&mutexreq);
  return den;
}

static __inline__ void __free_dentry(struct dentry *den)
{
  _assert(den>=direntry&&den<direntry+MAXDENTRY);
  _assert(den->d_lock==0);
  magic_assert(den->magic,DENTRY_MAGIC,"free_dentry: dentry MAGIC failed!");
  
  den->d_lock=-1;
  den->d_next=freelist;
  freelist=den;
}

/* insert dentry into the free queue */
void free_dentry(struct dentry *den)
{
  __fs_fastmutex_lock(&mutexreq);
  __free_dentry(den);
  __fs_fastmutex_unlock(&mutexreq);
}

static void __insert_dentry(struct dentry *den, struct dentry *parent)
{
  _assert(den>=direntry&&den<direntry+MAXDENTRY);
  magic_assert(den->magic,DENTRY_MAGIC,"insert_dentry: dentry MAGIC failed!");
  
  den->d_acc=gettimek();
  den->d_parent=parent;
  den->d_child=NULL;
  den->d_prev=NULL;
  den->d_next=parent->d_child;
  if (parent->d_child!=NULL)
    parent->d_child->d_prev=den;
  parent->d_child=den;
}

/* insert a dentry into the tree */
void insert_dentry(struct dentry *den, struct dentry *parent)
{
  __fs_mutex_lock(&mutex);
  __insert_dentry(den,parent);
  __fs_mutex_unlock(&mutex);
}

static void __remove_dentry_only(struct dentry *den)
{
  _assert(den>=direntry&&den<direntry+MAXDENTRY);
  magic_assert(den->magic,DENTRY_MAGIC,"remove_dentry: dentry MAGIC failed!");
  _assert(den->d_lock==0);
  
  if (den->d_prev!=NULL) 
    den->d_prev->d_next=den->d_next;
  else 
    den->d_parent->d_child=den->d_next;
  if (den->d_next!=NULL)
    den->d_next->d_prev=den->d_prev;
}

static __inline__ void __remove_dentry(struct dentry *den)
{
  __remove_dentry_only(den);
  unlock_inode(den->d_inode);  
}

/* remove a dentry from the tree */
void remove_dentry(struct dentry *den)
{
  __fs_mutex_lock(&mutex);
  __remove_dentry(den);
  __fs_mutex_unlock(&mutex);
}

/* find the oldest dentry into the tree */
/*
 * (potrebbe essere fatto in maniera non ricorsiva usando un
 * puntatore alle foglie e scandendo questa lista invece di visitare
 * tutto l'albero!)
 *
 * si potrebbe usare un task NRT periodico per mantenere liberi (se
 * possibile l'X% delle dentry libere.
 */

static struct dentry *__recurse(struct dentry *den, struct dentry *act)
{
  while (den!=NULL) {    
    if (den->d_child!=NULL) {
      act=__recurse(den->d_child,act);
    } else {
      if (den->d_lock==0) {
	if (act==NULL) 
	  act=den;
	else {
	  if (den->d_acc<act->d_acc)
	    act=den;
	}
      }      
    }
    den=den->d_next;
  }
  return act;
}

struct dentry *__purge_dentry(void)
{
  struct dentry *act;
  act=__recurse(root_direntry,NULL);
  if (act!=NULL) __remove_dentry(act);
  return act;
}

/* find & remove the oldest dentry from the tree */
struct dentry *purge_dentry(void)
{
  struct dentry *act;
  __fs_mutex_lock(&mutex);
  act=__purge_dentry();  
  __fs_mutex_unlock(&mutex);
  return act;
}

struct dentry *__catch_dentry(void)
{
  struct dentry *den;
  den=__get_dentry();
  if (den==NULL) den=__purge_dentry();
  return den;
}

/* get a dentry by searching into the free space or purging an old one */
/*
static struct dentry *catch_dentry(void)
{
  struct dentry *den;
  __fs_mutex_lock(&mutex);
  den=__catch_dentry();  
  __fs_mutex_unlock(&mutex);
  return den;
}
*/

/*---------------------*/

int set_root_dentry(struct super_block *sb)
{
  struct dentry *den;

  __fs_mutex_lock(&mutex);
  if (sb==NULL) {
    _assert(root_direntry!=NULL);
    root_direntry->d_lock--;
    _assert(root_direntry->d_lock==1);
    root_direntry=NULL;
    __fs_mutex_unlock(&mutex);
    return 0;
  }
  _assert(root_direntry==NULL);
  __fs_mutex_unlock(&mutex);
  
  den=get_dentry();
  if (den==NULL) return -1;

  den->d_next=NULL;
  den->d_prev=NULL;
  den->d_parent=NULL;
  den->d_child=NULL;
  
  den->d_acc=0;
  den->d_name.nameptr=ROOTDIRNAME;

  den->d_op=sb->sb_dop;
  den->d_sb=sb;
  den->d_inode=sb->sb_root;
  
  den->d_lock=1;
  
  sb->sb_droot=den;
  root_direntry=den;

  return 0;
}

struct dentry *get_root_dentry(void)
{
  _assert(root_direntry!=NULL);
  return root_direntry;
}

#ifdef DEBUG_FIND_DUMP
static void dump_dentry(struct dentry *ptr)
{
  printk7(KERN_DEBUG "dentry dump:");
  printk7(KERN_DEBUG "  name : %s",QSTRNAME(&ptr->d_name));
  printk7(KERN_DEBUG "  lock : %i",ptr->d_lock);
  printk7(KERN_DEBUG "  inode: %i",ptr->d_inode->i_st.st_ino);
}
#else
#define dump_dentry(x)
#endif

/*--------------*/

/*
 * flags for createflags:
 * DENTRY_NOCREATE   -> can't create a new inode
 * DENTRY_CANCREATE  -> an inode (only the last) can be created
 * DENTRY_MUSTCREATE -> the last inode MUST be created (it must no exist)
 *
 * return flags for createflags:
 * DENTRY_CREATED -> the last inode has been created
 * DENTRY_EXIST   -> (error) the last inode already exist
 *
 * return NULL on error (not found or DENTRY_EXIST)
 *
 * NB:
 * if createflag is NULL a DENTRY_NOCREATE is assumed
 * 'act' can be NULL if pathname is an absolute path
 */

struct dentry *find_dentry_from_ext(struct dentry *act,
				    char *pathname,
				    int *createflag)
{
  struct dentry *ptr;
  struct dentry *den;
  struct inode *ino;
  struct qstr str;
  char *sp;
  char *s;
  int  found,created;

  /* too much 'goto' :-( and 'if' */

  sp=pathname;
  if (*pathname==DIRDELIMCHAR) {
    act=root_direntry;
    sp++;
  }
  if (act==NULL) return NULL;
  
  printdd("°f<%i>",exec_shadow);
  __fs_mutex_lock(&mutex);
  printdd("±f<%i>",exec_shadow);
  
  if (*sp=='\0') goto END0;

  created=0;
  found=1;
  while (found) {
    s=strchr(sp,DIRDELIMCHAR);
    if (s!=NULL) *s='\0';
    
    printk7("dentry find: searching for %s...",sp);

    /* for handle special directory entry "." and ".." */
    if (*sp=='.') {
      if (*sp=='\0') {
	/* found "." special directory */
	printk7("dentry find: special directory entry '.'");
	found=1;
	if (s==NULL) goto END0;
	*s=DIRDELIMCHAR;
	sp=s+1;
	s=strchr(sp,DIRDELIMCHAR);
	continue;	
      }
      if (*sp=='.') {
	if (*sp=='\0') {
	  /* found ".." special directory */
	  printk7("dentry find: special directory entry '..'");
	  found=1;
	  if (act->d_parent!=NULL) act=act->d_parent;
	  if (s==NULL) goto END0;
	  *s=DIRDELIMCHAR;
	  sp=s+1;
	  s=strchr(sp,DIRDELIMCHAR);
	  continue;	
	}	
      }
    }
    
    ptr=act->d_child;
    found=0;
    str.nameptr=sp;
    while (ptr!=NULL) {
      if (ptr->d_op->d_compare(ptr,&ptr->d_name,&str)==0) {

	printk7("dentry find: found!");
	
	found=1;
	act=ptr;
	if (s==NULL) goto END0;
	*s=DIRDELIMCHAR;
	sp=s+1;
	s=strchr(sp,DIRDELIMCHAR);
	break;
      }
      ptr=ptr->d_next;
    }        
  }

  printk7("dentry find: NOT found!");
  
  for (;;) {
    den=__catch_dentry();
    if (den==NULL) {
      if (s!=NULL) *s=DIRDELIMCHAR;
      printk7("dentry find: can't have new struct dentry!");
      act=NULL;
      goto END1;
    }

    den->d_acc=0;
    den->d_name.nameptr=NULL;
    strcpy(den->d_name.name,sp);
    den->d_lock=0;
    den->d_op=act->d_op;
    den->d_sb=act->d_sb;

    printk7("dentry find: looking up for %s....",sp);
    dump_dentry(act);
    
    ino=act->d_inode->i_op->lookup(act->d_inode,den);

    printk7("dentry find: end looking");
    
    if (ino==NULL) {

      printk7("dentry find: NOT found!");
      if (s!=NULL) *s=DIRDELIMCHAR;      

      if (createflag==NULL) {
	__free_dentry(den);
	act=NULL;
	goto END1;
      }
      
      if ((s==NULL)&&(*createflag&DENTRY_CANCREATE)) {

	printk7("dentry find: check for read-only fs");
	if (!(act->d_sb->sb_mopts.flags&MOUNT_FLAG_RW)) {
	  printk7("dentry find: read-only fs.. creating new inode fails");
	  __free_dentry(den);
	  act=NULL;
	  goto END1;
	}
	
	printk7("dentry find: try to create a new inode");
	ino=act->d_inode->i_op->create(act->d_inode,den);

	if (ino==NULL) {  
	  printk7("dentry find: inode creation fail");
	  __free_dentry(den);
	  act=NULL;
	  goto END1;
	}
	printk7("dentry find: new inode created");
	created=1;
	
      } else {
	__free_dentry(den);
	act=NULL;
	goto END1;
      }
    }
    den->d_inode=ino;
    
    insert_inode(ino);   
    printk7("dentry find: found!");
    
    __insert_dentry(den,act);
    act=den;
  
    if (s==NULL) goto END2;
    
    sp=s+1;
    *s=DIRDELIMCHAR;    
    s=strchr(sp,DIRDELIMCHAR);
    if (s!=NULL) *s='\0';
  }

  /* for safety */
  _assert(0==-1);

END0:
  /* 'if found' we are here */
  if (createflag!=NULL) {
    *createflag&=~DENTRY_CREATEMASK;
    if (*createflag&DENTRY_MUSTCREATE) {
      _assert(act!=NULL);
      *createflag|=DENTRY_EXIST;
      act=NULL;
    }
  }
  goto END;

END1:
  /* 'if error' we are here */
  _assert(act==NULL);
  if (createflag!=NULL) *createflag&=~DENTRY_CREATEMASK;
  goto END;

END2:
  /* 'if ok' (found or created) we are here */
  _assert(act!=NULL);
  if (createflag!=NULL) {
    if (created) {
      *createflag&=~DENTRY_CREATEMASK;		  
      *createflag|=DENTRY_CREATED;
    } else if ((*createflag&DENTRY_MUSTCREATE)==DENTRY_MUSTCREATE) {
      *createflag&=~DENTRY_CREATEMASK;		  
      *createflag|=DENTRY_EXIST;
      act=NULL;
    } else
      *createflag&=~DENTRY_CREATEMASK;		  	  
  }
  goto END;    

END:
  if (act!=NULL) act->d_lock++;

  printdd("²f<%i>",exec_shadow);
  __fs_mutex_unlock(&mutex);
  printdd("Ûf<%i>",exec_shadow);
  return act;
  
}

void unlock_dentry(struct dentry *den)
{
  printdd("°u<%i>",exec_shadow);
  __fs_mutex_lock(&mutex);
  printdd("±u<%i>",exec_shadow);

  _assert(den>=direntry&&den<direntry+MAXDENTRY);
  if (den->d_lock<=0) {
    printk(KERN_DEBUG "unlock_dentry on %s",QSTRNAME(&den->d_name));
  }
  _assert(den->d_lock>0);
  
  den->d_lock--;
  //if (den->d_lock==0) {
  //  _assert(den->d_inode!=NULL);
  //  unlock_inode(den->d_inode);
  //}
  
  printdd("²u<%i>",exec_shadow);
  __fs_mutex_unlock(&mutex);
  printdd("Ûu<%i>",exec_shadow);
}

/*
 *
 */

static void __print_name(struct dentry *act,char *buffer)
{
  if (act->d_parent==NULL) return;
  __print_name(act->d_parent,buffer);
  strcat(buffer,ROOTDIRNAME);
  strcat(buffer,QSTRNAME(&act->d_name));
  return;
}

static char *print_name(struct dentry *act)
{
  static char buffer[1024];
  buffer[0]='\0';
  __print_name(act,buffer);
  return buffer;
}

void getfullname_dentry(struct dentry *act, char *buffer, int size)
{
  _assert(act>=direntry&&act<direntry+MAXDENTRY);
  magic_assert(act->magic,DENTRY_MAGIC,
	       "getfullname_dentry: dentry MAGIC failed!");

  buffer[0]='\0';
  __print_name(act,buffer);
  _assert(strlen(buffer)<size);
}

static void visit_tree(struct dentry *act)
{
  struct dentry *ptr;
  if (act==root_direntry) printk(KERN_DEBUG "  %s",ROOTDIRNAME);
  else { char *s=print_name(act); printk(KERN_DEBUG "  %s",s);}
  ptr=act->d_child;
  while (ptr!=NULL) {
    visit_tree(ptr);
    ptr=ptr->d_next;
  }
  return;
}

void dump_dentry_tree(void)
{
  printk(KERN_DEBUG "DTREE dump");
  visit_tree(root_direntry);
}

/*
 *
 */

#ifdef DEBUGPURGECHILD
#define dprint(fmt,args...) printk(fmt,##args)
#else
#define dprint(fmt,args...)
#endif

static __inline__ int __purgeallchilds_mg(struct dentry *de)
{
  int ret;
  struct dentry *ptr,*ptr2,*ptr3;

  dprint(KERN_DEBUG "START mg purgechild");

  {
#ifdef DEBUGPURGECHILD
    char *s=print_name(de);
    dprint(KERN_DEBUG "for %s",s);
#endif
  }
  
  /*
   * Phase 1
   * purge all childs (if possible!)
   */

  ptr=de->d_child;
  ret=0;
  
  /* why these? I do not want to use recursion... (or goto statement)*/
  for (;;) {
    /* if there is a child... continue using the child's chain */
    if (ptr->d_child!=NULL) {
      ptr=ptr->d_child;
      continue;      
    }    
    /* if a child dentry is locked... can't continue! */
    if (ptr->d_lock) {
      #ifdef DEBUGPURGECHILD
      char *s=print_name(ptr);
      dprint(KERN_DEBUG "%s is locked!!!",s);
      #endif
      ret=-1;
      break;
    }
    /* purge this dentry */
    ptr2=ptr->d_next;
    ptr3=ptr->d_parent;

    dprint(KERN_DEBUG "%s destroy",QSTRNAME(&ptr->d_name));
    ret=destroy_inode(ptr->d_inode);
    if (ret) break;
    __remove_dentry_only(ptr);
    
    /* if there is not another child... continue with the parent */
    if (ptr2==NULL) {
      ptr=ptr3;
      if (ptr==de) break;
      continue;
    }
    /* continue with next child */
    ptr=ptr2;
  }

  dprint(KERN_DEBUG "END mg purgechild");  
  return ret;
}

static __inline__ int __purgeallchilds_pj(struct dentry *x)
{
  struct dentry *p,*q;

  dprint(KERN_DEBUG "START pj purgechild");
  
  p = x->d_child;
  q = NULL;
  for(;;) {
    /* scorro i figli */
    while(p) {
      q = p;
      p = p->d_next;
    }

    if (q == x) {
      dprint(KERN_DEBUG "END pj purgechild");  
      return 0; /* finito */
    }

    if (q->d_child) {
      p = q->d_child;
      q = NULL;
    }
    else {
      /* sono su un nodo senza figli */
      /* q punta al nodo */
      if (q->d_prev)
	{ /* non e' il primo figlio */
	  q = q->d_prev;
	  if (q->d_next->d_lock) {
	    dprint(KERN_DEBUG "%s is locked",
		   QSTRNAME(&q->d_next->d_name));
	    return -1;
	  }
	  if (destroy_inode(q->d_next->d_inode)) {
	    dprint(KERN_DEBUG "error destroing %s",
		   QSTRNAME(&q->d_next->d_name));
	    return -1;
	  }
	  dprint(KERN_DEBUG "removing %s",QSTRNAME(&q->d_next->d_name));
	  __remove_dentry_only(q->d_next);
	  q->d_next = NULL;
	  p = NULL;
	}
      else {
	/* e' il primo figlio */
	q = q->d_parent;
	if (q->d_child->d_lock) {
	    dprint(KERN_DEBUG "%s is locked",
		   QSTRNAME(&q->d_child->d_name));
	  return -1;
	}
	if (destroy_inode(q->d_child->d_inode)) {
	    dprint(KERN_DEBUG "error destroing %s",
		   QSTRNAME(&q->d_child->d_name));
	  return -1;
	}
	dprint(KERN_DEBUG "removing %s",QSTRNAME(&q->d_child->d_name));
	__remove_dentry_only(q->d_child);
	q->d_child = NULL;
	p = NULL;
      }
    }
  }
  
}

#define __purgeallchilds(de) __purgeallchilds_mg(de)

int mount_dentry(struct super_block *sb, struct dentry *de)
{
  int ret;

  printk9("START mount_dentry()!");
  printk9("for %s",QSTRNAME(&de->d_name));
  
  _assert(de!=NULL);
  _assert(sb!=NULL);
  
  __fs_mutex_lock(&mutex);

  printk9("locked and ready");

  if (de->d_lock>1) {
    printk9("LOCKED!");    
    /* someone is using this dentry! */
    __fs_mutex_unlock(&mutex);    
    return -1;    
  }

  /*
   * Phase 1
   */

  /* purge all childs (if possible) */
  
  if (de->d_child!=NULL) {
    ret=__purgeallchilds(de);      
    if (ret) {
      printk9("PURGECHILD!");      
      __fs_mutex_unlock(&mutex);
      return ret;
    }
    
  }
  
  /*
   * Phase 2
   */

  ret=destroy_inode(de->d_inode);
  if (ret) {
    printk9("DESTROY INODE!");    
    __fs_mutex_unlock(&mutex);
    return ret;
  }
  
  de->d_op=sb->sb_dop;
  de->d_inode=sb->sb_root;
  de->d_sb=sb;

  sb->sb_droot=de;
  
  __fs_mutex_unlock(&mutex);
  printk9("END mount_dentry()!");
  return 0;
}

int umount_dentry(struct super_block *sb)
{
  struct dentry *de;
  int res;

  printk9("START umount_dentry()");

  __fs_mutex_lock(&mutex);

  _assert(sb!=NULL);
  _assert(sb->sb_droot!=NULL);
  if (sb->sb_droot->d_child!=NULL) {
    res=__purgeallchilds(sb->sb_droot);
    if (res) {      
      printk9("PURGING CHILD!");    
      __fs_mutex_unlock(&mutex);
      return res;
    }
  }

  printk9("umount_dentry: phase 1");
  
  de=sb->sb_droot;
  _assert(de->d_lock>0);
  if (de->d_lock!=1) {
    printk9("LOCK COUNT! (%i)",de->d_lock);
    __fs_mutex_unlock(&mutex);
    return -1;
  }
  de->d_lock--;

  printk9("umount_dentry: phase 2");
  
  __remove_dentry_only(de);
  destroy_inode(de->d_inode);
  __free_dentry(de);
  
  printk9("umount_dentry: phase 3");
    
  sb->sb_droot=NULL;
  sb->sb_root=NULL;
  
  __fs_mutex_unlock(&mutex);
  
  printk9("END umount_dentry()!");
  return 0;
}

int unlink_dentry(struct dentry *d)
{
  struct inode *in;
  int res;

  printkd("unlink_dentry: START");
  
  __fs_mutex_lock(&mutex);
  if (d->d_lock!=1) return EBUSY;

  in=d->d_inode;

  printkd("unlink_dentry: dentry ok");

  __rwlock_wrlock(&in->i_lock);  

  printkd("unlink_dentry: locked inode");
  
  res=in->i_op->unlink(d);

  printkd("unlink_dentry: inode unlinked from dentry");
  
  if (res==0&&in->i_st.st_nlink==0) {
    printkd("unlink_dentry: no more links erasing inode");
    erase_inode(in);    
  }
  
  __rwlock_wrunlock(&in->i_lock);

  printkd("unlink_dentry: unlocked inode");

  if (res==0) {
    d->d_lock--;
    __remove_dentry_only(d);
    printkd("unlink_dentry: dentry unlinked");
  }
    
  __fs_mutex_unlock(&mutex);
  
  printkd("unlink_dentry: END");
  return -res;
}
