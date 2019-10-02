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

  CVS :        $Id: lodsk.c,v 1.1.1.1 2002/03/29 14:12:49 pj Exp $
 
  Revision:    $Revision: 1.1.1.1 $

  Last update: $Date: 2002/03/29 14:12:49 $

  This module is used by block device sub-system to scan a physical
  hard disk to find logical layout (partition scheme).

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

#include "bdev.h"
#include "sysind.h"
#include "lodsk.h"

/*+ Log Level used to report an error +*/
#define LODSKERRLOG KERN_ERR

/*+ Log level used to inform +*/
#define LODSKLOG    KERN_NOTICE

/*+ An aligned buffer for data I/O +*/
static __uint8_t  buffer[512] __attribute__ ((aligned(4)));

/*++++++++++++++++++++++++++++++++++++++
  
  This function convert a physical partion structure to a partition
  structure (the difference is that the physical partition structure
  has a "strange" layout and must be parsed to find actually values that
  are saved into a partition structure).

  struct phpartition *ph
    physical partition to parse

  struct partition *lo
    values are stored into this structure
  ++++++++++++++++++++++++++++++++++++++*/

static void partph2lo(struct phpartition *ph,struct partition *lo)
{
  lo->boot_ind=((ph->boot_ind&0x80)?1:0);
  lo->sys_ind=ph->sys_ind;
  lo->rel_sect=ph->rel_sect;
  lo->nr_sects=ph->nr_sects;
  /**/
  lo->st_head=ph->st_h;
  lo->st_sect=ph->st_s&0x3f;
  lo->st_cyl=(__uint16_t)ph->st_c|(((__uint16_t)ph->st_s&0xc0)<<2);
  lo->en_head=ph->st_h;
  lo->en_sect=ph->en_s&0x3f;
  lo->en_cyl=(__uint16_t)ph->en_c|(((__uint16_t)ph->en_s&0xc0)<<2);
}

/*++++++++++++++++++++++++++++++++++++++
  
  Find if a partition is an extended partition.

  #define is_extended_partition
    return 0 if no, other value if yes

  sys_ind
    "system indicator" for that partition
  ++++++++++++++++++++++++++++++++++++++*/

#define is_extended_partition(sys_ind) (   \
  (sys_ind) == DOS_EXTENDED_PARTITION ||   \
  (sys_ind) == WIN98_EXTENDED_PARTITION || \
  (sys_ind) == LINUX_EXTENDED_PARTITION    \
)

/*++++++++++++++++++++++++++++++++++++++
  
  Scan recursiverly an extended partition to find other partition (this
  function suppose an IBM PC hard disk architecture). 
  When a new partition is found the callback function is called.

  int extended_partition
    return 0 on success, other value on error 

  char *lname
    it is the name of the physical disk; for informational purpose only
    (for example "hda").

  __dev_t device
    it's the device to scan

  lodsk_callback_func func
    it's the callback function to call when a new partition is found

  void *data
    this is opaque data passed to the callback function

  __blkcnt_t psect
    is the starting block (sector) number for this extended partition

  __blkcnt_t psize
    is the size in blocks of this extended partition

  int showinfo
    show information with printk()
  ++++++++++++++++++++++++++++++++++++++*/

/* this routine is from Linux 2.2.9 (modificated to run into S.Ha.R.K.) */

static int extended_partition(char *lname, __dev_t device,
			      lodsk_callback_func func, void *data,
			      __blkcnt_t psect, __blkcnt_t psize,
			      char *infobuffer, int nextid)
{
  char name[MAXLODSKNAME];
  struct phpartition *ppar;
  struct partition lpar;
  struct lodskinfo info;
  __blkcnt_t stpsect=psect;
  int ret;
  int i;
  int counter=5;
  int first=1;
  
  //if (phdsk_sectorsize(pdisk)!=512) return -1;
  
  for (;;) {

    //cprintf("reading block %li\n",(long)psect);
    
    ret=bdev_read(device,psect,buffer);
    if (ret) return -1;
    //cprintf("read block %li\n",(long)psect);
    if (*(__uint16_t*)(buffer+0x1fe)!=MSDOS_LABEL_MAGIC) return -1;
    //cprintf("ok sector\n");
    
    ppar=(struct phpartition*)(buffer+0x1be);

    /*
     * Usually, the first entry is the real data partition,
     * the 2nd entry is the next extended partition, or empty,
     * and the 3rd and 4th entries are unused.
     * However, DRDOS sometimes has the extended partition as
     * the first entry (when the data partition is empty),
     * and OS/2 seems to use all four entries.
     */

    /* 
     * First process the data partition(s)
     */
    
    for (i=0; i<4; i++, ppar++) {
      partph2lo(ppar,&lpar);

      /*
	cprintf("-- %i --\n",i+x);
	cprintf("c: %4i h: %4i s: %4i\n",lpar.st_cyl,lpar.st_head,lpar.st_sect);
	cprintf("c: %4i h: %4i s: %4i\n",lpar.en_cyl,lpar.en_head,lpar.en_sect);
	cprintf("sys: %02x relsec: %li  num sect: %li\n",
	lpar.sys_ind,lpar.rel_sect,lpar.nr_sects);
	*/
      
      if (!lpar.nr_sects || is_extended_partition(lpar.sys_ind))
	continue;

      /* Check the 3rd and 4th entries -
	 these sometimes contain random garbage */
      if (i >= 2
	  && lpar.rel_sect + lpar.nr_sects > psize
	  && (psect + lpar.rel_sect < psect ||
	      psect + lpar.rel_sect + lpar.nr_sects >
	      psect + psize))
	continue;
      
      sprintf(name,"%s%i ",lname,counter++);
      
      info.fs_ind=lpar.sys_ind;
      info.start=psect+lpar.rel_sect;
      info.size=lpar.nr_sects;
      ret=func(nextid++,&info,data);
      
      if (infobuffer!=NULL&&first) strcat(infobuffer,"< ");
      first=0;
      if (infobuffer) strcat(infobuffer,name);
    }
  
    ppar=(struct phpartition*)(buffer+0x1be);

    /*
     * Next, process the (first) extended partition, if present.
     * (So far, there seems to be no reason to make
     *  extended_partition()  recursive and allow a tree
     *  of extended partitions.)
     * It should be a link to the next logical partition.
     * Create a minor for this just long enough to get the next
     * partition table.  The minor will be reused for the next
     * data partition.
     */
    for (i=0; i<4; i++, ppar++)
      if(ppar->nr_sects && is_extended_partition(ppar->sys_ind))
	break;
    if (i == 4)
      break;	 /* nothing left to do, go to the end*/

    psect=stpsect+ppar->rel_sect;
    psize=ppar->nr_sects;
  }

  if (infobuffer!=NULL&&!first) strcat(infobuffer,"> ");
  return nextid;
}

//extern int ide_check_geometry(int pdisk, __uint8_t *buffer);

/*++++++++++++++++++++++++++++++++++++++
  
  This function is called to scan a physical hard disk to find its logical
  structure (IBM PC hard disk harchitecture is supposed).

  int msdos_partition
    return 0 on success, other value on error

  char *lname
    name to use if information is shown (for example "hdc")

  __dev_t device
    device to scan

  lodsk_callback_func func
    callback function to call when a partition is found

  void *data
    opaque data passed to the callback function

  int showinfo
    is true if informations will be displayed
  ++++++++++++++++++++++++++++++++++++++*/

/* this routine is from Linux 2.2.9 (modificated to run into S.Ha.R.K.) */

static int ibmpc_arch_scan(char *lname, __dev_t device,
			   lodsk_callback_func func,
			   void *data, char *infobuffer)
{
  struct lodskinfo info;
  char name[MAXLODSKNAME];
  struct phpartition *ppar;
  struct partition lpar;
  int i;
  int ret;
  int nextid;
  
  ret=bdev_read(device,0,buffer);
  if (ret) {
    cprintf("XXXXXXXXXXXXXXXX");
    return -1;
  }
  
  //if (phdsk_sectorsize(pdisk)!=512) return -1;

  if (*(__uint16_t*)(buffer+0x1fe)!=MSDOS_LABEL_MAGIC) return -1;
  
  ppar=(struct phpartition*)(buffer+0x1be);

  nextid=5;
  for (i=0;i<4;i++,ppar+=1) {
    partph2lo(ppar,&lpar);

    /*
    cprintf("-- %i --\n",i);
    cprintf("c: %4i h: %4i s: %4i\n",lpar.st_cyl,lpar.st_head,lpar.st_sect);
    cprintf("c: %4i h: %4i s: %4i\n",lpar.en_cyl,lpar.en_head,lpar.en_sect);
    cprintf("sys: %02x relsec: %li  num sect: %li\n",
	      lpar.sys_ind,lpar.rel_sect,lpar.nr_sects);
    */

    /*
    if (phdsk[pdisk].ide_check_geom&&i==0) {
      ide_check_geometry(pdisk,buffer);
    }
    */
    
    if (!lpar.nr_sects) continue;
    if (is_extended_partition(lpar.sys_ind)) {
      nextid=extended_partition(lname,device,func,data,
			 lpar.rel_sect,lpar.nr_sects,infobuffer,nextid);
      if (nextid<0) return -1;
    } else {
      sprintf(name,"%s%i ",lname,i+1);
      
      info.fs_ind=lpar.sys_ind;
      info.start=lpar.rel_sect;
      info.size=lpar.nr_sects;
      ret=func(i+1,&info,data);      
      
      if (infobuffer) strcat(infobuffer,name);

      if (ret) return 0;
    }    
  }
  
  return 0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  This function scan a physical hard disk for logical structure; actually
  only IBM PC hard disk architecture is recognized.

  int lodsk_scan
    return 0 on success, other value on error

  __dev_t device
    device to scan

  lodsk_callback_func func
    callback function to call when a new partition is found

  void *data
    opaque data passed to the callback function

  int showinfo
    if information must be displayed

  char *lname
    logical name, for information only (example "hda")
  ++++++++++++++++++++++++++++++++++++++*/

int lodsk_scan(__dev_t device, lodsk_callback_func func,
	       void *data, int showinfo, char *lname)
{
  char buffer[1024]; /* DANGER!!! */
  int ret;
  
  if (showinfo) sprintf(buffer,"%s: ",lname);
  ret=ibmpc_arch_scan(lname,device,func,data,showinfo?buffer:NULL);
  if (showinfo) printk(KERN_INFO "%s",buffer);
  return ret;
}



