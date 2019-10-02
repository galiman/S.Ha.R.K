
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
#include <fs/util.h>
#include <fs/maccess.h>
#include <fs/fcntl.h>

#include "file.h"
#include "dentry.h"
#include "fs.h"

/*
  this are from bits/fcntl.h
  if they are changed then this code must be rewritten!  
  #define F_DUPFD		0
  #define F_GETFD		1
  #define F_SETFD		2
  #define F_GETFL		3
  #define F_SETFL		4
  #define F_GETLK		5
  #define F_SETLK		6
  #define F_SETLKW	        7
 */

int dupfd(__pid_t pid, int fildes, void *ptr)
{
  int res;
  res=__duplicate_fd(pid,fildes,(int)ptr);
  unlock_desctable(pid);
  if (res) return -EMFILE;
  return EOK;
}

int getfd(__pid_t pid, int fildes, void *ptr)
{
  int res;
  res=*fildesflags_ptr(pid,fildes);
  unlock_desctable(pid);   
  return res;
}

int setfd(__pid_t pid, int fildes, void *ptr)
{
  *fildesflags_ptr(pid,fildes)=((int)ptr)&FD_CLOEXEC;
  unlock_desctable(pid);   
  return EOK;
}

int getfl(__pid_t pid, int fildes, void *ptr)
{
  int res;
  res=*fildesfflags_ptr(pid,fildes);
  unlock_desctable(pid);   
  return res;
}

int setfl(__pid_t pid, int fildes, void *ptr)
{
  *fildesfflags_ptr(pid,fildes)=((int)ptr)&
    (O_APPEND|O_DSYNC|O_NONBLOCK|O_RSYNC|O_SYNC);
  unlock_desctable(pid);   
  return EOK;
}

int dummy(__pid_t pid, int fildes, void *ptr)
{
  unlock_desctable(pid);   
  return -EINVAL;
}

static struct {
  int (*f)(__pid_t pid, int fildes, void *ptr);
} func[]={
  {dupfd},
  {getfd},
  {setfd},
  {getfl},
  {setfl},
  {dummy},
  {dummy},
  {dummy}
};

int k_fcntl(int fildes,int cmd,void *ptr)
{
  __pid_t pid;

  call_to_fs();
  
  pid=__get_pid();  
  lock_desctable(pid);
  if (!__isvalid_fd(pid,fildes)) {
    lock_desctable(pid);
    return_from_fs(-EBADF);
  }

  if (cmd<F_DUPFD||cmd>=F_SETLKW)
    return_from_fs(-ENOSYS);

  return_from_fs(func[cmd].f(pid,fildes,ptr));  
}
