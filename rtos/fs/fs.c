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
 * CVS :        $Id: fs.c,v 1.2 2003/01/07 17:14:05 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/01/07 17:14:05 $
 */

#include "dentry.h"

#include <kernel/func.h>

#include <fs/util.h>
#include <fs/types.h>
#include <fs/const.h>
#include <fs/bdev.h>
#include <fs/fsconf.h>
#include <fs/fsinit.h>
#include <fs/major.h>
#include <fs/fs.h>
#include <fs/sysmacro.h>
#include <fs/fsind.h>
#include <fs/assert.h>
#include <fs/mount.h>
#include <fs/errno.h>
#include <fs/task.h>

#include "mutex.h"
#include "fs.h"
#include "dcache.h"
#include "inode.h"
#include "super.h"
#include "superop.h"
#include "file.h"
#include "fileop.h"
#include "fsconst.h"

#include "debug.h"

// number of filesystem types supported
#define MAXFILESYSTEMS 8

// pointer into "fst"
static int nextfs=0;
// file system that are registered 
static struct file_system_type *fst[MAXFILESYSTEMS];

NOP_mutexattr_t fsdef_mutexattr=NOP_MUTEXATTR_INITIALIZER;
//static __fs_mutex_t    mutex;
void            *fsmutexattr=NULL;

/* if an assert fails into this module this go UP */
int fs_crash=0;


/*
 * used to block all filesystem calls
 */

/*+ state of the fs +*/
/*+ -1: before initialization 0:system calls allowed 1:calls rejected +*/
int              fssysstate=-1;
/*+ number of syscalls in progress +*/
long             fssyscounter=0;
/*+ mutex to update the above two variables +*/ 
__fs_fastmutex_t fssysmutex;
/*+ syncronization semaphore to change fssysstate from 0 to 1 +*/
__fs_sem_t       fssyssync;
/*+ number of task waiting for fs +*/
long             fssysinitcounter=0;
/*+ syncronization semaphore to change fssysstate from -1 to 0 +*/
__fs_sem_t       fssysinit;

/*
 * variables needed to have a proper filesystem shutdown
 */

/*+ number of module that needs filesystem support +*/
long             needcounter=0;
/*+ syncronization semaphore to start a filesystem shutdown +*/
__fs_sem_t       needsync;

/**
   This is the first function that must be called prior to call
   another function if the filesystem module; it initialize every
   sub-module of the filesystem.
   @memo
   @param ptr
   a pointer to a structure that contains information on how initialize
   the filesystem
   @return 
   0 on success else another value in case of error
*/

void filesystem_end(void *);

int filesystem_init(FILESYSTEM_PARMS *ptr)
{
  int result;
  int i,fd;

  printk(KERN_INFO "initializing filesystems...");
  printkb("fs_init: START");

  //__fs_mutex_init(&mutex);

  __fs_fastmutex_init(&fssysmutex);
  __fs_sem_init(&fssyssync,0);
  __fs_sem_init(&needsync,0);
  __fs_sem_init(&fssysinit,0);
  

  fsmutexattr=ptr->fs_mutexattr;
  if (fsmutexattr==NULL) {
    printk(KERN_ERR "filesystem_init failed: mutex attribute not specified!");
    printkb("fs_init: END");
    return -1;
  }
  
  nextfs=0;
  for (i=0;i<MAXFILESYSTEMS;i++) 
    fst[i]=NULL;

  printkb("fs_init: initiating cache...");
  dcache_init();
  
  printkb("fs_init: initiating supers...");
  super_init();
  printkb("fs_init: initiating inodes...");
  inode_init();
  printkb("fs_init: initiating dentries...");
  dentry_init();
  
  /*--*/

#ifdef MSDOS_FILESYSTEM
  printkb("fs_init: initiating FAT16 filesystem...");
  msdos_fs_init(ptr);
#endif
  
  /*--*/

  printkb("fs_init: mounting root...");
  result=mount_root(ptr->device,ptr->fs_ind,ptr->fs_opts);
  if (result!=0) {
    /*if (ptr->fs_showinfo)*/
    printk(KERN_CRIT "Mounting root FAILED! (error: %i)",result);
    /* for safety */
    _assert(result==0);
    printkb("fs_init: END");
    return result;
  }

  /*--*/

  /* must be done after mounting of the root filesystem because
   * the file descriptors table of every process contains a
   * pointer to a 'struct dentry' (the current working directory)
   */
  
  printkb("fs_init: initiating files...");
  file_init();

  /*--*/

  /* to do better: reserve the first 3 file descriptors for
   * console I/O   
   */

  printkb("fs_init: initiating console descriptors...");
  for (i=0;i<3;i++) {
    fd=get_fd(__get_pid(),(void*)-1);
    if (fd!=i) {
      printk(KERN_CRIT "Can't reserve file descriptor %i for console i/o",fd);
      _assert(fd==i);
      return -1;
    }
  }

  /*--*/
  
  printkb("fs_init: initiating shutdown routines...");
  sys_atrunlevel(filesystem_end,NULL,RUNLEVEL_SHUTDOWN);  

  /* -- */
  
  permit_all_fs_calls();
  
  printkb("fs_init: END");
  return 0;
}

/*+
   This function is used to find what filesystem module manage the
   requested file system indicator
   @memo
   @param fs_ind
   the file system to find
   @return
   the file system struct or NULL in case of error
+*/
struct file_system_type *filesystem_find_byid(BYTE fs_ind)
{
  int i;
  for (i=0;i<nextfs;i++)
    if (fst[i]->fs_ind==fs_ind) return fst[i];
  return NULL;
}

/*+
   This function is used by a filesystem to register itself to the
   filesystem manager; can be called more than one time.
   @memo
   @param ptr
   a pointer to the strutc file\_system\_type of that filesystem
   @return
   0 on success, -1 on failure
   @see file\_system\_type
   +*/

int filesystem_register(struct file_system_type *ptr)
{
  if (nextfs==MAXFILESYSTEMS) return -1;
  fst[nextfs]=ptr;
  nextfs++;
  return 0;
}

__uint8_t filesystem_find_byname(char *name)
{
  int i;
  for (i=0;i<nextfs;i++)
    if (!strcmp(name,(char *)fst[i]->name)) return fst[i]->fs_ind;
  return 255;  
}
     
/* --- */

int mount_root(__dev_t device, __uint8_t fs_ind,
	       struct mount_opts *opts)
{
  struct super_block      *sb;
  struct file_system_type *ptr;
  int res;

  printk9("START mount_root()");
  
  ptr=filesystem_find_byid(fs_ind);
  if (ptr==NULL) {
    printk(KERN_CRIT "mount_root(): errore -2");
    return -2;
  }

  printk9("mount_root(): filesystem type '%s'",ptr->name);
  printk9("mount_root(): filesystem entry at %p",(void *)ptr->read_super);
  printk9("mount_root(): filesystem found");

  res=dcache_lockdevice(device);
  _assert(res!=DCACHE_ERR);
  if (res==DCACHE_FAIL) {
    printk(KERN_CRIT "mount_root(): errore -6");
    return -6;
  }

  printk9("mount_root(): device locked");
  
  sb=ptr->read_super(device,fs_ind,opts);
  if (sb==NULL) {
    printk(KERN_CRIT "mount_root(): errore -4");
    return -4;
  }
  sb->sb_dev=device;
  sb->sb_fs=ptr;
  sb->sb_parent=NULL;
  sb->sb_childs=NULL;
  sb->sb_next=NULL;
  
  printk9("mount_root(): super read");
  
  set_root_superblock(sb);
  
  res=set_root_dentry(sb);
  if (res) {
    printk(KERN_CRIT "mount_root(): errore -5");
    return -5;
  }

  printk9("mount_root(): root dentry set");

  /* so other task can use this superblock */
  super_release(sb);

  printk9("mount_root(): super available");

  {
    char *n0,*n1;
    bdev_findname(device,&n0,&n1);
    printk(KERN_INFO "mounted %s%c%s (%s) on /",
	   n0,DIRDELIMCHAR,n1,
	   opts->flags&MOUNT_FLAG_RW?"rw":"ro");
  }
  
  printk9("END mount_root()");

  return 0;
}

/* --- */

int mount_device(__dev_t device, __uint8_t fs_ind, char *pathname,
		 struct mount_opts *opts)
{
  struct file_system_type *fs;
  struct super_block      *sb,*psb;
  struct dentry           *de;
  int ret,res;

  printk9("START mount_device()");
  
  fs=filesystem_find_byid(fs_ind);
  if (fs==NULL) {
    printk(KERN_ERR
	   "mount_device(): can't find filesystem");
    return -ENODEV;
  }

  de=find_dentry_from(NULL,pathname);
  if (de==NULL) {
    printk(KERN_ERR
	   "mount_device(): can't find directory entry");
    return -ENOENT;
  }

  if (!__S_ISDIR(de->d_inode->i_st.st_mode)) {
    printk(KERN_ERR
	   "mount_device(): pathname is not a directory");
    unlock_dentry(de);
    return -ENOTDIR;
  }
  
  res=dcache_lockdevice(device);
  _assert(res!=DCACHE_ERR);
  if (res==DCACHE_FAIL) {
    unlock_dentry(de);
    printk(KERN_ERR "mount_device(): can't lock device");
    return -EBUSY;
  }
  
  sb=fs->read_super(device,fs_ind,opts);
  if (sb==NULL) {
    unlock_dentry(de);
    printk(KERN_ERR
	   "mount_device(): can't read superblock");
    return -EINVAL;
  }
  sb->sb_dev=device;
  sb->sb_fs=fs;
  psb=de->d_sb;
  
  ret=mount_dentry(sb,de);
  if (ret) {
    unlock_dentry(de);
    super_freeblock(sb);
    printk(KERN_ERR
	   "mount_device(): can't associate directory entry with superblock");
    return -EBUSY;    
  }

  /* insert superblock into tree */
  super_insertintotree(sb,psb);
  
  /* so other task can use this superblock */
  super_release(sb);
  
  {
    char *n0,*n1;
    bdev_findname(device,&n0,&n1);
    printk(KERN_INFO "mounted %s%c%s (%s) on %s",
	   n0,DIRDELIMCHAR,n1,
	   opts->flags&MOUNT_FLAG_RW?"rw":"ro",
	   pathname);
  }

  printk9("END mount_device()");
  return 0;
}

/*
int mount_device(__dev_t device, __uint8_t fs_ind, char *pathname,
		 struct mount_opts *opts)
{
  int res;
  //__fs_mutex_lock(&mutex);
  res=__mount_device(device,fs_ind,pathname,opts);
  //__fs_mutex_unlock(&mutex);
  return res;
}
*/

int umount_device(__dev_t device)
{
  char path[1024];
  struct super_block *sb;
  int res;
  char *n0,*n1;

  printk9("START umount_device()");

  if (device==get_root_device()) {
    printk9(KERN_ERR
	   "umount_device(): can't umount root");
    return -EBUSY;
  }
  
  sb=super_aquire(device);
  if (sb==NULL) {
    printk9(KERN_ERR
	   "umount_device(): can't aquire super block");
    printk(KERN_ERR "can't unmount device 0x%04x",device);
    return -EBUSY;
  }

  printk9("getting pathname");

  /* get names for logging */
  getfullname_dentry(sb->sb_droot,path,sizeof(path));
  bdev_findname(device,&n0,&n1);

  printk9("checking for childs super_blocks");
  
  if (sb->sb_childs!=NULL) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_device(): super block has childs");
    printk(KERN_ERR "can't unmount %s%c%s from %s",n0,DIRDELIMCHAR,n1,path);
    return -EBUSY;    
  }
  
  printk9("calling virtual function put_super");

  res=sb->sb_op->put_super(sb);
  if (res!=0) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_device(): can't put_super");
    printk(KERN_ERR "can't unmount %s%c%s from %s",n0,DIRDELIMCHAR,n1,path);
    return -EIO;
  }

  printk9("calling umount_dentry");

  res=umount_dentry(sb);
  if (res!=0) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_device(): can't umount directory tree");
    printk(KERN_ERR "can't unmount %s%c%s from %s",n0,DIRDELIMCHAR,n1,path);
    return -EBUSY;
  }
  
  /* this is not needed but allows sync of cache with hard-diskes */
  res=dcache_purgedevice(device,0);
  /*
    the dentry has been unmounted so we must continue
  if (res!=0) {
    super_release(sb);
    printk(KERN_ERR
	   "umount_device(): can't sync cache with hard-disk");
    return -EIO;
  }
  */
  
  res=dcache_unlockdevice(device);
  _assert(res!=DCACHE_ERR);
  if (res!=DCACHE_OK) {
    // we must not go here
    _assert(0==-1);
    super_release(sb);
    printk9(KERN_ERR
	   "umount_device(): can't unlock device.cache");
    printk(KERN_ERR "can't unmount %s%c%s from %s",n0,DIRDELIMCHAR,n1,path);
    return -EBUSY;
  }

  /* remove superblock from tree */
  super_removefromtree(sb);
  
  /* free superblock! */
  super_freeblock(sb);

  printk(KERN_INFO "unmounted %s%c%s from %s",n0,DIRDELIMCHAR,n1,path);

  printk9("END umount_device()");
  return EOK;
}

/*
int umount_device(__dev_t device)
{
  int res;
  //__fs_mutex_lock(&mutex);
  res=__umount_device(device);
  //__fs_mutex_unlock(&mutex);
  return res;
}
*/

int umount_root(void)
{
  struct super_block *sb;
  __dev_t device;
  int res;
  char *n0,*n1;

  printk9("START umount_root()");
  device=get_root_device();
  
  sb=super_aquire(device);
  if (sb==NULL) {
    printk9(KERN_ERR
	   "umount_root(): can't aquire super block");
    printk(KERN_ERR "can't unmount device 0x%04x",device);
    return -EBUSY;
  }

  /* get names for logging */
  bdev_findname(device,&n0,&n1);

  printk9("unsetting root");

  set_root_superblock(NULL);  
  set_root_dentry(NULL);

  printk9("calling umount_dentry");
  
  res=umount_dentry(sb);
  printk9("called umount_dentry");
  if (res!=0) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_root(): can't umount directory tree");
    printk(KERN_ERR "can't unmount %s%c%s from /",n0,DIRDELIMCHAR,n1);
    return -EBUSY;
  }

  printk9("calling virtual function put_super");

  /* */
  res=sb->sb_op->put_super(sb);
  if (res!=0) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_root(): can't put_super");
    printk(KERN_ERR "can't unmount %s%c%s from /",n0,DIRDELIMCHAR,n1);
    return -EIO;
  }
  
  /* this is not needed but allows sync of cache with hard-diskes */
  res=dcache_purgedevice(device,0);
  if (res!=0) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_root(): can't sync cache with hard-disk");
    printk(KERN_ERR "can't unmount %s%c%s from /",n0,DIRDELIMCHAR,n1);
    return -EIO;
  }
  
  res=dcache_unlockdevice(device);
  _assert(res!=DCACHE_ERR);
  if (res!=DCACHE_OK) {
    super_release(sb);
    printk9(KERN_ERR
	   "umount_root(): can't unlock device.cache");
    printk(KERN_ERR "can't unmount %s%c%s from /",n0,DIRDELIMCHAR,n1);
    return -EBUSY;
  }

  /* free superblock! */
  super_freeblock(sb);

  printk(KERN_INFO "unmounted %s%c%s from /",n0,DIRDELIMCHAR,n1);

  printk9("END umount_root()");
  return EOK;
}

/* -- */

int suspend_fs_shutdown(void)
{
  SYS_FLAGS f;
  _printk0("<ssd%i>",exec_shadow);
  f=kern_fsave();
  if (needcounter==-1) {
    kern_frestore(f);
    _printk0("<ssd%ifail>",exec_shadow);
    return -1;
  }
  needcounter++;
  kern_frestore(f);
  _printk0("<ssd%i,%li>",exec_shadow,needcounter);
  return 0;
}

void resume_fs_shutdown(void)
{
  SYS_FLAGS f;
  _printk0("<rfs%i>",exec_shadow);
  f=kern_fsave();
  needcounter--;
  if (needcounter==0) {
    needcounter=-1;
    kern_frestore(f);      
    _printk0("<rfs%isig>",exec_shadow);
    __fs_sem_signal(&needsync);
    return;
  }
  kern_frestore(f);
}

/*+ wait the initialization of the fs +*/
int wait_for_fs_initialization(void)
{
  int ret=0;
  _printk0("<wfi%i>",exec_shadow);
  __fs_fastmutex_lock(&fssysmutex);
  if (fssysstate==-1) {
    fssysinitcounter++;
    __fs_fastmutex_unlock(&fssysmutex);
    _printk0("<wfi%ib>",exec_shadow);
    __fs_sem_wait(&fssysinit);
    _printk0("<wfi%ir>",exec_shadow);
    return 0;
  } else if (fssysstate==1) ret=-1;
  __fs_fastmutex_unlock(&fssysmutex);
  _printk0("<wfi%i,%i>",exec_shadow,ret);
  return ret;
}

void wait_for_notneed(void)
{
  SYS_FLAGS f;
  f=kern_fsave();
  printkc("fs_shut: needcounter=%li",needcounter);
  if (needcounter>0) {
    printkc("fs_shut%i: synch needed (sem=%i)",exec_shadow,
	    internal_sem_getvalue(&needsync));
    kern_frestore(f);
    __fs_sem_wait(&needsync);
    printkc("fs_shut%i: synch OK",exec_shadow);
    return ;
  }
  needcounter=-1;
  kern_frestore(f);
}

/* -- */

static int filecount=0;

static void func2(struct file *f)
{
  filecount++;
  f->f_op->close(f->f_dentry->d_inode,f);
  unlock_dentry(f->f_dentry);
  /* so operations on this file fail */
  f->f_count=0;
  return;
}

static int func(__dev_t device)
{
  int res;
  if (device==get_root_device()) return 0;
  res=umount_device(device);
  if (res) return -1;
  return 0;
}

TASK filesystem_shutdown(void *dummy)
{
  int res;
  
  printkc("fs_shut%i: START",exec_shadow);
  
  /*
  for (res=0;res<4*3;res++) {
    cprintf("Û");
    task_delay(250000l);
  }
  printk(KERN_DEBUG "TASK %i",exec_shadow);
  */

  /* wait for modules */
  printkc("fs_shut%i: waiting for modules that use filesystems",exec_shadow);  
  wait_for_notneed();
  
  /* block all filesystem calls */
  printkc("fs_shut%i: blocking all filesystem system calls",exec_shadow);
  printkc("fs_shut: task into filesystems primitives are %li",fssyscounter);
  block_all_fs_calls();
  
  /* close all files */
  printkc("fs_shut%i: closing all files",exec_shadow);
  enums_file(func2);
  printkc("fs_shut%i: closed %i files",exec_shadow,filecount);

  /* umount all devices */
  printkc("fs_shut: umounting all devices");
  res=super_enumdevice(func);
  if (res!=0) {
    printk(KERN_INFO "filesystems shutdown aborted!");
    printkc("fs_shut: ABORTING");
    return (void*)-1;
  }
  
  /* umount root device */
  printkc("fs_shut: umount root");
  res=umount_root();
  if (res!=0) {
    char *n0,*n1;
    bdev_findname(get_root_device(),&n0,&n1);
    printk(KERN_ERR "can't unmount %s%c%s from /",n0,DIRDELIMCHAR,n1);
    printk(KERN_INFO "filesystems shutdown aborted!");
    printkc("fs_shut: ABORTING");
    return (void*)-1;
  }

  printkc("fs_shut: END");

  return (void*)0;
}

void filesystem_end(void *dummy)
{
  SOFT_TASK_MODEL model;
  PID pid;

  printkc("fs_end: START");
  
  if (fs_crash) {
    printkc("fs_end: END");
    return;
  }
  printk(KERN_INFO "shuting down filesystems...");

  //nrt_task_default_model(model);
  //nrt_task_def_system(model);

  soft_task_default_model(model);
  soft_task_def_system(model);
  soft_task_def_notrace(model);
  soft_task_def_periodic(model);
  soft_task_def_period(model,50000);
  soft_task_def_met(model,10000);
  soft_task_def_wcet(model,10000);

  pid=task_create("ShutFS",filesystem_shutdown,&model,NULL);
  if (pid==-1)
    printk(KERN_ERR "can't start filesystem shutdown task");
  else {
    printkc("fs_end: shutting down task %i activated",pid);
    task_activate(pid);
  }
  	     
  printkc("fs_end: END");
}
