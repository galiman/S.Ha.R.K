
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

#include "fsconst.h"
#include "file.h"
#include "inode.h"
#include "fileop.h"
#include "dentry.h"
#include "fs.h"

#include "debug.h"

int k_stat(char *upathname, struct stat *buf)
{
  declare_buffer(char,pathname,MAXPATHNAME);
  struct dentry *d;
  const __pid_t pid=__get_pid();
  int len;

  call_to_fs();
  
  printk6("k_stat: START");
  
  len=__verify_read_nolen((char*)upathname);
  if ((!len)||(len>MAXPATHNAMELEN)) {
    printk6("k_stat: verify user memory failed");
    printk6("k_stat: END");
    return_from_fs(-EVERIFY);
  }
  __copy_from_user(pathname,upathname,len+1);

  printk6("k_stat: for '%s'",pathname);

  d=find_dentry_from(cwden_ptr(pid),(char *)pathname);
  if (d==NULL) {
    printk6("k_stat: not found!");
    printk6("k_stat: END");
    return_from_fs(-ENOENT);
  }
  printk6("k_stat: found... copying to user");

  if (!__verify_write(buf,sizeof(struct stat))) return_from_fs(-EVERIFY);
  __copy_to_user(buf,&(d->d_inode->i_st),sizeof(struct stat));

  unlock_dentry(d);

  printk6("k_stat: END");
  return_from_fs(EOK);
}
