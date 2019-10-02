
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
#include <fs/stat.h>
#include <fs/task.h>

#include "file.h"
#include "fs.h"

__mode_t k_umask(__mode_t newmask)
{
  __mode_t oldmask;
  __pid_t pid;

  call_to_fs();
  
  pid=__get_pid();  
  lock_desctable(pid);
  oldmask=*umask_ptr(pid);
  *umask_ptr(pid)=newmask&__ACCESSPERMS;
  unlock_desctable(pid);
  return_from_fs(oldmask);
}
