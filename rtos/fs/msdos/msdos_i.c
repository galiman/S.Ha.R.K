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
 * CVS :        $Id: msdos_i.c,v 1.2 2002/10/28 08:24:43 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2002/10/28 08:24:43 $
 */

#include <fs/types.h>
#include <fs/const.h>
#include <fs/stat.h>
#include <fs/assert.h>

#include "dcache.h"
#include "fs.h"
#include "msdos.h"
#include "msdos/msdos.h"

#include "debug.h"

/*
 * local DEBUG
 */

#define DEBUG_ADDCLUSTER KERN_DEBUG
#undef DEBUG_ADDCLUSTER

#define DEBUG_ADDCLUSTER_EXTRA KERN_DEBUG
#undef DEBUG_ADDCLUSTER_EXTRA

/**/

#ifdef DEBUG_ADDCLUSTER
#define printd0(fmt,args...) \
        if (debug_info) printk(DEBUG_ADDCLUSTER fmt,##args)
#else
#define printd0(fmt,args...)
#endif

#if defined(DEBUG_ADDCLUSTER)&&defined(DEBUG_ADDCLUSTER_EXTRA)
#define printd1(fmt,args...) \
        if (debug_info) printk(DEBUG_ADDCLUSTER fmt,##args)
#else
#define printd1(fmt,args...)
#endif

/*
 *
 */
     
static char *chr2str(char*s, int n)
{
  static char str[13];
  _assert(n<13);
  memcpy(str,s,n);
  str[n]='\0';
  return str;
}

void msdos_dump_direntry(struct directoryentry *ptr)
{
  printk(KERN_DEBUG "status    : 0x%02x",(int)ptr->name[0]);
  printk(KERN_DEBUG "filename  : '%s'",chr2str(ptr->name,8));
  printk(KERN_DEBUG "ext       : '%s'",chr2str(ptr->ext,3));
  printk(KERN_DEBUG "attribute : 0x%02x",ptr->attribute);
  printk(KERN_DEBUG "cluster   : %04x",ptr->cluster);
  printk(KERN_DEBUG "size      : %li",(long)ptr->size);
  printk(KERN_DEBUG "time      : %li",(long)ptr->time);
}

/* what is the logical sector of a cluster into the fat */
static __inline __uint32_t __cluster2lsect(struct super_block *sb,
					   __uint16_t cluster)
{
  return MSDOS_SB(sb).lfat+((__uint32_t)cluster*2)/SECTORSIZE;
}

/* what is the offset of a cluster into the fat */
static __inline __uint32_t __cluster2offs(__uint16_t cluster)
{
  return ((__uint32_t)cluster*2)%SECTORSIZE;
}

/**/

static __inline__ void __lock(struct super_block *sb,
				dcache_t *buf[],
				__uint32_t lsect)
{
  _assert(buf[0]==NULL);
  buf[0]=dcache_lock(sb->sb_dev,lsect);
}

static __inline__ void __unlock(struct super_block *sb,
				dcache_t *buf[])
{
  _assert(buf[0]!=NULL);
  dcache_unlock(buf[0]);
  buf[0]=NULL;
}

static __inline__ void __acquire(struct super_block *sb,
				dcache_t *buf[],
				__uint32_t lsect)
{
  int i=0;
  _assert(buf[0]==NULL);
  _assert(MSDOS_SB(sb).nfat<=MAXFATS);
  for (i=0;i<MSDOS_SB(sb).nfat;i++) {
    buf[i]=dcache_acquire(sb->sb_dev,lsect);
    _assert(buf[i]!=NULL);
    lsect+=MSDOS_SB(sb).spf;
  }
}

static __inline__ void __release(struct super_block *sb,
				dcache_t *buf[])
{
  int i;
  _assert(buf[0]!=NULL);
  for (i=MSDOS_SB(sb).nfat-1;i>=0;i--) {
    _assert(buf[i]!=NULL);
    dcache_release(buf[i]);
  }
  buf[0]=NULL;
}

static __inline__ __uint16_t __getcluster(struct super_block *sb,
					  dcache_t *buf[],
					  __uint16_t offs)
{
  _assert(buf[0]!=NULL);
  return *(__uint16_t*)(buf[0]->buffer+offs);  
}

static __inline__ void __setcluster(struct super_block *sb,
				    dcache_t *buf[],
				    __uint16_t offs,
				    __uint16_t val)
{
  int i;
  _assert(buf[0]!=NULL);
  for (i=0;i<MSDOS_SB(sb).nfat;i++) {
    _assert(buf[i]!=NULL);
    *(__uint16_t*)(buf[i]->buffer+offs)=val;
    dcache_dirty(buf[i]);
    dcache_skipwt(buf[i]);
  }
}

/*
 *
 */

__uint16_t msdos_nextcluster(struct super_block *sb, __uint16_t cluster)
{
  dcache_t *buf[MAXFATS];
  __uint32_t lsect;
  __uint16_t  offs;
  __uint16_t  ret;

  buf[0]=NULL;
  lsect=__cluster2lsect(sb,cluster);
  offs=__cluster2offs(cluster);

  __lock(sb,buf,lsect);
  ret=__getcluster(sb,buf,offs);  
  __unlock(sb,buf);

  return ret;
}

void msdos_freecluster(struct super_block *sb, __uint16_t cluster)
{
  dcache_t   *buf[MAXFATS];
  __uint32_t lsect;
  __uint16_t offs;

  buf[0]=NULL;
  lsect=__cluster2lsect(sb,cluster);
  offs=__cluster2offs(cluster);
 
  __acquire(sb,buf,lsect);

  _assert(__getcluster(sb,buf,offs)!=FREECLUSTER);
  __setcluster(sb,buf,offs,FREECLUSTER);
  
  __release(sb,buf);
  return;
}

void msdos_lastcluster(struct super_block *sb, __uint16_t cluster)
{
  dcache_t   *buf[MAXFATS];
  __uint32_t lsect;
  __uint16_t offs;

  buf[0]=NULL;
  lsect=__cluster2lsect(sb,cluster);
  offs=__cluster2offs(cluster);
 
  __acquire(sb,buf,lsect);

  //_assert(__getcluster(sb,buf,offs)==FREECLUSTER);
  _assert(__getcluster(sb,buf,offs)!=LASTCLUSTER);
  __setcluster(sb,buf,offs,LASTCLUSTER);
  
  __release(sb,buf);
  return;
}

__uint16_t msdos_addcluster(struct super_block *sb, __uint16_t lacluster)
{
  dcache_t   *buf[MAXFATS];
  __uint32_t lsect;
  __uint16_t offs;
  __uint32_t olsect;
  __uint16_t cluster,stcluster;

  printd0("msdos_addcluster START");
  
  stcluster=cluster=MSDOS_SB(sb).stclust;  
  olsect=0;
  buf[0]=NULL;

  printd0("searching from cluster 0x%04lx",(long)stcluster);
  
  for (;;) {
    lsect=__cluster2lsect(sb,cluster);
    offs=__cluster2offs(cluster);

    if (lsect>=MSDOS_SB(sb).lroot) {
      cluster=STARTCLUSTER;
      lsect=__cluster2lsect(sb,cluster);
      offs=__cluster2offs(cluster);
      _assert(lsect<MSDOS_SB(sb).lroot);
    }
	
    if (lsect!=olsect) {
      if (buf[0]!=NULL) __release(sb,buf);
      __acquire(sb,buf,lsect);
      printd1("read sector %li",(long)lsect);
      olsect=lsect;
    }
    
    if (__getcluster(sb,buf,offs)==FREECLUSTER) {
      printd0("cluster 0x%04lx (into FAT lsect: %li) added to 0x%04lx",
	      (long)cluster,(long)lsect,(long)lacluster);
      MSDOS_SB(sb).stclust=cluster;
      __setcluster(sb,buf,offs,LASTCLUSTER);

      /* if lacluster is not a cluster we do NOT change the FAT table!*/
      if (lacluster==NO_CLUSTER) break;
      __release(sb,buf);

      lsect=__cluster2lsect(sb,lacluster);
      offs=__cluster2offs(lacluster);
      __acquire(sb,buf,lsect);
      _assert(__getcluster(sb,buf,offs)==LASTCLUSTER);
      __setcluster(sb,buf,offs,cluster);
      break;
    }
    
    cluster++;
    if (cluster==stcluster) {
      cluster=NO_CLUSTER;
      break;
    }
  }  

  __release(sb,buf);
  printd0("msdos_addcluster END");
  return cluster;
}

void msdos_freeclusterchain(struct super_block *sb, __uint16_t cluster)
{
  dcache_t *buf[MAXFATS];
  __uint32_t  lsect;
  __uint16_t  offs;
  __uint32_t  olsect;

  buf[0]=NULL;
  olsect=0;
  
  for (;;) {
    lsect=__cluster2lsect(sb,cluster);
    offs=__cluster2offs(cluster);

    if (lsect!=olsect) {
      if (buf[0]!=NULL) __release(sb,buf);
      __acquire(sb,buf,lsect);
      olsect=lsect;
    }
    
    cluster=__getcluster(sb,buf,offs);
    _assert(cluster!=FREECLUSTER);
    __setcluster(sb,buf,offs,FREECLUSTER);

    if (cluster==LASTCLUSTER) break;
  }  

  __release(sb,buf);  
  return;
}

/*
 *
 */

/* from FS specific filename into 'de' to the general filename into 'str' */
int msdos_formatname(struct directoryentry *de, struct qstr *str)
{  
  char *s=str->name;
  char *ptr=de->name;
  int i;
  
  if (*ptr==0x05) *s++=0xed;
  else *s++=*ptr++;
  i=sizeof(de->name)-1;
  while (i-->0) {
    if (*ptr==' '||*ptr=='\0') break;
    *s++=*ptr++;
  }

  if (de->ext[0]!=' '&&de->ext[0]!='\0') {
    *s++='.';
    ptr=de->ext;
    i=sizeof(de->ext);
    while (i-->0) {
      if (*ptr==' '||*ptr=='\0') break;
      *s++=*ptr++;
    }
  }

  *s++='\0';
  
  str->nameptr=NULL;
  return 0;
}

/* all filename are traslated in upper case */
#define _up(x) (((x)>='a'&&(x)<='z')?(x)-'a'+'A':(x))

/* from the general filename into 'str' to FS specific filename into 'de' */
int msdos_deformatname(struct qstr *str,struct directoryentry *de)
{
  char *s=QSTRNAME(str);
  char *ptr=de->name;
  int i;

  memset(de->name,' ',sizeof(de->name));
  memset(de->ext,' ',sizeof(de->ext));
  
  i=0;
  for (i=0;i<sizeof(de->name);i++,s++,ptr++) {    
    if (*s=='.'||*s=='\0') break;
    *ptr=_up(*s);
  }
  if (*s=='\0') return 0;
  
  while (*s!='.') {
    if (*s=='\0') return 0;
    s++;
  }

  ptr=de->ext;
  s++;
  for (i=0;i<sizeof(de->name);i++,s++,ptr++) {    
    if (*s=='.'||*s=='\0') break;
    *ptr=_up(*s);
  }

  
  return 0;
}

static int msdos_dummy_truncate(struct inode *in, __off_t len)
{
  return -1;
}

static int msdos_truncate(struct inode *in, __off_t len)
{
  int numclusters;
  int cl,pcl;
  
  if (len>=in->i_st.st_size) return 0;

  if (len==0) {
    msdos_freeclusterchain(in->i_sb,MSDOS_I(in).scluster);
    MSDOS_I(in).scluster=FREE_CLUSTER;
    in->i_st.st_size=0;
    in->i_dirty=1;
    return 0;
  }
  
  numclusters=(len-1)/(MSDOS_SB(in->i_sb).spc*(long)SECTORSIZE)+1;

  /* walk on cluster chain */
  pcl=-1;
  cl=MSDOS_I(in).scluster;  
  while (numclusters-->0) {
    pcl=cl;
    cl=msdos_nextcluster(in->i_sb,cl);
  }
  if (cl!=LASTCLUSTER) {
    _assert(pcl!=-1);
    msdos_lastcluster(in->i_sb,pcl);
    msdos_freeclusterchain(in->i_sb,cl);
  }
  
  in->i_st.st_size=len;
  in->i_dirty=1;
  return 0;
}

/* msdos_create_inode() and msdos_lockup() are very similar */
/* they should be share code */

static struct inode *msdos_create_inode(struct inode *in, struct dentry *den)
{
  struct inode *in2;
  struct directoryentry *de;
  dcache_t *buf;
  __uint32_t lsect;
  __uint16_t  ldisk;
  __uint16_t cluster,prevcluster;
  int flagroot;
  int spc;
  int i,j;
  int res;
  
  /* for safety ;-) */
  if (!__S_ISDIR(in->i_st.st_mode)) return NULL;

  flagroot=(((in->i_st.st_ino&CLUSTER_MASK)>>CLUSTER_SHIFT)==ROOT_CLUSTER);
  if (flagroot) {
    cluster=SPECIAL_CLUSTER;
    lsect=MSDOS_SB(in->i_sb).lroot;
    spc=MSDOS_SB(in->i_sb).rootsect;
  } else {
    cluster=MSDOS_I(in).scluster;
    lsect=msdos_cluster2sector(in,cluster);
    _assert(lsect!=(__uint32_t)-1);
    spc=MSDOS_SB(in->i_sb).spc;
  }

  printk1("msdos create inode: start directory scanning");
  
  ldisk=in->i_sb->sb_dev;
  /* for every cluster... */
  for (;;) {

    /* for every sectors... */
    for (j=0;j<spc;j++,lsect++) {
      buf=dcache_acquire(ldisk,lsect);

      printk1("msdos create inode: logical sector %li",(long)lsect);
      
      if (buf==NULL) {
	printk1("msdos create inode: can't read sector");
	return NULL;
      }
      
      /* for every directory entry... */
      de=(struct directoryentry *)(buf->buffer);
      for (i=0;i<SECTORSIZE/DIRENTRYSIZE;i++) {
	
	if (msdos_isfreeentry(de+i)) {
	  printk1("msdos create inode: found free entry");
  
	  in2=catch_inode();
	  printk1("-------------------");	  
	  if (in2==NULL) {
	    printk1("msdos create inode: can't get free inode!");
	    dcache_release(buf);
	    return NULL;	    
	  }
	  
	  in2->i_sb=in->i_sb;
	  in2->i_st.st_ino=CLUOFF2INODE(cluster,j*SECTORSIZE/DIRENTRYSIZE+i);
	  res=in2->i_sb->sb_op->init_inode(in2);
	  if (res!=0) {
	    printk1("msdos create inode: can't init inode!");
	    free_inode(in2);
	    dcache_release(buf);
	    return NULL;    
	  }
	  
	  msdos_markentrybusy(de+i);
	  msdos_deformatname(&den->d_name,de+i);
	  dcache_dirty(buf);
	  
	  dcache_release(buf);
	  return in2;
	}
	
      } /* end directory entry loop */
 
      dcache_release(buf);

      printk1("msdos create inode: next sector");
     
    } /* end sector loop */
    
    if (flagroot) {
      printk1("msdos create inode : no free slot (can't add one)");
      return NULL;
    }
    
    cluster=msdos_nextcluster(in->i_sb,prevcluster=cluster);
    if (cluster==LASTCLUSTER) {
      printk1("msdos create inode: no free slot (try to add one)");

      cluster=msdos_addcluster(in->i_sb,prevcluster);
      if (cluster==NO_CLUSTER) {
	printk1("msdos create inode: can't add cluster");
	return NULL;
      }

      /* zeroing the new cluster */
      lsect=msdos_cluster2sector(in,cluster);
      _assert(lsect!=(__uint32_t)-1);
      for (j=0;j<spc;j++,lsect++) {
	buf=dcache_acquire(ldisk,lsect);
	_assert(buf!=NULL);
	memset(buf->buffer,0,SECTORSIZE);
	dcache_dirty(buf);
	dcache_release(buf);
      }
      
      printk1("msdos create inode: added cluster");
      
    }
    lsect=msdos_cluster2sector(in,cluster);
    _assert(lsect!=(__uint32_t)-1);

    printk1("msdos create inode: next cluster");
    
  } /* end cluster loop */
  
}

static struct inode *msdos_lookup(struct inode *in, struct dentry *den)
{
  struct inode *in2;
  struct directoryentry *de;
  struct qstr str;
  dcache_t *buf;
  __uint32_t lsect;
  __uint16_t  ldisk;
  __uint16_t cluster;
  int flagroot;
  int spc;
  int i,j;
  int res;

  printk1("msdos lookup: START for %s",QSTRNAME(&(den->d_name)));
  
  /* for safety ;-) */
  if (!__S_ISDIR(in->i_st.st_mode)) return NULL;

  printk1("msdos lookup: is directory... ok");
  
  flagroot=(((in->i_st.st_ino&CLUSTER_MASK)>>CLUSTER_SHIFT)==ROOT_CLUSTER);
  if (flagroot) {
    cluster=SPECIAL_CLUSTER;
    lsect=MSDOS_SB(in->i_sb).lroot;
    spc=MSDOS_SB(in->i_sb).rootsect;
  } else {
    cluster=MSDOS_I(in).scluster;
    lsect=msdos_cluster2sector(in,cluster);
    _assert(lsect!=(__uint32_t)-1);
    spc=MSDOS_SB(in->i_sb).spc;
  }

  printk1("msdos lookup: start directory scanning");
  
  ldisk=in->i_sb->sb_dev;
  /* for every cluster... */
  for (;;) {

    /* for every sectors... */
    for (j=0;j<spc;j++,lsect++) {
      buf=dcache_lock(ldisk,lsect);

      printk1("msdos lookup: logical sector %li",(long)lsect);
      
      if (buf==NULL) {
	printk1("msdos lookup: can't read sector");
	return NULL;
      }
      
      /* for every directory entry... */
      de=(struct directoryentry *)(buf->buffer);
      for (i=0;i<SECTORSIZE/DIRENTRYSIZE;i++) {
	if (msdos_islastentry(de+i)) {
	  /* last entry... not found! */
	  dcache_unlock(buf);
	  printk1("msdos lookup: NOT found");
	  return NULL;
	}

	/* if it' an invalid entry (not a regular file or directory entry) */
	if (msdos_isunvalidentry(de+i)) continue;
	
	/* extract name */
	res=msdos_formatname(de+i,&str);
	if (res!=0) {
	  dcache_unlock(buf);
	  printk1("msdos lookup: can't extract filename");
	  return NULL;
	}

	printk1e("msdos lookup: filename '%s'",QSTRNAME(&str));
	
	/* compare name and if found... */
	if (den->d_op->d_compare(den,&den->d_name,&str)==0) {

	  printk1("msdos lookup: found");	  
	  in2=catch_inode();
	  printk1("-------------------");
	  
	  if (in2==NULL) {
	    printk1("msdos lookup: can't get free inode!");
	    dcache_unlock(buf);
	    return NULL;    
	  }
	  in2->i_sb=in->i_sb;
	  in2->i_st.st_ino=CLUOFF2INODE(cluster,j*SECTORSIZE/DIRENTRYSIZE+i);
	  res=in2->i_sb->sb_op->read_inode(in2);
	  if (res!=0) {
	    printk1("msdos lookup: can't read inode!");
	    free_inode(in2);
	    dcache_unlock(buf);
	    return NULL;    
	  }

	  dcache_unlock(buf);
	  return in2;
	}
	
      } /* end directory entry loop */
 
      dcache_unlock(buf);
      printk1("msdos lookup: next sector");
     
    } /* end sector loop */
    
    if (flagroot) {
      printk1("msdos lookup: NOT found (no other entry into root dir)");
      return NULL;
    }
    
    cluster=msdos_nextcluster(in->i_sb,cluster);
    if (cluster==0||cluster>=0xfff0) {
      printk1("msdos lookup: NOT found (end cluster)");	  
      return NULL;
    }
    lsect=msdos_cluster2sector(in,cluster);
    _assert(lsect!=(__uint32_t)-1);

    printk1("msdos lookup: next cluster");
    
  } /* end cluster loop */
  
}

static int msdos_dummy_unlink(struct dentry *d)
{
  return EROFS;
}

static int msdos_unlink(struct dentry *d)
{
  printkd("msdos_unlink: START");
  
  /* only one dentry can own this inode... */
  _assert(d->d_inode->i_dlink==1);

  /* nothing to do */

  /* update link count */
  d->d_inode->i_st.st_nlink--;
  _assert(d->d_inode->i_st.st_nlink==0);
  
  printkd("msdos_unlink: END");
  return 0;  
}

static void msdos_destroy(struct inode *in)
{
  msdos_truncate(in,0);
  return;
}

/* ----------------- */

struct inode_operations msdos_inode_ro_operations={
  &msdos_file_ro_operations,
  msdos_create_inode,
  msdos_lookup,
  msdos_dummy_unlink,
  msdos_destroy,
  msdos_dummy_truncate
};

struct inode_operations msdos_inode_rw_operations={
  &msdos_file_rw_operations,
  msdos_create_inode,
  msdos_lookup,
  msdos_unlink,
  msdos_destroy,
  msdos_truncate
};


/*
 *
 * for inlines
 *
 */

int msdos_islastentry(struct directoryentry *de)
{
  return (*((de)->name)==LASTENTRYCHAR);
}

int msdos_isunvalidentry(struct directoryentry *de)
{
  return (*((de)->name)==ERASEDENTRYCHAR)||
    ((de)->attribute&ATTR_VOLUME)||
    (msdos_islastentry(de));
}
