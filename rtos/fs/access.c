
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

/* there is no permission test yet !!! */

int k_access(char *newpath, int amode)
{
  declare_buffer(char,pathname,MAXPATHNAME);  
  int createflag=DENTRY_NOCREATE;
  struct dentry *d;
  __pid_t pid;
  int len;

  call_to_fs();
  
  if (amode&~(R_OK|R_OK|X_OK|F_OK)) return_from_fs(-EINVAL);
    
  len=__verify_read_nolen((char*)newpath);
  if ((!len)||(len>MAXPATHNAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(pathname,newpath,len+1);

  pid=__get_pid();  
  lock_desctable(pid);

  d=find_dentry_from_ext(cwden_ptr(pid),(char *)pathname,&createflag);
  if (d==NULL) {
    unlock_desctable(pid);
    return_from_fs(-ENOENT);
  }
  unlock_dentry(d);
  
  unlock_desctable(pid);
  
  if (amode&X_OK) return_from_fs(-EACCES);
  return_from_fs(EOK);
}
