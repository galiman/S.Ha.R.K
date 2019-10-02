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

  CVS :        $Id: ide.c,v 1.1.1.1 2002/03/29 14:12:49 pj Exp $
 
  Revision:    $Revision: 1.1.1.1 $

  Last update: $Date: 2002/03/29 14:12:49 $

  This module contains IDE initialization, IDE glue between
  the "block device" module and the IDE low level module and some
  usefull function used by the low level module.

***************************************/

/*
 * Copyright (C) 19992,2000 Massimiliano Giorgi
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

#include <fs/sysmacro.h>
#include <fs/major.h>
#include <fs/magic.h>
#include <fs/bdevinit.h>
#include <fs/assert.h>
#include <fs/maccess.h>
#include <fs/assert.h>

#include "sysind.h"
#include "bdev.h"
#include "phdsk.h"
#include "lodsk.h"
#include "ide.h"
#include "idereq.h"

/*
 * FLAGS
 */

/*+ if defined: scan for device on interface also if soft reset fail +*/
#define FORCE_SCANNING 1
//#undef FORCE_SCANNING

/*+ if defined: use PIO (polled) I/O on all interfaces, disable DMA +*/
#define FORCE_PIOMODE 1
//#undef FORCE_PIOMODE

/*+ if 1: search only master device (no slave) +*/
/*+ if 0: search master and slave device +*/
#define ONLY_MASTER 1

/*
 * DEBUG
 */

/*+ if defined: trace initialization ++*/
#define TRACE_INIT KERN_DEBUG
#undef TRACE_INIT

/*+ if defined: trace ide_scan() [to find devices] +*/
#define TRACE_IDESCAN KERN_DEBUG
#undef TRACE_IDESCAN

/*+ if defined: trace ide_scandisk() [to find logical layout] +*/
#define TRACE_IDESCANDEV KERN_DEBUG
#undef TRACE_IDESCANDEV

/**/

/*+ these are debug macros... +*/

#ifdef TRACE_INIT
#define printk0(fmt,args...) \
        printk(TRACE_INIT "ide_init: " fmt,##args)
#else
#define printk0(fmt,args...)
#endif

#ifdef TRACE_IDESCAN
#define printk1(fmt,args...) \
        printk(TRACE_IDESCAN "ide_scan: " fmt,##args)
#else
#define printk1(fmt,args...)
#endif

#ifdef TRACE_IDESCANDEV
#define printk2(fmt,args...) \
        printk(TRACE_IDESCANDEV "ide_scandisk: " fmt,##args)
#else
#define printk2(fmt,args...)
#endif

/*
 *
 *
 */

/*+ Name used by this block device +*/
static char device_name[]="ide";

// /dev/ide/hd-i0mp1

/*+ Mantains informatio about a single interface +*/
/* warning: must not cross 4KBytes boundaries (required for DMA operations) */
struct ideinfo ide[MAXIDEINTERFACES]={{0}};
//struct ideinfo ide[MAXIDEINTERFACES] __attribute__((nocommon));
//struct ideinfo ide[MAXIDEINTERFACES] __attribute__((section("ide_section")))=
//{0};

/*+ Mantains information about every physical device +*/
static struct ideminorinfo mtable[MAXIDEMINOR];

/*
 *
 *
 *
 */

//#ifndef NDEBUG
void ide_dump_status(void)
{
  int i;

  printk(KERN_INFO "ide block device status:");
  for (i=0;i<MAXIDEINTERFACES;i++)
    if (!is_ide_free(i)) {
      printk(KERN_INFO "  ide%i:",i);
      printk(KERN_INFO "    errors: %i",ide[i].errors);
    }
}
//#endif

/* 
 * how parameters are mapped to device minor number:
 *
 *   7   6   5   4   3   2   1   0
 * ---------------------------------
 * I   .   .   I   I   .   .   .   I
 * ---------------------------------
 *       ^       ^         ^
 *       I       I         I-- logical disk (aka partition) 
 *       I       I------------ drive id     (aka master,slave)
 *       I-------------------- interface
 */

/*++++++++++++++++++++++++++++++++++++++
  
  Convert an interface id, driver id, logical disk to a device id.

  __dev_t idemakedev
    return a device id

  int ideif
    ide interface sequential number

  int drv
    ide channel (IDE_MASTER, IDE_SLAVE)

  int lodsk
    logical disk (aka partition) on than device
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ __dev_t idemakedev(int ideif,int drv,int lodsk)
{
  return makedev(MAJOR_B_IDE,((ideif&0x7)<<5)|((drv&0x1)<<4)|(lodsk&0xf));
}

/*++++++++++++++++++++++++++++++++++++++
  
  Extract an interface id from a device id.

  int idehwif
    return an interface id

  __dev_t dev
    device id to parse
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ int idehwif(__dev_t dev)
{
  return minor(dev)>>5;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Extract a drive id from a device id.

  int idedriveid
    return a device id

  __dev_t dev
    device id to parse
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ int idedriveid(__dev_t dev)
{
  return (minor(dev)&0x10)?1:0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Extract a logical disk id from a device id.

  int idelodsk
    return a logical disk id

  __dev_t dev
    device id to parse
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ int idelodsk(__dev_t dev)
{
  return minor(dev)&0xf;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Extract the minor part of a device number.

  int idemindex
    return a device minor number

  __dev_t dev
    device id to parse
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ int idemindex(__dev_t dev)
{
  return minor(dev);
}


/*++++++++++++++++++++++++++++++++++++++
  
  Parse a device minor number from a device id and a logical disk id.

  int idemindexext
    return a device minor number

  __dev_t dev
    device id to parse

  int lodsk
    logical disk id to parse
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ int idemindexext(__dev_t dev, int lodsk)
{
  return minor(dev)+(lodsk&0xf);
}

/* --- */

//#ifndef NDEBUG
int ide_dump_startsize(__dev_t dev, __blkcnt_t *start, __blkcnt_t *size)
{
  int i;
  if (major(dev)!=MAJOR_B_IDE) return -1;
  i=idemindex(dev);
  if (mtable[i].used) {
    *start=mtable[i].start;
    *size=mtable[i].size;
    return 0;
  }
  return -1;
}
//#endif

/*
 *
 * to manage struct ata_diskid
 *
 */

/*++++++++++++++++++++++++++++++++++++++
  
  Dump, using printk(), the "important" information returned by an ide
  device in response to an ATA IDENTIFY request.

  struct ata_diskid *ptr
    pointer to the structure returned by an IDENTIFY command
    
  struct ide_diskinfo *info
    pointer to the disk information structure (build decoding an
    ata_diskid structure)
  ++++++++++++++++++++++++++++++++++++++*/

static void ide_dump_diskid(struct ata_diskid *ptr,
			    struct ide_diskinfo *info)
{
  char atabuf[256];
  int i,j;
  
  if (is_ATA4(ptr)||is_ATA3(ptr)||is_ATA2(ptr)||is_ATA(ptr)) {
    i=0;
    if (is_ATA(ptr)) {
      atabuf[i++]='1';
      atabuf[i++]=' ';
    }
    if (is_ATA2(ptr)) {
      atabuf[i++]='2';
      atabuf[i++]=' ';
    }
    if (is_ATA3(ptr)) {
      atabuf[i++]='3';
      atabuf[i++]=' ';
    }
    if (is_ATA4(ptr)) {
      atabuf[i++]='4';
      atabuf[i++]=' ';
    }
    atabuf[i++]='\0';
  } else 
    /* actualy only ATA disk are recognized */
    sprintf(atabuf,"?");

  printk(IDELOG "    device         : hard disk");
  printk(IDELOG "    model          : %s",ptr->model);
  printk(IDELOG "    serial         : %s",ptr->serial);
  printk(IDELOG "    firmware rev   : %s",ptr->firmware);
  printk(IDELOG "    default C/H/S  : %i/%i/%i",
	 ptr->def_cyls,ptr->def_heads,ptr->def_sects);
  printk(IDELOG "    total capacity : ~%4.3f GBytes",
	 (double)ptr->lba_capacity/(1024.0*1024.0*2.0));
  printk(IDELOG "    ATA            : %s",atabuf);
  printk(IDELOG "    LBA            : %s",is_LBAcapable(ptr)?"yes":"no");

  i=0;
  if (info->max_pio_mode<0) strcpy(atabuf,"none");
  else {
    for (j=info->max_pio_mode;j>=0;j--) {
      strcpy(atabuf+i,"pio"); i+=3;
      atabuf[i++]=j+'0';
      atabuf[i++]=' ';
    }
    atabuf[i++]='\0';
  }
  printk(IDELOG "    PIO modes      : %s",atabuf);

  i=0;
  if (info->max_dma_mode<0) strcpy(atabuf,"none");
  else {
    for (j=info->max_dma_mode;j>=0;j--) {
      strcpy(atabuf+i,"mword"); i+=5;
      atabuf[i++]=j+'0';
      atabuf[i++]=' ';
    }
    atabuf[i++]='\0';
  }  
  printk(IDELOG "    DMA modes      : %s",atabuf);
  
  i=0;
  if (info->max_udma_mode<0) strcpy(atabuf,"none");
  else {
    for (j=info->max_udma_mode;j>=0;j--) {
      strcpy(atabuf+i,"udma"); i+=4;
      atabuf[i++]=j+'0';
      atabuf[i++]=' ';
    }
    atabuf[i++]='\0';
  }
  printk(IDELOG "    UDMA modes     : %s",atabuf);    
}

/*++++++++++++++++++++++++++++++++++++++
  
  Dump, using printk(), the "important" information returned by an ide
  device in response to an ATAPI PACKET IDENTIFY request.

  struct atapi_diskid *ptr
    pointer to the structure returned by a PACKET IDENTIFY command
  ++++++++++++++++++++++++++++++++++++++*/

static void ide_dump_atapidiskid(struct atapi_diskid *ptr)
{
  char atabuf[16];
  //__uint8_t *p;
  char *dev;
  int i=0;
  
  if (is_ATAPIdevice(ptr)) {
    strcpy(atabuf,"ATAPI ");
    i=6;
  }
  
  if (is_ATA4(ptr)||is_ATA3(ptr)||is_ATA2(ptr)||is_ATA(ptr)) {
    if (is_ATA(ptr)) {
      atabuf[i++]='1';
      atabuf[i++]=' ';
    }
    if (is_ATA2(ptr)) {
      atabuf[i++]='2';
      atabuf[i++]=' ';
    }
    if (is_ATA3(ptr)) {
      atabuf[i++]='3';
      atabuf[i++]=' ';
    }
    if (is_ATA4(ptr)) {
      atabuf[i++]='4';
      atabuf[i++]=' ';
    }
    atabuf[i++]='\0';
  } else {
    /* actualy only ATA disk are recognized */
    atabuf[i++]='?';
    atabuf[i++]='\0';
  }

  if (is_directdev(ptr)) dev="direct";
  else if (is_sequentialdev(ptr)) dev="sequential";
  else if (is_printerdev(ptr)) dev="printer";
  else if (is_processordev(ptr)) dev="processor";
  else if (is_writeoncedev(ptr)) dev="write once";
  else if (is_cdromdev(ptr)) dev="cd-rom";
  else if (is_scannerdev(ptr)) dev="scanner";
  else if (is_opticalmemorydev(ptr)) dev="optical memory";
  else if (is_mediachengerdev(ptr)) dev="media changer";
  else if (is_communicatordev(ptr)) dev="communicator";
  else if (is_arraydev(ptr)) dev="array";
  else dev="?";
  
  printk(IDELOG "    device         : %s",dev);
  printk(IDELOG "    model          : %s",ptr->model);
  printk(IDELOG "    serial         : %s",ptr->serial);
  printk(IDELOG "    firmware rev   : %s",ptr->firmware);
  printk(IDELOG "    ATA            : %s",atabuf);  
}
 
/*
 *
 * Interface the "block device" module to the low-level routine
 *
 */

/*
void dump_buffer(__uint8_t *buf)
{
  int i;
  for (i=0;i<512;i++) {
    cprintf("%02x ",(int)*(buf+i));
    if (i%16==15) cprintf("\n");
  }
}
*/


/*++++++++++++++++++++++++++++++++++++++
  
  Try to lock a device.
  (A device can be locked if the device is a partition and the whole
  disk is not locked or if the device is the whole disk and any partititions
  have been locked).
  
  int ide_trylock
    return ?_ERR on error, ?_OK on success and, ?_CANT on failure
    
  __devt_t device
    device to use
  ++++++++++++++++++++++++++++++++++++++*/

static int ide_trylock(__dev_t device)
{
  int i,ind,ind2;
  
  ind=idemindex(device);
  assertk(ind>=0&&ind<MAXIDEMINOR);
  
  if (!mtable[ind].used) return TRYLOCK_ERR;

  if (mtable[ind].blocked) return TRYLOCK_CANT;
  
  if (idelodsk(device)==0) {
    for (i=1;;i++) {
      ind2=idemindex(idemakedev(idehwif(device),idedriveid(device),i));
      assertk(ind2>=0&&ind2<MAXIDEMINOR);
      if (ind2==ind) {
	mtable[ind].blocked=1;
	return TRYLOCK_OK;
      }
      if (!mtable[ind2].used) continue;
      if (mtable[ind2].blocked) return TRYLOCK_CANT;
    }
  }

  ind2=idemindex(idemakedev(idehwif(device),idedriveid(device),0));
  assertk(ind2>=0&&ind2<MAXIDEMINOR);
  assertk(mtable[ind2].used==1);

  if (mtable[ind2].blocked) return TRYLOCK_CANT;

  mtable[ind].blocked=1;
  return TRYLOCK_OK;
}

static int ide_tryunlock(__dev_t device)
{
  int ind;
  
  ind=idemindex(device);
  assertk(ind>=0&&ind<MAXIDEMINOR);
  
  if (!mtable[ind].used) return TRYUNLOCK_ERR;
  if (!mtable[ind].blocked) return TRYUNLOCK_ERR;
  mtable[ind].blocked=0;
  return TRYUNLOCK_OK;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Read a block of data (sector) from a device into a buffer.

  int bd_ide_read
    return 0 on success, other values on error

  __dev_t device
    device id to read from

  __blkcnt_t blocknum
    block number to read

  __uint8_t *buffer
    buffer for the readed data
  ++++++++++++++++++++++++++++++++++++++*/

static int bd_ide_read(__dev_t device, __blkcnt_t blocknum, __uint8_t *buffer)
{
  register int ideif=idehwif(device);
  register int index=idemindex(device);
  int res;
  
  magic_assert(ide[ideif].magic,IDE_MAGIC,
	       "ide%i: interface overwritten",ideif);
  magic_assert(mtable[index].magic,IDE_MAGIC_MINOR,
	       "ide%i: minor %i overwritten",ideif,minor(device));
  
  if (major(device)!=MAJOR_B_IDE) {
    printk(IDEERRLOG "ide%i: major %i is not EIDE",ideif,major(device));
    return -1;
  }
  
  if (minor(device)>MAXIDEMINOR) {
    printk(IDEERRLOG "ide%i: minor %i out of range",ideif,minor(device));
    return -1;
  }
  
  if (!mtable[index].used) {
    printk(IDEERRLOG "ide%i: minor %i not present",ideif,minor(device));
    return -1;
  }

  //cprintf("index=%i\n",index);
  //cprintf("blocknum=%li\n",(long)blocknum);
  //cprintf("size=%li\n",(long)mtable[index].size);
  //cprintf("start=%li\n",(long)mtable[index].start);  
  //cprintf("sizeof(size)=%i\n",sizeof(mtable[index].size));
 
  /*
  if (blocknum<0||blocknum>=mtable[index].size) {
    printk(IDEERRLOG "ide%i: request block out of range",ideif);
    return -1;
  }
  */
  
  res=ide_read(ideif,idedriveid(device),
		  mtable[index].start+blocknum,
		  buffer);

  //if (blocknum==0x9ea19)
  //  dump_buffer(buffer);
  
  //cprintf("index=%i\n",index);
  //cprintf("blocknum=%li\n",(long)blocknum);
  //cprintf("size=%li\n",(long)mtable[index].size);
  //cprintf("sizeof(size)=%i\n\n",sizeof(mtable[index].size));

  return res;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Move the head on a specified cylinder.

  int bd_ide_seek
    return 0 on success, other values on error

  __dev_t device
    device id to read from

  __blkcnt_t blocknum
    block number to moving into
  ++++++++++++++++++++++++++++++++++++++*/

static int bd_ide_seek(__dev_t device, __blkcnt_t blocknum)
{
  register int ideif=idehwif(device);
  register int index=idemindex(device);
  int res;
  
  magic_assert(ide[ideif].magic,IDE_MAGIC,
	       "ide%i: interface overwritten",ideif);
  magic_assert(mtable[index].magic,IDE_MAGIC_MINOR,
	       "ide%i: minor %i overwritten",ideif,minor(device));

  if (major(device)!=MAJOR_B_IDE) {
    printk(IDEERRLOG "ide%i: major %i is not EIDE",ideif,major(device));
    return -1;
  }
  
  if (minor(device)>MAXIDEMINOR) {
    printk(IDEERRLOG "ide%i: minor %i out of range",ideif,minor(device));
    return -1;
  }
  
  if (!mtable[index].used) {
    printk(IDEERRLOG "ide%i: minor %i not present",ideif,minor(device));
    return -1;
  }

  res=ide_seek(ideif,idedriveid(device),
		  mtable[index].start+blocknum);

  return res;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Write a block of data (sector) from a buffer into a device (not yet
  implemented, return always error).

  int bd_ide_write
    return 0 on success, other value on errors

  __dev_t device
    device id to write into

  __blkcnt_t blocknum
    block number to write

  __uint8_t *buffer
    buffer to write into
  ++++++++++++++++++++++++++++++++++++++*/

static int bd_ide_write(__dev_t device, __blkcnt_t blocknum, __uint8_t *buffer)
{
  register int ideif=idehwif(device);
  register int index=idemindex(device);
  int res;
  
  magic_assert(ide[ideif].magic,IDE_MAGIC,
	       "ide%i: interface overwritten",ideif);
  magic_assert(mtable[index].magic,IDE_MAGIC_MINOR,
	       "ide%i: minor %i overwritten",ideif,minor(device));

  if (major(device)!=MAJOR_B_IDE) {
    printk(IDEERRLOG "ide%i: major %i is not EIDE",ideif,major(device));
    return -1;
  }
  
  if (minor(device)>MAXIDEMINOR) {
    printk(IDEERRLOG "ide%i: minor %i out of range",ideif,minor(device));
    return -1;
  }
  
  if (!mtable[index].used) {
    printk(IDEERRLOG "ide%i: minor %i not present",ideif,minor(device));
    return -1;
  }

  res=ide_write(ideif,idedriveid(device),
		  mtable[index].start+blocknum,
		  buffer);

  return res;
}

/*
 *
 * Initialization
 *
 */

/*+ Data used to register the IDE module to the block device manager +*/
static struct block_device_operations ide_operations={
  ide_trylock,
  ide_tryunlock,
  bd_ide_read,
  bd_ide_seek,
  bd_ide_write
};

/* this information are saved from the initialization routine for
 * futher reference
 */

/*+ initialization parameter (for the "glue" initialization) +*/
void *ide_parm_initserver=NULL;

/*+ show information during startup? +*/
int ide_showinfo_flag=0;

/*++++++++++++++++++++++++++++++++++++++
  
  This function try to register an IDE interface, scan for device and
  identify the logical struct of the device

  int ide_tryregister 
    return 0 on successe <0 on error

  int port0 
    first port of the ide interface (ide port)

  int port1
    second port of the ide interface (control port)

  int irq
    irq of the ide interface
    
  int dma
    dma 16bits channel (if using not bus master mode)

  int bmdma
    port for the bus master dma operations
  ++++++++++++++++++++++++++++++++++++++*/

static int ide_tryregister(int port0, int port1, int irq, int dma, int bmdma)
{
  int ideif,res,j;
  
  ideif=ide_register(port0,port1,irq,dma,bmdma);
  printk0("registering IDE at 0x%04x,0x%04x,%i,%i,0x%04x... ",
	  port0,port1,irq,dma,bmdma);
  if (ideif>=0) {
    res=ide_scan(ideif);
    printk0("found %i peripheral(s)",res);
    if (res>0) {
      for (j=IDE_MASTER;j<=IDE_SLAVE;j++) {
	if (ide[ideif].pdisk[j]!=NULL) {
	  printk0("scanning %s for logical layout",
		  j==IDE_MASTER?"master":"slave");
	  ide_scandisk(ideif,j);
	}
      }
      return 0;
    }
    return -1;
  } else
    printk0("registering failed");
  return -2;
}

/*++++++++++++++++++++++++++++++++++++++

  This function parse a string to find "command line options" for the
  IDE subsystem; see source for comments.

  int ide_scanparms
    return 0 on success, other values on error

  char *str
    string to parse
  ++++++++++++++++++++++++++++++++++++++*/

static int ide_scanparms(char *str)
{
  int port0,port1,irq,dma=-1,bmdma=-1;
  char *ptr;
  int ret;

  /* available string options:

     ide=port,sec_port,irq 
     example 
     "ide=0x1f0,0x3f0,14"

     try to register this ide interface
   */

  printk0("(start) searching for parameters");  
  if (str==NULL) {
    printk0("(end) searching for parameters");  
    return 0;
  }
  
  ptr=str;
  for (;;) {
    ptr=strscn(ptr,"ide=");
    if (ptr==NULL) return 0;
    ret=sscanf(ptr,"ide=%x,%x,%i,%i,%x",&port0,&port1,&irq,&dma,&bmdma);
    if (ret==3||ret==5) {
      {
	int len;
	char *s;
	s=strchr(ptr,' ');
	if (s!=NULL) len=s-ptr+1;
	else len=strlen(ptr);
	memset(ptr,len,' ');
      }
      ide_tryregister(port0,port1,irq,dma,bmdma);
    }
    ptr++;
  }
  printk0("(end) searching for parameters");  
  return 0;
}

/*++++++++++++++++++++++++++++++++++++++

  This function initialize the IDE subsystem; must be called before every
  other function.

  It initialize internal data structure, register itself to the bdev
  (block dev) manager and try to initialize "standard" IDE interface;
  non-standard ide interface can be initialize using the struct IDE_PARMS
  (see include/fs/fsinit.h).

  int ide_init
    return 0 on success, other on failure

  BDEV_PARMS *parms
    pointer to a structure contains general and specific initialization
    data
  ++++++++++++++++++++++++++++++++++++++*/

int ide_init(BDEV_PARMS *parms)
{
  struct block_device bd;
  __uint32_t addr;
  int res;
  int i;

  printk0("START");
  
  init_idereq();
  printk0("init_idereq()... done");
  
  for (i=0;i<MAXIDEINTERFACES;i++) {
    
    magic_set(ide[i].magic,IDE_MAGIC);
    
    ide[i].server=NIL;
    ide[i].io_port=0;
    ide[i].io_port2=0;
    ide[i].irq=0;
    ide[i].pdisk[IDE_MASTER]=NULL;
    ide[i].info[IDE_MASTER].use_lba=0;
    ide[i].pdisk[IDE_SLAVE]=NULL;
    ide[i].info[IDE_SLAVE].use_lba=0;    
    //ide[i].reqhead=NIL;
    //ide[i].reqtail=NIL;
    ide[i].errors=0;

    /* some check on the ide[].prd struct */    
    addr=__lin_to_phy(ide[i].prd);

    /* must be 32bits aligned! */
    assertk((addr&3)==0); /* this is granted by aligned(4) of the struct */

    /* must not cross 4KBytes boundaries! */
    if ((addr&0xfffff000)!=((addr+sizeof(ide[i].prd)-1)&0xfffff000)) {
      printk(KERN_EMERG "ide[%i].prd cross 4Kbytes boundary!",i);
      printk(KERN_EMERG "ide[] table is located at 0x%08lx",(long)&ide);
      printk(KERN_EMERG "ide[%i] is located at 0x%08lx",
	     i,(long unsigned)(&ide[i]));
      printk(KERN_EMERG "ide[%i].prd is located at 0x%08lx",
	     i,(long unsigned)(&ide[i].prd));
      printk(KERN_EMERG "ide[%i].prd is located at 0x%08lx (phy) for 0x%04x",
	     i,(long unsigned)addr,(int)sizeof(ide[i].prd));
      
      /* now! */
      assertk(0==-1);
    }
    
  }

  for (i=0;i<MAXIDEMINOR;i++) {
    
    magic_set(mtable[i].magic,IDE_MAGIC_MINOR);

    mtable[i].start=0;
    mtable[i].size=0;
    mtable[i].used=0;
    mtable[i].blocked=0;
  }

  ide_parm_initserver=IDEPARMS(parms).parm_initserver;
  ide_showinfo_flag=parms->showinfo;
  bd.bd_sectorsize=IDE_SECTOR_SIZE;
  bd.bd_op=&ide_operations;

  printk0("static data initialization... done");
  
  res=bdev_register(makedev(MAJOR_B_IDE,0),device_name,&bd);
  if (res) return -1;

  printk0("registering IDE to block-dev... done");

  /* Well... bus master dma operations i/o ports would be reported
   * by the pci device management module
   */
  ide_tryregister(0x1f0,0x3f0,14,3,0xe000);
  ide_tryregister(0x170,0x370,15,-1,0xe008);

  ide_scanparms(parms->config);

  printk0("END");
  return 0;    
}

/*
 *
 *
 */

/*++++++++++++++++++++++++++++++++++++++
  
  Scan for device on an IDE interface: reset the device, issue a
  IDENTIFY command and optional a PACKET IDENTIFY; if a hard disk
  is found it is registered to the "physical disk" module.

  int ide_scan
    return the number of device found on the interface (0,1 or 2)

  int ideif
    the interface to scan (every interface has a progess number)
  ++++++++++++++++++++++++++++++++++++++*/

int ide_scan(int ideif)
{
  struct ata_diskid   info;
  struct atapi_diskid info2;
  struct phdskinfo    disk;
  struct phdskinfo    *phdskptr;
  int                 drv;
  int                 res;
  //int                 ind;
  int                 found;
  int                 atapi;
  int                 fl;
  
  printk1("START");
  
  magic_assert(ide[ideif].magic,IDE_MAGIC,
	       "ide: interface(%i) overwritten",ideif);

  if (ide[ideif].pdisk[IDE_MASTER]!=NULL||
      ide[ideif].pdisk[IDE_SLAVE]!=NULL)
    return 0;

  /* phase 0 */
  /* softreset */

  printk1("making a soft reset...");
  res=do_ide_softreset(ideif);
  if (res) {
    printk1("soft reset fail");
#ifndef FORCE_SCANNING
    printk1("END");
    return 0;
#endif
    printk1("FORCE_SCANNING");
  } else
    printk1("soft reset... done");
  
  /* phase 1 */
  /* searching for disk drive */

  found=0;
  for (drv=IDE_MASTER;drv<=(ONLY_MASTER?IDE_MASTER:IDE_SLAVE);drv++) {

    printk1("scanning for %s",drv==IDE_MASTER?"master":"slave");
    
    atapi=0;
    res=ide_identify(ideif,drv,&info);
    
    printk1("identify... done (device %sfound)",res==IDE_OK?"":"NOT ");
      
    if (res!=IDE_OK) {
      atapi=1;
      res=ide_pidentify(ideif,drv,&info2);
      printk1("pidentify... done (device %sfound)",res==IDE_OK?"":"NOT ");
    }
    
    if (res==IDE_OK) {
	
      if (ide_showinfo_flag&&!found) {
	printk(IDELOG "ide%i: 0x%3x,%i,%i,0x%04x",ideif,
	       ide[ideif].io_port,ide[ideif].irq,
	       (ide[ideif].dma==255)?-1:ide[ideif].dma,
	       ide[ideif].io_bmdma);
      }
      found++;
	      
      /* if this is an ATAPI device... */      
      if (atapi) {
	if (ide_showinfo_flag) {
	  printk(IDELOG "ide%i: %s device",
		 ideif,drv==IDE_MASTER?"master":"slave");
	  ide_dump_atapidiskid(&info2);
	}
	/* ATAPI devices not managed yet! */
	continue;
      }

      disk.pd_device=idemakedev(ideif,drv,0);

      disk.pd_sectsize=IDE_SECTOR_SIZE;      
      disk.pd_size=(__uint32_t)info.act_cyls*info.act_heads*info.act_sects;

      disk.pd_logeom.cyls=disk.pd_phgeom.cyls=info.act_cyls;
      disk.pd_logeom.heads=disk.pd_phgeom.heads=info.act_heads;
      disk.pd_logeom.sectors=disk.pd_phgeom.sectors=info.act_sects;

      if (is_LBAcapable(&info)) {
	ide[ideif].info[drv].use_lba=TRUE;
	disk.pd_ide_check_geom=0;
      } else {
	ide[ideif].info[drv].use_lba=FALSE;
	disk.pd_ide_check_geom=0;
      }

      /* for PIO capabilities */
      {
	__int8_t support;
	if (info.fields_valid&2) {
	  if (info.eide_PIO_modes&2) support=4;
	  else if (info.eide_PIO_modes&1) support=3;
	  else support=info.PIO_mode;	
	} else
	  support=info.PIO_mode;	
	ide[ideif].info[drv].max_pio_mode=support;
      }
      /* for DMA capabilities */
      {
	__int8_t support;
	if (info.DMA_mword&4) support=2;
	else if (info.DMA_mword&2) support=1;
	else if (info.DMA_mword&1) support=0;
	else support=-1;
	ide[ideif].info[drv].max_dma_mode=support;
      }
      /* for ULTRA DMA capabilities */
      {
	__int8_t support;
	if (!(info.fields_valid&4)) support=-1;
	else if (info.UDMA_cap&4) support=2;
	else if (info.UDMA_cap&2) support=1;
	else if (info.UDMA_cap&1) support=0;
	else support=-1;
	ide[ideif].info[drv].max_udma_mode=support;
      }
      
      /* Well... BM-DMA used by default (if possible)*/
      ide[ideif].info[drv].use_dma=0;
      fl=0;
      if (ide[ideif].info[drv].max_dma_mode!=-1) fl=1;
      if (ide[ideif].info[drv].max_udma_mode!=-1) fl=1;
#ifdef FORCE_PIOMODE
      fl=0; // so use PIO mode
#endif
      ide[ideif].info[drv].use_bm_dma=fl;
      
      /* Show informations... if request */
      if (ide_showinfo_flag) {
	printk(IDELOG "ide%i: %s device",
	       ideif,drv==IDE_MASTER?"master":"slave");	
	ide_dump_diskid(&info,&ide[ideif].info[drv]);
      }

      sprintf(disk.pd_name,"hd%c",'a'+ideif*2+drv);

      /* Register physical disk information */
      phdskptr=phdsk_register(&disk);
      if (phdskptr==NULL) continue;
      ide[ideif].pdisk[drv]=phdskptr;
      
      /* Well, the queueing algorithm must know the physical disk layout */
      ide[ideif].queue[drv].disk=phdskptr;

      /* Activate look a head... (if possible) */
      res=ide_enablelookahead(ideif,drv);
      if (ide_showinfo_flag)
	printk(IDELOG "ide%i: look ahead %s for %s device",
	       ideif,
	       res==IDE_OK?"activated":"not activated",
	       drv==IDE_MASTER?"master":"slave"
	       );      
    }      
  }

  if (found==0) {
    printk(IDELOG "ide: no device found on 0x%x,%i, unregistering interface",
	   ide[ideif].io_port,ide[ideif].irq);

    ide_unregister(ideif);

  } 
  
  return found;
}
			 
/* --- */

/*++++++++++++++++++++++++++++++++++++++
  
  This function is called by the "logical disk" module to inform this module
  that a logical disk partition is found [see ide_scandisk()].

  int _callback
    must return 0 to continue searching for new partition, -1 to stop
    scanning

  int ind
    partition index (for linux users: it is the number after the device
    name, ex. hda1, hdb5 )

  struct lodskinfo *ptr
    information on partition found

  void *data
    private data for this module: is a pointer to a __dev_t to identify
    the device where this partition reside
  ++++++++++++++++++++++++++++++++++++++*/

static int _callback(int ind, struct lodskinfo *ptr, void *data)
{
  char name[32];
  int index;
  __dev_t dev;
  
  printk2("found logical device %i",ind);
  
  assertk(ind>0);
  if (ind>MAXIDELODSK) {
    printk(IDELOG "ide: found more than %i partitions... skipped!",
	   MAXIDELODSK);
    return -1;
  }
  
  index=idemindexext(*(__dev_t *)data,ind);
  mtable[index].start=ptr->start;  
  mtable[index].size=ptr->size;
  mtable[index].used=1;

  
  dev=makedev(major(*(__dev_t *)data),index);
  sprintf(name,"hd%c%i",
	  idehwif(dev)*2+idedriveid(dev)+'a',
	  ind
	  );
  
  index=bdev_register_minor(dev,name,ptr->fs_ind);
  
  return 0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Scan an IDE device to find its logical structure; it use the service
  from the "logical disk" module to perform this.

  int ide_scandisk
    return 0 on success, other on error

  int hwif
    ide interface where there is the device

  int drv
    device to scan (IDE_MASTER, IDE_SLAVE)

  ++++++++++++++++++++++++++++++++++++++*/

int ide_scandisk(int hwif, int drv)
{
  char name[8];
  __dev_t device;
  int index;

  printk2("START");
    
  //cprintf("hwif=%i drv=%i\n",hwif,drv);
  device=idemakedev(hwif,drv,0);
  //cprintf("device=%i\n",(int)device);
  index=idemindex(device);
  //cprintf("minor=%i\n",(int)minor(device));
  //cprintf("---index=%i\n",index);
  
  mtable[index].start=0;
  mtable[index].size=ide[hwif].pdisk[drv]->pd_size;
  mtable[index].used=1;  
  sprintf(name,"hd%c",hwif*2+drv+'a');

  bdev_register_minor(device,name,FS_DEFAULT);
  
  lodsk_scan(device,
	     _callback,
	     (void*)&device,
	     ide_showinfo_flag,
	     name);

  printk2("END");
  return 0;
}


