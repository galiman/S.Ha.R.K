
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

#include "file.h"
#include "inode.h"
#include "dentry.h"
#include "fileop.h"
#include "fs.h"

#include "debug.h"

int k_close(int fd)
{
  struct file *f;
  __pid_t pid;

  call_to_fs();
  
  /* don't try to use a bad file descriptor */
  pid=__get_pid();
  if (check_fd(pid,fd)) {
    printk(KERN_INFO "k_close: using bad file descriptor");
    return_from_fs(-EBADF);
  }

  /* don't try to close a directory */
  if (file_ptr(pid,fd)->f_flag_isdir) return_from_fs(-EBADF);
  
  f=free_fd(pid,fd);
  if (f==NULL) return_from_fs(-EBADF);

  f->f_op->close(f->f_dentry->d_inode,f);
  unlock_dentry(f->f_dentry);
  
  free_file(f);
  return_from_fs(EOK);
}
