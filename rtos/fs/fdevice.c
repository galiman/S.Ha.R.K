
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
#include <fs/bdev.h>

#include "fsconst.h"
#include "debug.h"
#include "fs.h"

int k_fdevice(char *device_name)
{
  declare_buffer(char,dname,MAXFILENAMELEN);
  int len,ret;

  call_to_fs();
  
  len=__verify_read_nolen((char*)device_name);
  if ((!len)||(len>MAXFILENAMELEN)) return_from_fs(-EVERIFY);
  __copy_from_user(dname,device_name,len+1);

  ret=(int)bdev_find_byname(dname);
  if (ret<0) return_from_fs(-EINVAL);
  
  return_from_fs(ret);
}

