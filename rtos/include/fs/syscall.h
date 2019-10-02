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


#ifndef __FS_SYSCALL_H
#define __FS_SYSCALL_H

#include <fs/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define SYS_CALL(fun)                  fun()
#define SYS_CALL1(fun,a1)              fun(a1)
#define SYS_CALL2(fun,a1,a2)           fun((a1),(a2))
#define SYS_CALL3(fun,a1,a2,a3)        fun((a1),(a2),(a3))
#define SYS_CALL4(fun,a1,a2,a3,a4)     fun((a1),(a2),(a3),(a4))
#define SYS_CALL5(fun,a1,a2,a3,a4,a5)  fun((a1),(a2),(a3),(a4),(a5))

__mode_t k_umask(__mode_t newmask);
__mode_t k_getumask(void);
int k_getcwd(char *buf, __ssize_t sz);

__ssize_t k_read(int fd, __ptr_t buf, __ssize_t nbytes);
__ssize_t k_write(int fd, __ptr_t buf, __ssize_t nbytes);
__off_t k_lseek(int fd, __off_t, int);
int k_close(int fd);

int k_ftruncate(int fildes, __off_t len);
int k_unlink(char *path);

__END_DECLS
#endif
