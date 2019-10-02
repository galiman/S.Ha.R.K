
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
#include <fs/major.h>

#include "file.h"
#include "inode.h"
#include "dentry.h"
#include "fs.h"

#include "debug.h"

int k_fstat(int fd, struct stat *buf)
{
  struct inode *in=NULL;
  __pid_t pid;

  call_to_fs();
  
  /* to do better: for console i/o */
  if (fd>=0&&fd<=2) {
    struct stat st;

    /* !!! */
    return_from_fs(-EBADF);
    
    st.st_dev=NULLDEV;
    st.st_ino=0;
    st.st_mode=((fd==0)?
		__S_IRUSR|__S_IRGRP|__S_IROTH:
		__S_IWUSR|__S_IWGRP|__S_IWOTH);
    st.st_nlink=1;
    st.st_uid=0;
    st.st_gid=0;
    st.st_size=0;
    st.st_atime=0;
    st.st_mtime=0;
    st.st_ctime=0;
      
    if (!__verify_write(buf,sizeof(struct stat))) return_from_fs(-EVERIFY);
    __copy_to_user(buf,&(in->i_st),sizeof(struct stat));
    return_from_fs(EOK);
  }
  
  /* don't try to use bad file descriptor */
  pid=__get_pid();
  if (check_fd(pid,fd)) {
    printk(KERN_INFO "k_fstat: using a bad file descriptor");
    return_from_fs(-EBADF);
  }

  if (!__verify_write(buf,sizeof(struct stat))) return_from_fs(-EVERIFY);

  in=file_ptr(pid,fd)->f_dentry->d_inode;
  __copy_to_user(buf,&(in->i_st),sizeof(struct stat));
  
  return_from_fs(EOK);
}
