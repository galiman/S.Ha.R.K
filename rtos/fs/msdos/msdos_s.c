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
 * CVS :        $Id: msdos_s.c,v 1.2 2005/01/08 14:59:23 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2005/01/08 14:59:23 $
 */

#include <fs/types.h>
#include <fs/const.h>
#include <fs/stat.h>
#include <fs/fsinit.h>
#include <fs/fsind.h>
#include <fs/util.h>
#include <fs/mount.h>
#include <fs/util.h>
#include <fs/mount.h>
#include <fs/assert.h>

#include "dcache.h"
#include "fs.h"

#include "msdos/msdos.h"
#include "msdos/msdos_s.h"
#include "msdos/msdos_i.h"

#define DEBUG_READSUPER KERN_DEBUG
#undef DEBUG_READSUPER

#ifdef DEBUG_READSUPER
#define printk0(fmt,args...) \
        printk(DEBUG_READSUPER fmt,##args)
#else
#define printk0(fmt,args...)
#endif

static struct super_block *msdos_read_super(__dev_t dev, __uint8_t fs_ind,
					    struct mount_opts *options);

static struct file_system_type msdosfs[]={
  {"fat16",0,FS_MSDOS,msdos_read_super},
};
/*{"fat16+",0,FS_DOSBIG,msdos_read_super}*/

int msdos_fs_init(FILESYSTEM_PARMS *ptr)
{
  int i;
  for (i=0;i<sizeof(msdosfs)/sizeof(struct file_system_type);i++)
    filesystem_register(msdosfs+i);
  return 0;
}


/*-----------------------------------------*/

/*
 * boot record (akin to super block)
 */

struct bootrecord {
  __uint8_t  reserved[3];
  char       oemname[8];
  __uint16_t bytespersector;
  __uint8_t  sectorspercluster;
  __uint16_t hiddensectors;
  __uint8_t  fatsnumber;
  __uint16_t rootentry;
  __uint16_t sectors;
  __uint8_t  media;
  __uint16_t sectorsperfat;
  __uint16_t sectorpertrak;
  __uint16_t headsperdisk;
  __uint32_t hiddensectors32;
  __uint32_t sectors32;
  __uint16_t physicaldrive;
  __uint8_t  signature;
  __uint8_t  serialnumber[4];
  char       volumelabel[11];
  char       fattype[8];
} __attribute__ ((packed));

static char *chr2str(char *s, int n)
{
  static char str[12];
  memcpy(str,s,n);
  str[n]='\0';
  return str;
}

static __inline__ __uint32_t br_numhiddensectors(struct bootrecord *br)
{
  return br->hiddensectors?(long)br->hiddensectors:(long)br->hiddensectors32;
}

static __inline__ __uint32_t br_numsectors(struct bootrecord *br)
{
  return br->sectors?(long)br->sectors:(long)br->sectors32;
}

static void dump_br(struct bootrecord *br) __attribute__ ((unused));

static void dump_br(struct bootrecord *br)
{
  printk(KERN_DEBUG "oem     : %s",chr2str(br->oemname,8));
  printk(KERN_DEBUG "label   : %s",chr2str(br->volumelabel,11));
  printk(KERN_DEBUG "fat type: %s",chr2str(br->fattype,8));
  printk(KERN_DEBUG "serial  : %02x%02x-%02x%02x",br->serialnumber[3],
	 br->serialnumber[2],
	 br->serialnumber[1],
	 br->serialnumber[0]);
  printk(KERN_DEBUG "sector  : %i bytes",(int)br->bytespersector);
  printk(KERN_DEBUG "cluster : %i sectors",(int)br->sectorspercluster);
  printk(KERN_DEBUG "fat     : %i copies",(int)br->fatsnumber);
  printk(KERN_DEBUG "root    : %i sectors",
	 br->rootentry*32/br->bytespersector);
  printk(KERN_DEBUG "fat     : %i sectors",(int)br->sectorsperfat);
  printk(KERN_DEBUG "hidden  : %li sectors",br->hiddensectors?
	 (long)br->hiddensectors:(long)br->hiddensectors32);
  printk(KERN_DEBUG "size    : %li sectors",br->sectors?
	 (long)br->sectors:(long)br->sectors32);
 
}

/* from Linux */
/* time,date conversions routine */

static int day_n[]={
  0,31,59,90,120,151,181,212,243,273,304,334,0,0,0,0
};
		 
/* Convert a MS-DOS time/date pair to a UNIX date (seconds since 1 1 70). */
__time_t date_dos2unix(__uint16_t time,__uint16_t date)
{
  int month,year;
  __time_t secs;

  month = ((date >> 5) & 15)-1;
  year = date >> 9;
  secs = (time & 31)*2l+60l*((time >> 5) & 63)+(time >> 11)*3600l+86400l*
    ((date & 31)-1+day_n[month]+(year/4l)+year*365l-((year & 3) == 0 &&
						   month < 2 ? 1 : 0)+3653l);
  /* days since 1.1.70 plus 80's leap day */
  /*secs += sys_tz.tz_minuteswest*60;*/
  /*if (sys_tz.tz_dsttime) secs -= 3600;*/
  return secs;
}

/* Convert linear UNIX date to a MS-DOS time/date pair. */
void date_unix2dos(__time_t unix_date,__uint16_t *time,
    __uint16_t *date)
{
  int day,year,nl_day,month;

  /*unix_date -= sys_tz.tz_minuteswest*60;*/
  /*if (sys_tz.tz_dsttime) unix_date += 3600;*/

  *time = (unix_date % 60)/2+(((unix_date/60) % 60) << 5)+
    (((unix_date/3600) % 24) << 11);
  day = unix_date/86400-3652;
  year = day/365;
  if ((year+3)/4+365*year > day) year--;
  day -= (year+3)/4+365*year;
  if (day == 59 && !(year & 3)) {
    nl_day = day;
    month = 2;
  }
  else {
    nl_day = (year & 3) || day <= 59 ? day : day-1;
    for (month = 0; month < 12; month++)
      if (day_n[month] > nl_day) break;
  }
  *date = nl_day-day_n[month-1]+1+(month << 5)+(year << 9);
}

/*
 *
 * Super operations
 *
 */

static int  msdos_init_inode(struct inode *in);
static int  msdos_read_inode(struct inode *in);
static int  msdos_write_inode(struct inode *in);
static int  msdos_put_super(struct super_block *sb);
static int  msdos_delete_inode(struct inode *in);

static struct super_operations msdos_super_operations={
  msdos_init_inode,
  msdos_read_inode,
  msdos_write_inode,
  msdos_put_super,
  msdos_delete_inode
};

static int msdos_put_super(struct super_block *sb)
{
  return 0;
}

static int msdos_init_inode(struct inode *in)
{
  __uint16_t  cluster;
  __uint32_t lsector;
  __uint16_t  deoffs,offs;
  
  /*i_sb gia' messo*/
  /*i_st.st_ino gia' messo*/
  cluster=INODE2CLUSTER(in->i_st.st_ino);
  deoffs=INODE2DEOFFS(in->i_st.st_ino);

  _assert(cluster!=ROOT_CLUSTER); 
  if (cluster==SPECIAL_CLUSTER)
    lsector=MSDOS_SB(in->i_sb).lroot;
  else 
    lsector=(cluster-2)*MSDOS_SB(in->i_sb).spc+MSDOS_SB(in->i_sb).ldata;     
  lsector+=(__uint16_t)(((__uint32_t)deoffs*DIRENTRYSIZE)/SECTORSIZE);
  offs=(((__uint16_t)deoffs*DIRENTRYSIZE)%SECTORSIZE)/DIRENTRYSIZE;

  MSDOS_I(in).lsector=lsector;
  MSDOS_I(in).offs=offs;

  in->i_st.st_dev=in->i_sb->sb_dev;
  in->i_st.st_mode=__DEFFILEMODE;
  in->i_st.st_mode|=__S_IFREG;
  in->i_st.st_blksize=SECTORSIZE;
  in->i_st.st_nlink=1;
  in->i_st.st_uid=0;
  in->i_st.st_gid=0;
  in->i_st.st_size=0;  
  in->i_st.st_atime=in->i_st.st_mtime=in->i_st.st_ctime=0;
  /* for safety */
  if (in->i_sb->sb_mopts.flags&MOUNT_FLAG_RW)
    in->i_op=&msdos_inode_rw_operations;
  else
    in->i_op=&msdos_inode_ro_operations;

  MSDOS_I(in).scluster=FREE_CLUSTER;

  in->i_dirty=1;
  return 0;
}

static int msdos_read_inode(struct inode *in)
{
  struct directoryentry *den;
  dcache_t *ptr;
  __uint16_t  cluster;
  __uint32_t lsector;
  __uint16_t  deoffs,offs;
  
  /*i_sb gia' messo*/
  /*i_st.st_ino gia' messo*/

  cluster=INODE2CLUSTER(in->i_st.st_ino);
  deoffs=INODE2DEOFFS(in->i_st.st_ino);
  if (cluster==ROOT_CLUSTER) {
    /* special case: root inode */
    MSDOS_I(in).lsector=0;
    MSDOS_I(in).offs=0;
    MSDOS_I(in).scluster=ROOT_CLUSTER;
    in->i_st.st_dev=in->i_sb->sb_dev;
    in->i_st.st_mode=__S_IRWXO|__S_IRWXG|__S_IRWXU;
    in->i_st.st_mode|=__S_IFDIR;
    in->i_st.st_nlink=1+1; /* must be fixed */
    in->i_st.st_blksize=SECTORSIZE;
    in->i_st.st_uid=0;
    in->i_st.st_gid=0;
    in->i_st.st_size=MSDOS_SB(in->i_sb).rootsect*SECTORSIZE;  
    in->i_st.st_atime=in->i_st.st_mtime=in->i_st.st_ctime=0;

    /* for safety */
    if (in->i_sb->sb_mopts.flags&MOUNT_FLAG_RW)
      in->i_op=&msdos_inode_rw_operations;
    else
      in->i_op=&msdos_inode_ro_operations;

    in->i_dirty=0;
    return 0;
  } else if (cluster==SPECIAL_CLUSTER) {
    /* special case: an inode on root directory */
    lsector=MSDOS_SB(in->i_sb).lroot;
  } else {
    /* normal inode */
    lsector=(cluster-2)*MSDOS_SB(in->i_sb).spc+MSDOS_SB(in->i_sb).ldata;
  }
  lsector+=(__uint16_t)(((__uint32_t)deoffs*DIRENTRYSIZE)/SECTORSIZE);
  offs=(((__uint16_t)deoffs*DIRENTRYSIZE)%SECTORSIZE)/DIRENTRYSIZE;

  MSDOS_I(in).lsector=lsector;
  MSDOS_I(in).offs=offs;

  ptr=dcache_lock(in->i_sb->sb_dev,lsector);
  if (ptr==NULL) return -1;
  den=(struct directoryentry *)(ptr->buffer)+offs;
		   
  in->i_st.st_dev=in->i_sb->sb_dev;
  in->i_st.st_mode=__S_IRWXO|__S_IRWXG|__S_IRWXU;
  in->i_st.st_mode|=(den->attribute&ATTR_DIR?__S_IFDIR:__S_IFREG);
  in->i_st.st_nlink=1+(__S_ISDIR(in->i_st.st_mode)?1:0); /* must be fixed */
  in->i_st.st_blksize=SECTORSIZE;
  in->i_st.st_uid=0;
  in->i_st.st_gid=0;
  in->i_st.st_size=den->size;  
  in->i_st.st_atime=in->i_st.st_mtime=in->i_st.st_ctime=
    date_dos2unix(den->time,den->date);  
  /* for safety */
  if (in->i_sb->sb_mopts.flags&MOUNT_FLAG_RW)
    in->i_op=&msdos_inode_rw_operations;
  else
    in->i_op=&msdos_inode_ro_operations;

  if (den->cluster==FREECLUSTER)
    MSDOS_I(in).scluster=FREE_CLUSTER;
  else
    MSDOS_I(in).scluster=den->cluster;

  in->i_dirty=0;
  dcache_unlock(ptr);
  return 0;
}

static int msdos_write_inode(struct inode *in)
{
  struct directoryentry *den;
  dcache_t              *ptr;
  __uint16_t            cluster;
  
  cluster=INODE2CLUSTER(in->i_st.st_ino);
  if (cluster==ROOT_CLUSTER) return 0;

  ptr=dcache_acquire(in->i_sb->sb_dev,MSDOS_I(in).lsector);
  if (ptr==NULL) {
    dcache_release(ptr);
    return -1;
  }
  den=(struct directoryentry *)(ptr->buffer)+MSDOS_I(in).offs;

  //printk("writing inode\n start cluster=0x%04x\n",MSDOS_I(in).scluster);
  
  cluster=MSDOS_I(in).scluster;
  _assert(cluster!=NO_CLUSTER&&
	  cluster!=ROOT_CLUSTER&&
	  cluster!=SPECIAL_CLUSTER);
  if (cluster==FREE_CLUSTER) cluster=FREECLUSTER;
  
  date_unix2dos(in->i_st.st_atime,&den->time,&den->date);
  den->cluster=cluster;
  den->size=in->i_st.st_size;

  in->i_dirty=0;
  dcache_dirty(ptr);
  
  dcache_release(ptr);		
  return 0;
}

static int msdos_delete_inode(struct inode *in)
{
  struct directoryentry *den;
  dcache_t              *ptr;
  __uint16_t            cluster;
  
  cluster=INODE2CLUSTER(in->i_st.st_ino);
  if (cluster==ROOT_CLUSTER) return 0;

  ptr=dcache_acquire(in->i_sb->sb_dev,MSDOS_I(in).lsector);
  if (ptr==NULL) {
    dcache_release(ptr);
    return -1;
  }
  den=(struct directoryentry *)(ptr->buffer)+MSDOS_I(in).offs;

  msdos_markentryfree(den);
  
  dcache_dirty(ptr);  
  dcache_release(ptr);		
  return 0;

}

static int msdos_dump_inode(struct inode *in) __attribute__((unused));

static int msdos_dump_inode(struct inode *in)
{
  printk(KERN_DEBUG "dev   : %i",in->i_st.st_dev);
  printk(KERN_DEBUG "inode : %08lx",(long)in->i_st.st_ino);
  printk(KERN_DEBUG "mode  : %04x",in->i_st.st_mode);
  printk(KERN_DEBUG "nlink : %i",in->i_st.st_nlink);
  printk(KERN_DEBUG "uid   : %i",in->i_st.st_uid);
  printk(KERN_DEBUG "gid   : %i",in->i_st.st_gid);
  //printk(KERN_DEBUG "rdev  : %i",in->i_st.st_rdev);
  printk(KERN_DEBUG "size  : %li",in->i_st.st_size);
  printk(KERN_DEBUG "atime : %li",(long)in->i_st.st_atime);
  printk(KERN_DEBUG "mtime : %li",(long)in->i_st.st_mtime);
  printk(KERN_DEBUG "ctime : %li",(long)in->i_st.st_ctime);
  
  printk(KERN_DEBUG "lsect : %li",(long)MSDOS_I(in).lsector);
  printk(KERN_DEBUG "off   : %li",(long)MSDOS_I(in).offs);
  printk(KERN_DEBUG "sclust: %li",(long)MSDOS_I(in).scluster);
  
  return 0;
}

/*
 *
 * Mount operation
 *
 */

#define MSDOSSBSIGNATURE 0x29

static struct super_block *msdos_read_super(__dev_t dev, __uint8_t fs_ind,
					    struct mount_opts *opts)
{
  struct inode       *in;
  struct bootrecord  *br;
  struct super_block *sb;
  dcache_t           *dc;
  int                res;

  printk0("msdos_read_super(): START");
  
  dc=dcache_lock(dev,0);
  if (dc==NULL) {
    printk(KERN_DEBUG "msdos_read_super FAIL: can't read/lock cache");
    return NULL;
  }
  br=(struct bootrecord*)dc->buffer;
  if (br->fatsnumber>MAXFATS) {
    dcache_unlock(dc);
    printk(KERN_DEBUG "msdos_read_super FAIL: too many FATS (%i)",
	   br->fatsnumber);
    return NULL;
  }  
  if (br->signature!=MSDOSSBSIGNATURE) {
    dcache_unlock(dc);
    printk(KERN_DEBUG "msdos_read_super FAIL: can't find MSDOS signature");
    return NULL;
  }

  printk0("msdos_read_super(): boot sector locked");

  sb=super_getblock();
  if (sb==NULL) {
    dcache_unlock(dc);
    printk(KERN_DEBUG "msdos_read_super FAIL: can't have a super struct");
    return NULL;
  }

  printk0("msdos_read_super(): super structure allocated");

  if (opts==NULL) {msdosfs_std_parms(sb->sb_mopts);}
  else memcpy(&sb->sb_mopts,opts,sizeof(struct mount_opts));
  
  sb->sb_dev=dev;
  sb->sb_op=&msdos_super_operations;
  sb->sb_dop=&msdos_dentry_operations;
  
  MSDOS_SB(sb).lfat=br_numhiddensectors(br);
  MSDOS_SB(sb).lroot=MSDOS_SB(sb).lfat+
    (__uint32_t)br->sectorsperfat*br->fatsnumber;
  MSDOS_SB(sb).ldata=MSDOS_SB(sb).lroot+
    br->rootentry*DIRENTRYSIZE/br->bytespersector;
  MSDOS_SB(sb).lsize=br_numsectors(br)+br_numhiddensectors(br);
  MSDOS_SB(sb).spc=br->sectorspercluster;
  MSDOS_SB(sb).rootsect=(__uint32_t)br->rootentry*DIRENTRYSIZE/
    br->bytespersector;
  MSDOS_SB(sb).spf=br->sectorsperfat;
  MSDOS_SB(sb).nfat=br->fatsnumber;
  MSDOS_SB(sb).stclust=STARTCLUSTER;

  printk0("msdos_read_super(): super structure filled");

  in=catch_inode();
  if (in==NULL) {
    super_freeblock(sb);
    dcache_unlock(dc);
    printk(KERN_DEBUG "msdos_read_super FAIL: can't have an inode struct");
    return NULL;    
  }

  printk0("msdos_read_super(): inode structure allocated");
  
  sb->sb_root=in;
  in->i_sb=sb;
  in->i_st.st_ino=ROOT_INODE;
  res=msdos_read_inode(in);
  if (res!=0) {
    free_inode(in);
    super_freeblock(sb);
    dcache_unlock(dc);
    printk(KERN_DEBUG "msdos_read_super FAIL: can't read inode");
    return NULL;    
  }

  printk0("msdos_read_super(): root inode read");

  insert_inode(in);

  printk0("msdos_read_super(): root inode inserted");
  
  dcache_unlock(dc);
  printk0("msdos_read_super(): boot sector released");
  printk0("msdos_read_super(): END");
  return sb;
}


/* to remove */
/*
{
  struct qstr str;
  struct inode *in2,*in3,*in4;
  struct dentry *d;
    
  extern struct inode *msdos_lookup(struct inode *, struct dentry *);
    
  printk(KERN_DEBUG "---ROOT INODE---\n");
  msdos_dump_inode(in);
    
  d=get_dentry();
  if (d!=NULL) {
    exit(1234);
  }
  strcpy(d->d_name.name,"BC45");
  d->d_name.nameptr=NULL;
  d->d_op=&msdos_dentry_operations;

  printk(KERN_DEBUG "searching --------------------------\n");
  in2=msdos_lookup(in,d);
  if (in2!=NULL) printk(KERN_DEBUG "found!\n");
  else printk(KERN_DEBUG "not found!\n");

  printk(KERN_DEBUG "---IN2 INODE---\n");
  msdos_dump_inode(in2);

  printk(KERN_DEBUG "searching --------------------------\n");
  strcpy(d->d_name.name,"BIN");
  in3=msdos_lookup(in2,d);
  if (in3!=NULL) printk(KERN_DEBUG "found!\n");
  else printk(KERN_DEBUG "not found!\n");

  printk(KERN_DEBUG "---IN3 INODE---\n");
  msdos_dump_inode(in3);

  printk(KERN_DEBUG "searching --------------------------\n");
  strcpy(d->d_name.name,"pspsp");
  in4=msdos_lookup(in3,d);
  if (in4!=NULL) printk(KERN_DEBUG "found!\n");
  else printk(KERN_DEBUG "not found!\n");

  printk(KERN_DEBUG "---IN4 INODE---\n");
  msdos_dump_inode(in4);
    
}
*/
/* end to remove */
