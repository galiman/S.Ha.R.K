
/*
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


#include <fs/maccess.h>
#include <fs/util.h>
#include <fs/errno.h>
#include <fs/stat.h>
#include <fs/dirent.h>
#include <fs/fcntl.h>

#include "dcache.h"
#include "fs.h"
#include "msdos.h"
#include "fsconst.h"

#include "file.h"
#include "fileop.h"
#include "msdos/msdos_f.h"

#include "debug.h"

/* debug __position() */
#define DEBUG_POSITION KERN_DEBUG
#undef DEBUG_POSITION

/* debug __increase() */
#define DEBUG_INCREASE KERN_DEBUG
#undef DEBUG_INCREASE

/*
 *
 */

#ifdef DEBUG_POSITION
#define printk3a(fmt,args...) \
        if (debug_info) printk(DEBUG_POSITION fmt,##args)
#else
#define printk3a(fmt,args...)
#endif

#ifdef DEBUG_INCREASE
#define _printk1(fmt,args...) \
        if (debug_info) printk(DEBUG_INCREASE fmt,##args)
#else
#define _printk1(fmt,args...)
#endif

static inline void startoffile(struct inode *in, struct file *f)
{
  MSDOS_F(f).cluster=MSDOS_I(in).scluster;
  MSDOS_F(f).bakcluster=NO_CLUSTER;
  MSDOS_F(f).lsector=msdos_cluster2sector(in,MSDOS_F(f).cluster);
  if (MSDOS_F(f).cluster!=FREE_CLUSTER)
    _assert(MSDOS_F(f).lsector!=(__uint32_t)-1);
  MSDOS_F(f).sectnum=0;
  MSDOS_F(f).bpos=0;
}

/*
 *
 *
 */

static int msdos_open(struct inode *in, struct file *f)
{
  startoffile(in,f);
  f->f_pos=0;
  printk0("msdos_open: starting (cluster 0x%lx) (lsect %li)",
	  (long)MSDOS_I(in).scluster,
	  (long)MSDOS_F(f).lsector);
  return EOK;
}

static void __position(struct file *f,__off_t pos)
{
  struct inode *in=f->f_dentry->d_inode;
  struct super_block *sb=f->f_dentry->d_sb;
  __uint32_t nc;

  /* it can be optimized:
   * if the new position is after the actual position we
   * can not go thought all the file list chain
   */
  
  /* from 0 (start-of-file) to in->i_st.st_size (end-of-file) */
  _assert(pos>=0&&pos<=in->i_st.st_size);

  /* remeber that f->f_pos is the position (in bytes) of the next
   * bytes to read/write
   * so
   * if we are at end-of-file the bytes is not into the file
   */
  
  printk3a("__position: START");

  f->f_pos=pos;
  startoffile(in,f);
  
  nc=pos/(MSDOS_SB(sb).spc*SECTORSIZE);  /* number of cluster to skip */
  pos-=nc*(MSDOS_SB(sb).spc*SECTORSIZE); 
  printk3a("__position: cluster to skip %li",(long)nc);
  printk3a("__position: position into cluster %li",(long)pos);
  
  /* very good :-( I must go thought the list chain! */
  while (nc>0) {
    MSDOS_F(f).bakcluster=MSDOS_F(f).cluster;
    MSDOS_F(f).cluster=msdos_nextcluster(sb,MSDOS_F(f).cluster);
    nc--;
  }

  MSDOS_F(f).sectnum=pos/SECTORSIZE;
  pos-=MSDOS_F(f).sectnum*SECTORSIZE;
  printk3a("__position: sector number %i",MSDOS_F(f).sectnum);
  
  MSDOS_F(f).lsector=msdos_cluster2sector(in,MSDOS_F(f).cluster);
  _assert(MSDOS_F(f).lsector!=(__uint32_t)-1);
  MSDOS_F(f).lsector+=MSDOS_F(f).sectnum;
  
  MSDOS_F(f).bpos=pos;
  printk3a("__position: position into sector %li",(long)pos);
  printk3a("__position: logical sector %li",(long)MSDOS_F(f).lsector);

  printk3a("__position: END");
}

static __off_t msdos_lseek(struct file *f, __off_t off, int whence)
{
  struct inode *in=f->f_dentry->d_inode;
  __off_t   pos;

  /* da fare il test sulla fine cluster!!!! */

  printk3("msdos_lseek: START");
  
  printk3("msdos_lseek: current position %li",(long)f->f_pos);  
  switch (whence) {
    case SEEK_SET: pos=off; break;
    case SEEK_CUR: pos=f->f_pos+off; break;
    case SEEK_END: pos=in->i_st.st_size+off; break;
    default:
      printk3("msdos_lseek: END");
      return -EINVAL;
  }
  printk3("msdos_lseek: next position %li",(long)pos);

  if (pos<0) {
    printk3("msdos_lseek: out of position range");
    printk3("msdos_lseek: END");
    return -ESPIPE; /* it is "illegal seek" */
  }
  
  f->f_pos=pos;

  if (f->f_pos>in->i_st.st_size) {
    printk3("msdos_lseek: after end-of-file");
    printk3("msdos_lseek: END");
    return f->f_pos;
  }

  __position(f,pos);

  printk3("msdos_lseek: END");
  return f->f_pos;  
}

static int msdos_close(struct inode *in, struct file *f)
{
  return EOK;
}

/* --- */

/* advance the file pointer by sz bytes */
/* PS: sz must NOT cross sectors (see msdos_read())*/
static void __inline__ __advance(struct file *f,
			       struct super_block *sb,
			       struct inode *in,
			       int sz)
{
  f->f_pos+=sz;
  if (sz==SECTORSIZE-MSDOS_F(f).bpos) {
    MSDOS_F(f).bpos=0;
    MSDOS_F(f).lsector++;
    MSDOS_F(f).sectnum++;

    if (MSDOS_I(in).scluster==ROOT_CLUSTER) return;
	
    if (MSDOS_F(f).sectnum==MSDOS_SB(sb).spc) {
      MSDOS_F(f).sectnum=0;

      //printk(KERN_DEBUG "act cluster 0x%04li",(long)MSDOS_F(f).cluster);

      MSDOS_F(f).bakcluster=MSDOS_F(f).cluster;
      MSDOS_F(f).cluster=msdos_nextcluster(sb,MSDOS_F(f).cluster);
      if (MSDOS_F(f).cluster!=NO_CLUSTER) {
	MSDOS_F(f).lsector=msdos_cluster2sector(in,MSDOS_F(f).cluster);
	_assert(MSDOS_F(f).lsector!=(__uint32_t)-1);
      }
      
      //printk(KERN_DEBUG "new cluster 0x%04li",(long)MSDOS_F(f).cluster);
      
    }
  } else {
    MSDOS_F(f).bpos+=sz;
  }
}

/* increase the file size by delta (we must be on end-of-file) */
static __ssize_t __inline__ __increase(struct file *f,
			       struct super_block *sb,
			       struct inode *in,
			       __ssize_t delta)
{
  __uint16_t scluster,cluster;
  __ssize_t l;
  long i,n;

  _printk1("__increase: START");
  _printk1("__increase: clusters act:0x%04x bak:0x%04x",
	   MSDOS_F(f).cluster,MSDOS_F(f).bakcluster);

  /*
   * MSDOS_F(f).bakcluster contains the previous cluster of the actual
   * file position;
   * it is used only into when MSDOS_F(f).cluster==NO_CLUSTER
   * (we are at end-of-file of a file that is multiple of a cluster)
   */

  _assert(delta!=0);
  if (MSDOS_I(in).scluster==ROOT_CLUSTER) return 0;

  if (MSDOS_F(f).cluster==FREE_CLUSTER) {
    /* we must allocate a cluster */
    /* we do not have any cluster in this chain! */
    _printk1("__increase: free cluster");
    MSDOS_F(f).cluster=msdos_addcluster(sb,NO_CLUSTER);
    if (MSDOS_F(f).cluster==NO_CLUSTER) return 0;
    MSDOS_F(f).lsector=msdos_cluster2sector(in,MSDOS_F(f).cluster);    
    _assert(MSDOS_F(f).lsector!=(__uint32_t)-1);
    MSDOS_I(in).scluster=MSDOS_F(f).cluster;
    in->i_dirty=1;
  }
  
  if (MSDOS_F(f).cluster==NO_CLUSTER) {
    /* we must allocate a cluster */
    /* we are at end-of-file on a cluster boundary! */
    _printk1("__increase: no cluster");    
    _assert(MSDOS_F(f).bakcluster!=NO_CLUSTER);
    MSDOS_F(f).cluster=msdos_addcluster(sb,MSDOS_F(f).bakcluster);
    if (MSDOS_F(f).cluster==NO_CLUSTER) return 0;
    MSDOS_F(f).lsector=msdos_cluster2sector(in,MSDOS_F(f).cluster);
    _assert(MSDOS_F(f).lsector!=(__uint32_t)-1);
  }
  
  _printk1("__increase: compute space available");
  
  /* space available (to the end of cluster) */
  l=((__ssize_t)MSDOS_SB(sb).spc-(__ssize_t)MSDOS_F(f).sectnum-1)*SECTORSIZE;
  l+=(SECTORSIZE-(__ssize_t)MSDOS_F(f).bpos);

  if ((long)delta-(long)l>0) {
    /* some other clusters needed */
    n=(delta-l)/(SECTORSIZE*MSDOS_SB(sb).spc)+1;
    scluster=cluster=MSDOS_F(f).cluster;

    _printk1("__increase: other clusters needed");
    
    for (i=0;i<n;i++) {
      _printk1("__increase: added cluster from 0x%04x",cluster);
      cluster=msdos_addcluster(sb,cluster);
      if (cluster==NO_CLUSTER) {
	/* we shall allow an allocation lesser than requested */
	delta=l+i*SECTORSIZE*MSDOS_SB(sb).spc;
	break;
      }
    }
    
  }

  if (delta>0) {
    _printk1("__increase: delta>0 => dirty inode");    
    in->i_st.st_size+=delta;
    in->i_dirty=1;
  }
  _printk1("__increase: END");
  return delta;
}

/*
 *
 */

static __ssize_t msdos_read(struct file *f, char *p, __ssize_t d)
{
  struct inode *in=f->f_dentry->d_inode;
  struct super_block *sb=f->f_dentry->d_sb;
  dcache_t *buf;
  int sz;
  __ssize_t bc;
  
  //printk(KERN_DEBUG "reading %i bytes from position %li",d,(long)f->f_pos);

  if (d==0) return 0;
  
  /* if we are at or after the end-of-file */
  if (f->f_pos>=in->i_st.st_size) return 0;
    
  bc=0;
  while (bc<d&&in->i_st.st_size>f->f_pos) {
    buf=dcache_lock(sb->sb_dev,MSDOS_F(f).lsector);
    if (buf==NULL) return -EIO;
    sz=min(d-bc,SECTORSIZE-MSDOS_F(f).bpos);
    sz=min(sz,in->i_st.st_size-f->f_pos);
    memcpytouser(p,buf->buffer+MSDOS_F(f).bpos,sz);
    dcache_unlock(buf);
    p+=sz;    
    bc+=sz;
    __advance(f,sb,in,sz);    
  }

  return bc;
}

static __ssize_t msdos_write(struct file *f, char *p, __ssize_t d)
{
  struct inode *in=f->f_dentry->d_inode;
  struct super_block *sb=f->f_dentry->d_sb;
  dcache_t *buf;
  int sz;
  __ssize_t bc;

  //printk(KERN_DEBUG "writing %i bytes from position %li",d,(long)f->f_pos);
	
  if (d==0) return 0;

  /* O_APPEND flag - test for safety */
  /*
  if (f->f_flags&O_APPEND)
    if (f->f_pos!=in->i_st.st_size) {
      __position(f,in->i_st.st_size);      
    }
  */
  
  if (f->f_pos>in->i_st.st_size) {
    /* lseek() has moved the file position AFTER the end of file !!*/

    __ssize_t inc,delta;

    //printk(KERN_DEBUG "lseek() after end of file!");
    
    delta=f->f_pos-in->i_st.st_size;
    __position(f,in->i_st.st_size);
    inc=__increase(f,sb,in,delta);
    if (inc!=delta) return -EIO;

    bc=0;
    while (bc<delta) {
      buf=dcache_acquire(sb->sb_dev,MSDOS_F(f).lsector);    
      if (buf==NULL) return -EIO;
      dcache_dirty(buf);
      sz=min(delta-bc,SECTORSIZE-MSDOS_F(f).bpos);
      sz=min(sz,in->i_st.st_size-f->f_pos);
      memset(buf->buffer+MSDOS_F(f).bpos,0,sz);
      dcache_release(buf);
      bc+=sz;
      __advance(f,sb,in,sz);    
    }
    
  }

  /*
  printk(KERN_DEBUG "pos: %li d: %li act size: %li",
	 (long)f->f_pos,(long)d,(long)in->i_st.st_size);
  */
  
  if (f->f_pos+d>in->i_st.st_size) {
    /* no room for the data that must be written */
    __ssize_t inc;

    //printk(KERN_DEBUG "increase request" );
    
    inc=__increase(f,sb,in,f->f_pos+d-in->i_st.st_size);
    if (inc==0) return -EIO;
    d=inc+in->i_st.st_size-f->f_pos;

    //printk(KERN_DEBUG "new size: %li",(long)in->i_st.st_size);
  }

  //printk(KERN_DEBUG "init writing");

  bc=0;
  while (bc<d&&in->i_st.st_size>f->f_pos) {
    buf=dcache_acquire(sb->sb_dev,MSDOS_F(f).lsector);
    if (buf==NULL) {
      //printk(KERN_DEBUG "can't aquire!");
      return -EIO;
    }    
    //printk(KERN_DEBUG "write on %li",MSDOS_F(f).lsector);	   
    dcache_dirty(buf);
    sz=min(d-bc,SECTORSIZE-MSDOS_F(f).bpos);
    sz=min(sz,in->i_st.st_size-f->f_pos);
    memcpyfromuser(buf->buffer+MSDOS_F(f).bpos,p,sz);
    dcache_release(buf);
    p+=sz;    
    bc+=sz;
    __advance(f,sb,in,sz);    
  }

  return bc;
}

#ifdef DEBUG_READDIR_DUMP
#define dump_directory_entry(ptr) msdos_dump_direntry(ptr)
#else
#define dump_directory_entry(ptr)
#endif

//int startcounter=0;
//int counter;

static int msdos_readdir(struct file *f, void *den)
{
  struct super_block *sb=f->f_dentry->d_sb;
  struct inode *in=f->f_dentry->d_inode;
  struct directoryentry *de;
  struct dirent ude;
  dcache_t *buf=NULL;
  struct qstr name;
  __uint32_t lsect;
  int res;

  //debug_check_mutex(__FILE__,__LINE__,501);
  
  printk5("msdos_readdir: START");

  if (MSDOS_F(f).cluster==NO_CLUSTER) {
    printk5("msdos_readdir: end of cluster (no more entries)");
    printk5("msdos_readdir: END");
    return 1;
  }
  
  lsect=-1;
  for (;;) {

    if (MSDOS_F(f).lsector!=lsect) {
      if (lsect!=-1) dcache_unlock(buf);
      buf=dcache_lock(sb->sb_dev,MSDOS_F(f).lsector);
      if (buf==NULL) {
	//printk(KERN_ERR "msdos_readdir: can't lock sector %li",
	//       (long)MSDOS_F(f).lsector);
	printk5("msdos_readdir: END");
	return -EIO;
      }
      lsect=MSDOS_F(f).lsector;
      printk5("msdos_readdir: read %li logical sector",(long int)lsect);
    }

    de=(struct directoryentry *)(buf->buffer+MSDOS_F(f).bpos);
    dump_directory_entry(de);

    /*
      for debug purpose
    if (startcounter) {
      if (--counter==0) return 1;
    } else

    if (msdos_islastentry(de)) {
      startcounter=1;
      counter=1;
    }
    */
	    
    if (msdos_islastentry(de)) {
      printk5("msdos_readdir: last entry found");
      printk5("msdos_readdir: END");
      // *(int*)&ude=0;
      dcache_unlock(buf);
      return 1;      
    }
    
    __advance(f,sb,in,sizeof(struct directoryentry));

    if (!msdos_isunvalidentry(de)) {
      res=msdos_formatname(de,&name);

      /* there's no need to check for result, but in future... */       
      if (res!=0) {
	printk5("msdos_readdir: entry found BUT can't format the name");
	printk5("msdos_readdir: END");
	dcache_unlock(buf);
	return -EIO;
      }

      strcpy(ude.d_name,QSTRNAME(&name));
      printk5("msdos_readdir: entry found '%s'",ude.d_name);
      break;
    }
    printk5("msdos_readdir: invalid entry found");
  }
  dcache_unlock(buf);

  __copy_to_user(den,&ude,sizeof(struct dirent));
  printk5("msdos_readdir: END");
  return EOK;
}


/*--------*/

struct file_operations msdos_file_ro_operations={
  msdos_lseek,
  msdos_read,
  dummy_write,
  msdos_readdir,
  msdos_open,
  msdos_close
};

struct file_operations msdos_file_rw_operations={
  msdos_lseek,
  msdos_read,
  msdos_write,
  msdos_readdir,
  msdos_open,
  msdos_close
};
