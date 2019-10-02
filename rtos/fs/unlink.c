
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
#include "super.h"
#include "fileop.h"
#include "dentryop.h"
#include "inodeop.h"
#include "fsconst.h"
#include "fs.h"

#include "debug.h"

int k_unlink(char *upathname)
{
  declare_buffer(char,pathname,MAXPATHNAME);
  struct dentry *d;
  int createflag;
  __pid_t pid;
  int len,res;

  call_to_fs();

  printkd("k_unlink: START");
  pid=__get_pid();

  len=__verify_read_nolen((char*)upathname);
  if ((!len)||(len>MAXPATHNAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(pathname,upathname,len+1);

  printkd("k_unlink: pathname copied");

  createflag=DENTRY_NOCREATE;
  d=find_dentry_from_ext(cwden_ptr(pid),(char *)pathname,&createflag);

  printkd("k_unlink: dentry found");

  if (d==NULL) return_from_fs(-ENOENT);
  if (__S_ISDIR(d->d_inode->i_st.st_mode)) return_from_fs(-EPERM);
  if (!(d->d_sb->sb_mopts.flags&MOUNT_FLAG_RW)) return_from_fs(-EROFS);

  printkd("k_unlink: dentry is ok");
  
  res=unlink_dentry(d);

  printkd("k_unlink: dentry unlinked");

  printkd("k_unlink: END");
  return_from_fs(res);
}
