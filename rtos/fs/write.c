
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
#include "fsconst.h"
#include "fs.h"

/* NB: alcune operazioni sui files vanno fatte in mutua esclusione */

/* to FIX:
 * -- fd == 0,1,2 then we must write on teminal
 *    (must be integrated better)
 */
#include <ll/i386/cons.h>

__ssize_t k_write(int fd, __ptr_t buf, __ssize_t nbytes)
{
  __pid_t pid;
  __ssize_t sz;

  call_to_fs();
  
  /* TO FIX! */  
  if (fd>=0&&fd<=2) {
    __ssize_t x;
    char *c;    
  
    //printk(KERN_INFO "writing on console with fd=%i",fd);

    /* WARNING buf is a pointer into user-space
     * we are using it without cast
     */

    /* WARNING cputc is called without mutex
     * lock
     */
    
    if (fd==0) return_from_fs(-EBADF);
    
    c=(char*)buf;
    x=nbytes;    
    while (x-->0) cputc(*c++);
    
    return_from_fs(nbytes);
  }
  
  /* don't try to use a bad file descriptor */
  pid=__get_pid();
  if (check_fd(pid,fd)) {
    printk(KERN_INFO "k_write: using bad file descriptor");
    return_from_fs(-EBADF);
  }
  
  /* don't try to write a directory */
  if (file_ptr(pid,fd)->f_flag_isdir) return_from_fs(-EBADF);

  /* don't try to write from not user memory */
  if (!__verify_write(buf,nbytes)) return_from_fs(-EVERIFY);

  /* if in read only mode (errno OK?)*/
  if (*fildesfflags_ptr(pid,fd)&O_RDONLY)
    return_from_fs(-EPERM);

  /* cancellation point */
  //fs_test_cancellation(-EINTR);

  /* acquire write lock on inode's file */
  __rwlock_wrlock(&file_ptr(pid,fd)->f_dentry->d_inode->i_lock);

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
  
  /* WARNIG buf is a pointer into user-space */  
  sz=file_ptr(pid,fd)->f_op->write(file_ptr(pid,fd),buf,nbytes);

  /* release the lock */
SKIP:
  __rwlock_wrunlock(&file_ptr(pid,fd)->f_dentry->d_inode->i_lock);

  return_from_fs(sz);
}
