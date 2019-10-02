
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

#include <fs/types.h>
#include <fs/errno.h>
#include <fs/task.h>
#include <fs/maccess.h>
#include <fs/stat.h>
#include <fs/fcntl.h>
#include <fs/mount.h>

#include "fsconst.h"
#include "file.h"
#include "dentry.h"
#include "inode.h"
#include "fileop.h"
#include "inodeop.h"
#include "super.h"
#include "fs.h"

#include "debug.h"


/*
  Example of how the code is expanded in system with/without
  memory protection.
  
  If DUMMY_MACCESS is defined:
  
  char *pathname;
  int len;

  len=1;
  if ((!len)||len>MAXSIZE) return -EVERIFY;
  pathname=userpathname;
  ...


  if DUMMY_MACCESS is NOT defined:
  
  char pathname[MAXSIZE];
  int len;

  len=verifyreadnolen(userpathanme);
  if ((!len)||len>MAXSIZE) return -EVERIFY;
  memcpyfromuser(pathname,userpathname);
  ...

*/

/* PS: open() e opendir() potrebbero condividere codice! */

int k_open(char *upathname, int oflag, int mode)
{
  declare_buffer(char,pathname,MAXPATHNAME);
  int len;
  struct file   *f;
  struct dentry *d;
  int           ret,res,createflag;
  __pid_t       pid;

  call_to_fs();
  
  printk0("k_open: START");
  pid=__get_pid();
  
  len=__verify_read_nolen((char*)upathname);
  if ((!len)||(len>MAXPATHNAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(pathname,upathname,len+1);

  printk0("k_open: verifing memory... done");
  
  /* flags not supported */
  /* O_NOCTTY is ignored */
  if (oflag&O_DSYNC||
      oflag&O_NOCTTY||
      oflag&O_NONBLOCK||
      oflag&O_RSYNC||
      oflag&O_SYNC) {
    return_from_fs(-EACCES);
  }

  /* O_TRUNC wiht O_RDONLY not permitted! */
  if ((oflag&O_RDONLY)&&(oflag&O_TRUNC)) return_from_fs(-EACCES);
  
  printk0("k_open: checking flags... done");
  
  if (oflag&O_CREAT) {
    if (oflag&O_EXCL) createflag=DENTRY_MUSTCREATE;
    else createflag=DENTRY_CANCREATE;
  } else createflag=DENTRY_NOCREATE;

  
  d=find_dentry_from_ext(cwden_ptr(pid),(char *)pathname,&createflag);
  if (d==NULL) {
    printk0("k_open: can't find/create directory entry");
    if (createflag&DENTRY_EXIST) return_from_fs(-EEXIST);
    return_from_fs(-ENOENT);
  }
  
  if (__S_ISDIR(d->d_inode->i_st.st_mode)) {
    if (oflag&O_WRONLY||oflag&O_RDWR)
      /* can't write on a directory */
      return_from_fs(-EISDIR);
  }
  
  /* check for read-only filesystem */
  if (!(d->d_sb->sb_mopts.flags&MOUNT_FLAG_RW)) {
    if (oflag&O_WRONLY||oflag&O_RDWR||oflag&O_TRUNC) {
      /* NB: O_CREAT va testata prima della find_dentry() */
      unlock_dentry(d);      
      return_from_fs(-EROFS);
    }
  }
  
  printk0("k_open: find dir entry... done");

  f=get_file(d->d_inode);
  if (f==NULL) {
    unlock_dentry(d);
    return_from_fs(-ENFILE);
  }

  printk0("k_open: getting system file entry... done");

  if (createflag&DENTRY_CREATED)
    d->d_inode->i_st.st_mode=(mode&__ALLPERMS)|__S_IFREG;
  
  f->f_dentry=d;
  f->f_op=d->d_inode->i_op->default_file_ops;
  f->f_pos=0;
  f->f_flag_isdir=__S_ISDIR(d->d_inode->i_st.st_mode);
  
  ret=get_fd(pid,f);
  if (ret==-1) {
    free_file(f);
    unlock_dentry(d);
    return_from_fs(-EMFILE);
  }
  *fildesfflags_ptr(pid,ret)=oflag;

  printk0("k_open: getting file descriptor... done");

  if (oflag&O_TRUNC&&!(createflag&DENTRY_CREATED)) {
    struct inode *in=file_ptr(pid,ret)->f_dentry->d_inode;
    /* acquire write lock on inode's file */
    __rwlock_wrlock(&in->i_lock);
    /* truncate the file */
    in->i_op->truncate(in,0);      
    /* release write lock on inode's file */
    __rwlock_wrunlock(&in->i_lock);
    printk0("k_open: file truncation... done");
  }
  
  res=f->f_op->open(f->f_dentry->d_inode,f);
  if (res!=0) {
    free_fd(pid,ret);
    free_file(f);
    unlock_dentry(d);
    return_from_fs(res);
  }

  printk0("k_open: called proper filesystem open... done");

  printk0("k_open: END");
  return_from_fs(ret);
}

