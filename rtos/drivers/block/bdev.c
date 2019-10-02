/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */


/***************************************

  CVS :        $Id: bdev.c,v 1.2 2004/05/26 11:20:23 giacomo Exp $
 
  Revision:    $Revision: 1.2 $

  Last update: $Date: 2004/05/26 11:20:23 $

  This module contains the block device manager: it export some functions
  to the rest of the kernel that are mapped to a real device driver
  sub-system.

***************************************/

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

#include "glue.h"
#include <fs/bdevconf.h>
#include <fs/bdev.h>
#include <fs/magic.h>
#include <fs/assert.h>
#include <fs/sysmacro.h>
#include <fs/bdevinit.h>
#include <fs/fsind.h>
//#include <fs/mutex.h>

#include "bdev.h"

/*+ Log Level to use +*/
#define BDEVLOG KERN_ERR

/*+ an entry for every block device +*/
static struct block_device bdev[MAXBLOCKDEVICE];

/*+ an index into bdev_minor +*/
int bdev_minor_index=0;

/*+ an entry for every block device minor number +*/
struct block_device_minor bdev_minor[MAXBLOCKDEVICEMINOR];

/*+ some operations must be done in mutual exclusion +*/
NOP_mutexattr_t        bdevdef_mutexattr=NOP_MUTEXATTR_INITIALIZER;
__b_mutex_t            mutex;
void                   *bmutexattr=NULL;
static BDEV_PARMS      defaultparms=BASE_BDEV;

/*++++++++++++++++++++++++++++++++++++++
  
  Initialize the block device manager and then initialize every
  block-device sub-system defined into bdevconf.h (Actualy it is
  impossible to load a module at run-time).

  int bdev_init 
    return 0 on success, other value on error

  BDEV_PARMS *parms
    pointer to a structure that contains all device inizialization parameter
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_init(BDEV_PARMS *parms)
{
  int i;

  if (parms==NULL) parms=&defaultparms;
  
  /* 'bmutexattr' initialization */
  bmutexattr=parms->bmutexattr;
  if (bmutexattr==NULL) bmutexattr=&bdevdef_mutexattr;
  
  /* initialize mutex */
  __b_mutex_init(&mutex);
  
  /* initialize the bdev[] table */
  for (i=0;i<MAXBLOCKDEVICE;i++) {    
    magic_set(bdev[i].magic,BLOCK_DEVICE_MAGIC);    
    bdev[i].bd_flag_used=0;
  }

  /* initialize the bdev_minor[] table */
  /*for (i=0;i<MAX*/
	 
#ifdef IDE_BLOCK_DEVICE
  /* if requested initialize the IDE subsystem */
  ide_init(parms);
#endif

#ifdef LOOP_BLOCK_DEVICE
  /* if requested initialize the "loop device" subsystem */
  loop_init(parms);
#endif

  return 0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Every block device sub-system must call this function yo register
  itself to the device block manager.

  int bdev_register
    return 0 on success, -1 on failure

  __dev_t devmajor
    major block device number to register

  char *name
    the name of the device (must be a pointer to static data)

  struct block_device *dev
    information to hook the block device routine
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_register(__dev_t devmajor, char *name, struct block_device *dev)
{
  __dev_t mj=major(devmajor);

  /* check for consistency */
  if (mj<=0||mj>=MAXBLOCKDEVICE) {
    printk(BDEVLOG "bdev: request to register invalid major %i",
	   mj);
    return -1;
  }
  if (bdev[mj].bd_flag_used) {
    printk(BDEVLOG "bdev: requet to register used major %i",
	   mj);
    return -1;
  }
  magic_assert(bdev[mj].magic,BLOCK_DEVICE_MAGIC,
	       "bdev: device(%i,%i) overwritten prior to use",
	       mj,minor(devmajor));

  /* register the device */
  memcpy(bdev+mj,dev,sizeof(struct block_device));
  bdev[mj].bd_flag_used=1;
  bdev[mj].bd_name=name;
  magic_set(bdev[mj].magic,BLOCK_DEVICE_MAGIC);
  
  return 0;
}

static __uint8_t part2sysind(__uint8_t par_ind)
{
  switch(par_ind) {
    case FSPAR_DOS12:
    case FSPAR_DOS16:
    case FSPAR_DOSBIG:
      return FS_MSDOS;
    case FSPAR_LNXNTV:
      return FS_EXT2;
  }
  return FS_DEFAULT;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Every block device sub-system must call this function to register
  itself to the device block manager (these information are used by
  the devfs filesystem to export device name to the system in a
  POSIX fanshion).

  int bdev_register_minor
    return a minor index on success, <0 on failure

  __dev_t device
    block device to register

  char *name
    the name of the device

  __uint8_t fsind
    file system indicator (see fsind.h)
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_register_minor(__dev_t device, char *name,__uint8_t fsind)
{
  /* checks for bad requests */
  if (bdev_minor_index==MAXBLOCKDEVICEMINOR) return -1;
  if (major(device)>MAXBLOCKDEVICE) return -2;
  if (!bdev[major(device)].bd_flag_used) return -3;

  /* register minor device */
  strncpy(bdev_minor[bdev_minor_index].bdm_name,name,MAXDEVICENAME);
  bdev_minor[bdev_minor_index].bdm_name[MAXDEVICENAME-1]='\0';
  bdev_minor[bdev_minor_index].bdm_device=device;
  bdev_minor[bdev_minor_index].bdm_fsind=part2sysind(fsind);
    
  return bdev_minor_index++;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Find a device using its name.
  
  __dev_t bdev_find_byname
    return a device number on success, <0 on failure

  char *name
    the name of the device to find
  ++++++++++++++++++++++++++++++++++++++*/

__dev_t bdev_find_byname(char *name)
{
  char *s0,*s1,*s2;
  char saved;
  int i;

  /*
   * a name can be, for example, "/dev/ide/hda1";
   * after this code s0 is a pointer to "ide" (the major name) and s1 is
   * a pointer to "hda1" (the minor name)
   */
  s0=name;  
  s1=strchr(name,'/');
  if (s1==NULL) return -1;
  while ((s2=strchr(s1+1,'/'))!=NULL) {
    s0=s1+1;
    s1=s2;
  }
  saved=*s1;
  *s1++='\0';

  /* search for a device... */
  for (i=0;i<bdev_minor_index;i++)
    /* compare a minor name */
    if (!strcmp(s1,bdev_minor[i].bdm_name))
      /* compare a major name */
      if (!strcmp(bdev[major(bdev_minor[i].bdm_device)].bd_name,s0)) {
	*(s1-1)=saved;
	return bdev_minor[i].bdm_device;
      }
  *(s1-1)=saved;
  return -2;
}


/*++++++++++++++++++++++++++++++++++++++
  
  Find major and minor name using a device id (__dev_t).
  
  int bdev_findname
    return 0 on success; -1 on failure.
    
  __dev_t dev
    device identification

  char **major
    the major name is pointed here
    
  char **minor
    the minor name is pointed here
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_findname(__dev_t dev, char **major,char **minor)
{
  int i;
  
  assertk(major!=NULL&&minor!=NULL);
  
  for (i=0;i<bdev_minor_index;i++)
    if (bdev_minor[i].bdm_device==dev) {
      *minor=bdev_minor[i].bdm_name;
      *major=bdev[major(dev)].bd_name;
      return 0;
    }

  return -1;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Find a device using its file system indicator (the first device
  that use this file system).
  
  __dev_t bdev_find_byfs
    return a device number on success, <0 on failure

  __uint8_t fsind
    file system indicator
  ++++++++++++++++++++++++++++++++++++++*/

__dev_t bdev_find_byfs(__uint8_t fsind)
{
  int i;
  for (i=0;i<bdev_minor_index;i++)
    if (bdev_minor[i].bdm_fsind==fsind)
      return bdev_minor[i].bdm_device;		
  return -1;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Scan all devices into the system: for every device found call
  the callback function, if it returns -1 continue scanning else
  the values passed is returned.
  
  __dev_t bdev_find_byfs
    return a device number on success, <0 on failure

  __uint8_t fsind
    file system indicator
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_scan_devices(int(*callback)(__dev_t,__uint8_t))
{
  int res=0;
  int i;
  
  for (i=0;i<bdev_minor_index;i++) {
    res=callback(bdev_minor[i].bdm_device,
		 bdev_minor[i].bdm_fsind
		 );
    if (res!=-1) break;
  }
  return res;
}

//#ifndef NDEBUG

#include <fs/major.h>
extern int ide_dump_startsize(__dev_t, __blkcnt_t*, __blkcnt_t*);
extern void ide_dump_status(void);

void bdev_dump_names(void)
{
  int i;
  __blkcnt_t start,size;
  
  printk(KERN_INFO "block device names:");
  for (i=0;i<bdev_minor_index;i++) {
    start=size=0;
    if (major(bdev_minor[i].bdm_device)==MAJOR_B_IDE)
      ide_dump_startsize(bdev_minor[i].bdm_device,&start,&size);
    printk(KERN_INFO "    %s/%8s   %08x (start-%-8li size-%-8li sysind-%02x)",
	   bdev[major(bdev_minor[i].bdm_device)].bd_name,
	   bdev_minor[i].bdm_name,
	   bdev_minor[i].bdm_device,
	   (long int)start,(long int)size,
	   bdev_minor[i].bdm_fsind	   
	   );
  }
  
}

void bdev_dump_status(void)
{
  int counter,i;
  
  printk(KERN_INFO "block device status:");
  printk(KERN_INFO "  registered devices:");
  for (counter=0,i=0;i<MAXBLOCKDEVICE;i++)
    if (bdev[i].bd_flag_used) {
      printk(KERN_INFO "    %s",bdev[i].bd_name);
      counter++;
    }   
  printk(KERN_INFO "  num. majors: %i",counter);
  printk(KERN_INFO "  num. minors: %i",bdev_minor_index);

#ifdef IDE_BLOCK_DEVICE
  ide_dump_status();
#endif
  
}

//#endif

/*++++++++++++++++++++++++++++++++++++++
  
  Try to lock a device.
  Locking a device means that the locking entity can do I/O on that
  device (no check is performed during I/O operations).
  
  int bdev_trylock
    return 1 on success, 0 on failure and, -1 on error
    
  __dev_t dev
    device to lock
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_trylock(__dev_t dev)
{
  int res;

  magic_assert(bdev[major(dev)].magic,BLOCK_DEVICE_MAGIC,
	       "bdev: device(%04x:%04x) overwritten",
	       major(dev),minor(dev));

  if (!bdev[major(dev)].bd_flag_used) {
    printk(BDEVLOG "bdev: device(%04x:%04x) not registered",
	   major(dev),minor(dev));	  
    return -1;
  }
  
  __b_mutex_lock(&mutex);
  res=bdev[major(dev)].bd_op->_trylock(dev);
  __b_mutex_unlock(&mutex);
				   
  return res;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Try to unlock a device.
  
  int bdev_tryunlock
    return 1 on success, 0 on failure and, -1 on error
  
  __dev_t dev
    device to unlock
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_tryunlock(__dev_t dev)
{
  int res;

  magic_assert(bdev[major(dev)].magic,BLOCK_DEVICE_MAGIC,
	       "bdev: device(%04x:%04x) overwritten",
	       major(dev),minor(dev));

  if (!bdev[major(dev)].bd_flag_used) {
    printk(BDEVLOG "bdev: device(%04x:%04x) not registered",
	   major(dev),minor(dev));	  
    return -1;
  }
  
  __b_mutex_lock(&mutex);
  res=bdev[major(dev)].bd_op->_tryunlock(dev);
  __b_mutex_unlock(&mutex);
				   
  return res;
}

	     
/*++++++++++++++++++++++++++++++++++++++
  
  The following function are used to "virtualize" the call of a function
  of a block device sub-system. 
  Actually they are implemeted by function to perform some parameters
  check but they will be implemented by macro.
  +latex+ \\

  This function request to read a block number.

  int bdev_read
    return 0 on success, other value on error

  __dev_t dev
    device where to read from

  __blkcnt_t blocknum
    block number to read

  __uint8_t *buffer
    buffer for the readed data
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_read(__dev_t dev, __blkcnt_t blocknum, __uint8_t *buffer)
{

  magic_assert(bdev[major(dev)].magic,BLOCK_DEVICE_MAGIC,
	       "bdev: device(%04x:%04x) overwritten",
	       major(dev),minor(dev));

  if (!bdev[major(dev)].bd_flag_used) {
    printk(BDEVLOG "bdev: device(%04x:%04x) not registered",
	   major(dev),minor(dev));	  
    return -1;
  }
   
  if (blocknum<0) {
    printk(BDEVLOG "bdev: device(%04x:%04x) read request out of range",
	   major(dev),minor(dev));	  
    return -1;
  }

  return bdev[major(dev)].bd_op->read(dev,blocknum,buffer);
}

/*++++++++++++++++++++++++++++++++++++++
  
  This function request to move the head to a specified block of a device.

  int bdev_seek
    return 0 on success, other value on error

  __dev_t dev
    device to seek

  __blkcnt_t blocknum
    block number to seek into
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_seek(__dev_t dev, __blkcnt_t blocknum)
{
  magic_assert(bdev[major(dev)].magic,BLOCK_DEVICE_MAGIC,
	       "bdev: device(%04x:%04x) overwritten",
	       major(dev),minor(dev));

  if (!bdev[major(dev)].bd_flag_used) {
    printk(BDEVLOG "bdev: device(%04x:%04x) not registered",
	   major(dev),minor(dev));	  
    return -1;
  }
   
  if (blocknum<0) {
    printk(BDEVLOG "bdev: device(%04x:%04x) seek request out of range",
	   major(dev),minor(dev));	  
    return -1;
  }
    
  return bdev[major(dev)].bd_op->seek(dev,blocknum);
}

/*++++++++++++++++++++++++++++++++++++++
  
  Write a block into a device.

  int bdev_write
    return 0 on success, other value on error

  __dev_t dev
    device to write into

  __blkcnt_t blocknum
    block number to write

  __uint8_t *buffer
    buffer with data to write
  ++++++++++++++++++++++++++++++++++++++*/

int bdev_write(__dev_t dev, __blkcnt_t blocknum, __uint8_t *buffer)
{
  magic_assert(bdev[major(dev)].magic,BLOCK_DEVICE_MAGIC,	       
	       "bdev: device(%04x:%04x) overwritten",
	       major(dev),minor(dev));

  if (!bdev[major(dev)].bd_flag_used) {
    printk(BDEVLOG "bdev: device(%04x:%04x) not registered",
	   major(dev),minor(dev));	  
    return -1;
  }

  if (blocknum<0) {
    printk(BDEVLOG "bdev: device(%04x:%04x) write request out of range",
	   major(dev),minor(dev));	  
    return -1;
  }

  return bdev[major(dev)].bd_op->write(dev,blocknum,buffer);
}
