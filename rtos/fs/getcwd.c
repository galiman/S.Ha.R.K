
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
#include "fs.h"

int k_getcwd(char *buf, size_t sz)
{
  char *cwd;
  int len;
  __pid_t pid;

  call_to_fs();
  
  pid=__get_pid();  
  lock_desctable(pid);
  cwd=cwd_ptr(pid);
  if (buf==NULL||strlen(cwd)>=sz) {
    unlock_desctable(pid);
    return_from_fs(-ERANGE);
  }
  len=strlen(cwd)+1;
  if (!__verify_write(buf,len)) {
    unlock_desctable(pid);
    return_from_fs(-EVERIFY);
  }
  __copy_to_user(buf,cwd,len);
  unlock_desctable(pid);
  return_from_fs(EOK);
}
