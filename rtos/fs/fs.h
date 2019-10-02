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
 * CVS :        $Id: fs.h,v 1.3 2005/01/08 14:59:23 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.3 $
 * Last update: $Date: 2005/01/08 14:59:23 $
 */

/***
 This file contains the registration functions for filesystems mudules and the blocking functions
 used internally for filesystem primitive calls.
 ***/

#ifndef __FS_H__
#define __FS_H__

#include <fs/types.h>
#include <fs/mount.h>
#include <time.h>

#include "mutex.h"
#include "semaph.h"

struct super_block;

/*+
 This structure is used to inform the file system manager that a new
 filesystem is ready to bind data on some device:

 name
   It's the name of the filesystem; can be any string.
 fs_flags
   I don't remember what it is
 fs_ind
   It's a unique identifier for the filesystem; it is taken from the
   PC IBM hard disk architecture
 read_super
   It's the main entry point to the fileystem: the mount function call
   this routine to perform the initial binding between a disk device and
   a filesystem; the struct super_block it is the only thing that
   the filesystem manager required to perform a disk operation.

 +*/
struct file_system_type {
  /*+ filesystem name +*/
  const char *name;
  /*+ filesystem flags +*/
  int        fs_flags;
  /*+ filesystem indicator +*/
  __uint8_t  fs_ind;

  /*+ filesystem read_super function +*/
  struct super_block *(*read_super)(__dev_t device, __uint8_t fs_ind,
				    struct mount_opts *options);
};


int filesystem_register(struct file_system_type *ptr);
struct file_system_type *filesystem_find_byid(__uint8_t fs_ind);

int mount_device(__dev_t device, __uint8_t fs_ind, char *pathname,
		 struct mount_opts *opts);

int umount_device(__dev_t device);

/*
 *
 *
 *
 */

/* define this if you want to debug the synchronization functions */
#define FS_DEBUGSYNC KERN_DEBUG
#undef FS_DEBUGSYNC

#ifdef FS_DEBUGSYNC
#define _printk0(fmt,args...) cprintf(fmt,##args)
#else
#define _printk0(fmt,args...)
#endif

/*+ state of the fs: 0 can accept syscalls - 1 abort all syscalls +*/
extern int              fssysstate;
/*+ number of syscalls in progress +*/
extern long             fssyscounter;
/*+ mutex to update the above two variables +*/ 
extern __fs_fastmutex_t fssysmutex;
/*+ syncronization semaphore to change fssysstate +*/
extern __fs_sem_t       fssyssync;
/*+ number of task waiting for fs +*/
extern long             fssysinitcounter;
/*+ syncronization semaphore to change fssysstate from -1 to 0 +*/
extern __fs_sem_t       fssysinit;

/*+ prolog for all syscalls +*/
static __inline__ int __call_to_fs(void)
{
  __fs_fastmutex_lock(&fssysmutex);
  if (fssysstate!=0) {
    __fs_fastmutex_unlock(&fssysmutex);
    return -1;
  }
  fssyscounter++;
  __fs_fastmutex_unlock(&fssysmutex);
  return 0;
}

#define call_to_fs() { if (__call_to_fs()) return -ENOENT; }

/*+ release temporally a syscall (very dangerous) +*/
static __inline__ void release_sys_call(void)
{
  __fs_fastmutex_lock(&fssysmutex);
  fssyscounter--;
  __fs_fastmutex_unlock(&fssysmutex);
}

/*+ reaquire a syscall (MUST be tested the return value for abort) +*/
static __inline__ int reacquire_sys_call(void)
{
  int ret=0;
  __fs_fastmutex_lock(&fssysmutex);
  if (fssysstate==1) ret=-1;
  else fssyscounter++;
  __fs_fastmutex_unlock(&fssysmutex);
  return ret;
}

/*+ epilog for all syscalls +*/
static __inline__ int __return_from_fs(int code)
{ 
  __fs_fastmutex_lock(&fssysmutex);
  fssyscounter--; 
  if (fssysstate==1) { 
    if (fssyscounter==0) {
      __fs_fastmutex_unlock(&fssysmutex);
      __fs_sem_signal(&fssyssync);
      return (code);
    }
  }
  __fs_fastmutex_unlock(&fssysmutex);
  return (code);
}

#define return_from_fs(code) return __return_from_fs(code)

#define fs_test_cancellation(code) \
{  \
  if (proc_table[exec_shadow].control & KILL_ENABLED && \
      proc_table[exec_shadow].control & KILL_REQUEST ) { \
    __return_from_fs(0); \
    task_testcancel(); \
    return (code); \
  } \
}

/*+ called only one time when the system is ready +*/
static __inline__ void permit_all_fs_calls(void)
{
  _printk0("<pfscall>");
  __fs_fastmutex_lock(&fssysmutex);
  fssysstate=0;
  if (fssysinitcounter>0) {
    __fs_fastmutex_unlock(&fssysmutex);
    _printk0("<pfscall:%li>",fssysinitcounter);
    while (fssysinitcounter-->0) 
      __fs_sem_signal(&fssysinit);
    return;
  }
  __fs_fastmutex_unlock(&fssysmutex);
}

#define SHUTDOWNTIMEOUT 6000000
#define SHUTDOWNSLICE    250000
#define SHUTDOWNCOUNTER (SHUTDOWNTIMEOUT/SHUTDOWNSLICE)

/*+ called only one during shutdown to disable syscalls +*/
static __inline__ void block_all_fs_calls(void)     
{
  _printk0("<bfscall>");
  __fs_fastmutex_lock(&fssysmutex);
  fssysstate=1;
  if (fssyscounter>0) {
    _printk0("<bfscallfor%li>",fssyscounter);
    __fs_fastmutex_unlock(&fssysmutex);
    _printk0("<bfscallwait>");

#ifdef SHUTDOWNTIMEOUT
    {
      int counter;
      struct timespec delay;
      delay.tv_sec=SHUTDOWNSLICE/1000000;
      delay.tv_nsec=(SHUTDOWNSLICE%1000000)*1000;
      counter=0;
      while (counter<SHUTDOWNCOUNTER&&__fs_sem_trywait(&fssyssync)) {
	counter++;
	nanosleep(&delay, NULL);
      }
      if (counter>=SHUTDOWNCOUNTER) {
	printk(KERN_NOTICE "filesystem shutdown timeout... aborting!");
	exit(371);
      }
    }
#else
    __fs_sem_wait(&fssyssync);
#endif
    
    _printk0("<bfscallresum>");
  } else {
    __fs_fastmutex_unlock(&fssysmutex);
  }
}


#endif
