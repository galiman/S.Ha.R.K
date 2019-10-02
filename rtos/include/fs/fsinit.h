/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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


#ifndef __FS_FSINIT_H__
#define __FS_FSINIT_H__

#include <kernel/model.h>

#include <fs/types.h>
#include <fs/const.h>
#include <fs/sysmacro.h>
#include <fs/major.h>
#include <fs/fsind.h>
#include <fs/mount.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*
 * file systems params structure
 */

extern NPP_mutexattr_t fsdef_mutexattr;

typedef struct filesystem_parms {
  __dev_t           device;  /* root device (default: /dev/hda1) */
  __uint8_t         fs_ind;  /* root device filesystem (default: FS_DEFAULT) */
  __uint32_t        fs_showinfo:1; /* show info on startup */
  void              *fs_mutexattr;
  struct mount_opts *fs_opts;  
} FILESYSTEM_PARMS;

#define BASE_FILESYSTEM {makedev(MAJOR_B_IDE,1),FS_DEFAULT,0, \
    &fsdef_mutexattr,NULL}

#define filesystem_def_rootdevice(par,rootdev) ((par).device=(rootdev))
#define filesystem_def_fs(par,fs) ((par).fs_ind=(fs))
#define filesystem_def_showinfo(par,show) ((par).fs_showinfo=(show))
#define filesystem_def_options(par,opts) ((par).fs_opts=&(opts))

/**/

int filesystem_init(FILESYSTEM_PARMS *);

#ifdef MSDOS_FILESYSTEM
int msdos_fs_init(FILESYSTEM_PARMS *);
#endif

__END_DECLS
#endif
