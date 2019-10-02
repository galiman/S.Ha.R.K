
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
#include <fs/maccess.h>
#include <fs/mount.h>

#include "fsconst.h"
#include "fs.h"

#include "debug.h"

int k_mount(__dev_t device,
	    __uint8_t fsind,
	    char *where,
	    struct mount_opts *options)
{
  declare_buffer(char,dname,MAXFILENAMELEN);  
  declare_buffer(struct mount_opts,opts,1);
  int len;
  int res;

  call_to_fs();
  
  len=__verify_read_nolen((char*)where);
  if ((!len)||(len>MAXFILENAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(dname,where,len+1);
  
  if (options!=NULL) {
    res=__verify_read(opts,sizeof(struct mount_opts));
    if (!res) return_from_fs(-EVERIFY);
    __copy_from_user(opts,options,len+1);
  
    return_from_fs(mount_device(device,fsind,dname,opts));
  }

  return_from_fs(mount_device(device,fsind,dname,NULL));
}

