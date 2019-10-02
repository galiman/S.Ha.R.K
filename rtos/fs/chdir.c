
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

#include "file.h"
#include "dentry.h"
#include "fs.h"

int k_chdir(char *newpath)
{
  declare_buffer(char,pathname,MAXPATHNAME);  
  int createflag=DENTRY_NOCREATE;
  struct dentry *d;
  __pid_t pid;
  int len;

  call_to_fs();
  
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

  unlock_dentry(cwden_ptr(pid));
  /* dovrebbe essere: cwden_ptr(pid)=d; */
  desctable[pid].fd_cwden=d;
  strcpy(cwd_ptr(pid),pathname);
  
  unlock_desctable(pid);
  return_from_fs(EOK);
}
