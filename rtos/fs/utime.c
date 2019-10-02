
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

#include <bits/utime.h>
#include <fs/types.h>
#include <fs/errno.h>
#include <fs/task.h>
#include <fs/util.h>
#include <fs/maccess.h>


#include "file.h"
#include "dentry.h"
#include "inode.h"
#include "rwlock.h"
#include "fs.h"

/*
struct utimbuf {
  __time_t acctime;
  __time_t modtime;
};
*/

int k_utime(char *path, struct utimbuf *times)
{
  declare_buffer(char,pathname,MAXPATHNAME);
  declare_buffer(struct utimbuf,t,1);
  int createflag=DENTRY_NOCREATE;
  struct dentry *d;
  __pid_t pid;
  int len;

  call_to_fs();
  
  len=__verify_read_nolen((char*)path);
  if ((!len)||(len>MAXPATHNAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(pathname,path,len+1);
  init_buffer(t);
  if (times!=NULL) {
    len=__verify_read((char*)times,sizeof(struct utimbuf));
    if (!len) return_from_fs(-EPERM);
    __copy_from_user(t,times,len);
  } 
  
  pid=__get_pid();  
  lock_desctable(pid);

  d=find_dentry_from_ext(cwden_ptr(pid),(char *)pathname,&createflag);
  if (d==NULL) {
    unlock_desctable(pid);
    return_from_fs(-ENOENT);
  }
  
  __rwlock_wrlock(&d->d_inode->i_lock);
  if (times==NULL) {
    d->d_inode->i_st.st_atime=d->d_inode->i_st.st_mtime=gettimek();
  } else {
    d->d_inode->i_st.st_atime=t->actime;
    d->d_inode->i_st.st_mtime=t->modtime;
  }  
  d->d_inode->i_dirty=1;
  __rwlock_wrunlock(&d->d_inode->i_lock);
    
  unlock_dentry(d);
  unlock_desctable(pid);
  return_from_fs(EOK);
}
