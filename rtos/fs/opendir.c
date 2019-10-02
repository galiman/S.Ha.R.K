
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

#include "fsconst.h"
#include "file.h"
#include "dentry.h"
#include "inode.h"
#include "fileop.h"
#include "inodeop.h"
#include "fs.h"

#include "debug.h"

int k_opendir(char *upathname)
{
  declare_buffer(char,pathname,MAXPATHNAMELEN+1);
  int           len;
  struct file   *f;
  struct dentry *d;
  int           res;
  int           fd;
  __pid_t       pid;

  call_to_fs();
  
  printk4("k_opendir: START");
  pid=__get_pid();
  
  len=__verify_read_nolen((char*)upathname);
  if ((!len)||(len>MAXPATHNAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(pathname,upathname,len+1);
  
  printk4("k_opendir: verifing memory... done");
  
  d=find_dentry_from(cwden_ptr(pid),(char *)pathname);
  if (d==NULL) return_from_fs(-ENOENT);
  /* NB: a questo punto ci dovrebbe essere il check per le directory! */
  if (!__S_ISDIR(d->d_inode->i_st.st_mode)) return_from_fs(-ENOENT);

  printk4("k_opendir: find dir entry... done");

  /* NB: andrebbe cercato se questo file e' gia' in uso! */
  f=get_file(NULL);
  if (f==NULL) {
    unlock_dentry(d);
    return_from_fs(-ENFILE);
  }

  printk4("k_open: getting system file entry... done");
  
  f->f_dentry=d;
  f->f_op=d->d_inode->i_op->default_file_ops;
  f->f_pos=0;
  f->f_flag_isdir=1;
  
  fd=get_fd(pid,f);
  if (fd==-1) {
    free_file(f);
    unlock_dentry(d);
    return_from_fs(-EMFILE);
  }
  if ((void*)fd==NULL) {
    /* Well, we can't return a value that can be confused with NULL :-( */
    int newfd;
    newfd=get_fd(pid,f);
    if (newfd==-1) {
      free_fd(pid,fd);
      free_file(f);
      unlock_dentry(d);
      return_from_fs(-EMFILE);
    }
    free_fd(pid,fd);
    fd=newfd;
  }
  
  printk4("k_open: getting file descriptor... done");
  
  res=f->f_op->open(f->f_dentry->d_inode,f);
  if (res!=0) {
    free_fd(pid,fd);
    free_file(f);
    unlock_dentry(d);
    return_from_fs(res);
  }

  printk4("k_open: called proper filesystem open... done");

  printk4("k_open: END");
  return_from_fs(fd);  
}
