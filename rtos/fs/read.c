
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
#include "rwlock.h"
#include "fs.h"

#define EBOH EIO

/* temporary: for console i/o */
#include <ll/i386/cons.h>
static int __getch(int flag)
{

	return 0;

}

__ssize_t k_read(int fd, __ptr_t buf, __ssize_t nbytes)
{
  __pid_t pid;
  __ssize_t sz;

  call_to_fs();
  
  /* TO FIX! */  
  if (fd>=0&&fd<=2) {    
    __ssize_t x;
    int ret,ch;
    char *c;    
  
    //printk(KERN_INFO "reading from console");

    /* WARNIG buf is a pointer into user-space
     * we are using it without cast
     */

    /* WARNIG there is a blocking call...
     * the system can go down if it is blocked!
     */
    
    if (fd!=0) return_from_fs(-EBADF);

    c=(char*)buf;
    x=0;    
    while (x<nbytes) {
      ch=__getch(NON_BLOCK);
      if (ch!=-1) *c++=ch;
      else {
	if (x>0) break;
	release_sys_call();
	ch=__getch(BLOCK);
	ret=reacquire_sys_call();
	if (ret) return -ENOSYS;
	*c++=ch;
      }
      x++;
      cputc(ch);
    }
    
    return_from_fs(x);
  }
  
  /* don't try to use bad file descriptor */
  pid=__get_pid();
  if (check_fd(pid,fd)) {
    printk(KERN_INFO "k_read: using bad file descriptor");
    return_from_fs(-EBADF);
  }

  /* don't try to read a directory */
  if (file_ptr(pid,fd)->f_flag_isdir) return_from_fs(-EBADF);

  /* don't try to read into not user memory */
  if (!__verify_write(buf,nbytes)) return_from_fs(-EVERIFY);

  /* if in write only mode */
  if (*fildesfflags_ptr(pid,fd)&O_WRONLY)
    return_from_fs(-EBOH);

  /* cancellation point */
  //fs_test_cancellation(-EINTR);
  
  /* acquire read lock on inode's file */
  __rwlock_rdlock(&file_ptr(pid,fd)->f_dentry->d_inode->i_lock);

  /* O_APPEND flags */
  if (*fildesfflags_ptr(pid,fd)&O_APPEND) {
    sz=0;
    goto SKIP;
  }

  /* WARNING buf is a pointer into user-space! */
  sz=file_ptr(pid,fd)->f_op->read(file_ptr(pid,fd),buf,nbytes);
  
  /* release the lock */
SKIP:
  __rwlock_rdunlock(&file_ptr(pid,fd)->f_dentry->d_inode->i_lock);
  
  return_from_fs(sz);
}
