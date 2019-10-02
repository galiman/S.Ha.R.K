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
 * CVS :        $Id: file.c,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

#include <fs/fsconf.h>
#include <fs/util.h>
#include <fs/types.h>
#include <fs/fcntl.h>
#include <fs/stat.h>
#include <fs/errno.h>
#include <fs/maccess.h>
#include <fs/limits.h>
#include <fs/task.h>

#include "mutex.h"
#include "fs.h"
#include "file.h"
#include "fsconst.h"
#include "dentry.h"
#include "fileop.h"
#include "inode.h"
#include "inodeop.h"

#include "debug.h"

/* OSKit stdio need this */
int getdtablesize(void)
{
  return MAXOPENFILES;
}

/*-----------*/

//#if !defined GLOB_DESC_TABLE
//#if !defined LOC_DESC_TABLE
//#error "GLOB_DESC_TABLE or LOC_DESC_TABLE must be defined! (see fsconfig.h)"
//#endif
//#else
//#if defined LOC_DESC_TABLE
//#error "GLOB_DESC_TABLE and LOC_DESC_TABLE are defined! (see fsconfig.h)"
//#endif
//#endif

/*-----------*/

//static __fs_mutex_t mutex;
static __fs_mutex_t mutexreq;

static struct file *freelist;
static struct file filetab[MAXSYSFILE];

//#ifdef GLOB_DESC_TABLE
//#include "fileg.inc"
//#else
//#include "filel.inc"
//#endif

struct file_descriptors desctable[MAXIMUMPROCESS];

int get_fd(__pid_t pid, struct file *f)
{
  int i;
  lock_desctable(pid);
  for (i=0;i<MAXOPENFILES;i++)
    if (desctable[pid].fd_file[i]==NULL) {
      desctable[pid].fd_file[i]=f;
      desctable[pid].fd_file[i]->f_count=1;
      desctable[pid].fd_flags[i]=0;
      desctable[pid].fd_fflags[i]=0;
      unlock_desctable(pid);
      return i;
    }
  unlock_desctable(pid);
  return -1;
}

int __duplicate_fd(__pid_t pid, int fd, int low)
{
  while (low<MAXOPENFILES) {
    if (desctable[pid].fd_file[low]==NULL) {
      desctable[pid].fd_file[low]=desctable[pid].fd_file[fd];
      desctable[pid].fd_file[fd]->f_count++;
      return low;
    }
    low++;
  }
  return -1;
}


int __isvalid_fd(__pid_t pid, int fd)
{
  return desctable[pid].fd_file[fd]!=NULL;
}


struct file *free_fd(__pid_t pid, int fd)
{
  struct file *ret;
  lock_desctable(pid);
  ret=desctable[pid].fd_file[fd];
  desctable[pid].fd_file[fd]=NULL;  
  unlock_desctable(pid);
  return ret;  
}

static int subfile_init(void)
{
  int i,j;
  for (j=0;j<MAXIMUMPROCESS;j++) {

    strcpy(desctable[j].fd_cwd,ROOTDIRNAME);
    desctable[j].fd_cwden=get_root_dentry();
    get_root_dentry()->d_lock++;
    desctable[j].fd_umask=__DEFUMASK;

    /* TO FIX : 0,1,2 are reserved file descriptors */
    /*
    desctable[j].fd_file[0]=(void*)(-1);
    desctable[j].fd_flags[0]=0;
    desctable[j].fd_fflags[0]=0;
    desctable[j].fd_file[1]=(void*)(-1);
    desctable[j].fd_flags[1]=0;
    desctable[j].fd_fflags[1]=0;
    desctable[j].fd_file[2]=(void*)(-1);
    desctable[j].fd_flags[2]=0;
    desctable[j].fd_fflags[2]=0;
    */
    for (i=0;i<MAXOPENFILES;i++) {
      desctable[j].fd_file[i]=NULL;
      desctable[j].fd_flags[i]=0;
      desctable[j].fd_fflags[i]=0;
    }

    
    init_mutex(j);
  }
  
  return 0;
}

/*-----------*/

/*
 * DANGER: must be called AFTER the mounting of the root device!
 * because the subfile_init() use get_root_dentry()
 */

int file_init(void)
{
  int i;
  //__fs_mutex_init(&mutex);
  __fs_mutex_init(&mutexreq);
  freelist=filetab;
  for (i=0;i<MAXSYSFILE;i++) {
    memset(filetab+i,0,sizeof(struct file));
    filetab[i].f_next=filetab+i+1;
    filetab[i].f_count=0;
  }
  filetab[MAXSYSFILE-1].f_next=NULL;

  return subfile_init();  
}

/**/

struct file *__get_file(void)
{
  struct file *f;
  if (freelist==NULL) return NULL;    
  f=freelist;
  freelist=f->f_next;
  _assert(f->f_count==0);
  f->f_count=1;
  return f;
}

/* get a file struct from the free queue */
struct file *get_file(struct inode *ptr)
{
  /* NB: ptr is not used yet */
  struct file *f;
  __fs_mutex_lock(&mutexreq);
  //cprintf("Û{");
  f=__get_file();
  __fs_mutex_unlock(&mutexreq);
  //cprintf("Û}");
  return f;
}

/**/

static void __free_file(struct file *f)
{
  _assert(f->f_count==1);
  f->f_count=0;
  f->f_next=freelist;
  freelist=f;
}

/* insert file struct into the free queue */
void free_file(struct file *f)
{
  __fs_mutex_lock(&mutexreq);
  //cprintf("Û{");
  __free_file(f);
  __fs_mutex_unlock(&mutexreq);
  //cprintf("Û}");
}

/* enumerate all files */
void enums_file(void (*func)(struct file *))
{
  int i;
  __fs_mutex_lock(&mutexreq);
  //cprintf("Û{");
  for (i=0;i<MAXSYSFILE;i++)
    if (filetab[i].f_count!=0) func(filetab+i);
  __fs_mutex_unlock(&mutexreq);
  //cprintf("Û}");
}


/*
 *
 * DUMMY file operations
 *
 */

__off_t dummy_lseek(struct file *f, __off_t off, int whence)
{
  return -ENOSYS;
}

__ssize_t dummy_read(struct file *f, char *p, __ssize_t d)
{
  return -ENOSYS;
}

__ssize_t dummy_write(struct file *f, char *p, __ssize_t d)
{
  return -ENOSYS;
}

int dummy_readdir(struct file *f, void *e)
{
  return -ENOSYS;
}

int dummy_open(struct inode *i, struct file *f)
{
  return -ENOSYS;
}

int dummy_close(struct inode *i, struct file *f)
{
  return -ENOSYS;
}

/*
 *
 * for function inlines
 *
 *
 */

#ifdef MULTITHREAD

void lock_desctable(__pid_t pid)
{
  __mutex_lock(&desctable[pid].fd_mutex);
}

void unlock_desctable(__pid_t pid)
{
  __mutex_unlock(&desctable[pid].fd_mutex);
}

void init_mutex(__pid_t pid)
{
  __mutex_init(&desctable[pid].fd_mutex,fsmutexattr);
}

#else

void lock_desctable(__pid_t pid)
{}

void unlock_desctable(__pid_t pid)
{}

void init_mutex(__pid_t pid)
{}

#endif

__mode_t *umask_ptr(__pid_t pid)
{
  return &desctable[pid].fd_umask;
}

char *cwd_ptr(__pid_t pid)
{
  return desctable[pid].fd_cwd;
}

struct dentry *cwden_ptr(__pid_t pid)
{
  return desctable[pid].fd_cwden;
}

struct file *file_ptr(__pid_t pid, int i)
{
  return desctable[pid].fd_file[i];
}






