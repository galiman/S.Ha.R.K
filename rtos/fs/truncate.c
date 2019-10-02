
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
#include <fs/fcntl.h>

#include "file.h"
#include "inode.h"
#include "dentry.h"
#include "fileop.h"
#include "dentryop.h"
#include "inodeop.h"
#include "fsconst.h"
#include "fs.h"

int k_ftruncate(int fd, __off_t len)
{
  struct inode *in;
  int res;
  __pid_t pid;

  call_to_fs();
  
  /* don't try to use a bad file descriptor */
  pid=__get_pid();
  if (check_fd(pid,fd)) {
    printk(KERN_INFO "k_ftruncate: using bad file descriptor");
    return_from_fs(-EBADF);
  }

  /* don't try to write a directory */
  if (file_ptr(pid,fd)->f_flag_isdir) return_from_fs(-EBADF);

  /* if in read only mode (errno OK?)*/
  if (*fildesfflags_ptr(pid,fd)&O_RDONLY)
    return_from_fs(-EPERM);

  /* acquire write lock on inode's file */
  in=file_ptr(pid,fd)->f_dentry->d_inode;
  __rwlock_wrlock(&in->i_lock);

  /*
  if (*fildesfflags_ptr(pid,fd)&O_APPEND)
    if (file_ptr(pid,fd)->f_pos!=
	file_ptr(pid,fd)->f_dentry->d_inode->i_st.st_size) {
      __off_t off;
      off=file_ptr(pid,fd)->f_op->lseek(file_ptr(pid,fd),0,SEEK_END);
      if (off!=file_ptr(pid,fd)->f_dentry->d_inode->i_st.st_size) {
	sz=-ESPIPE;
	goto SKIP;
      }
    }
    */

  /* truncate */
  res=in->i_op->truncate(in,len);
 
  /* release the lock */
  __rwlock_wrunlock(&in->i_lock);

  return_from_fs(res);
}
