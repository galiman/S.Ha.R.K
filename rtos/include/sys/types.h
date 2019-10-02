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


/**
 ------------
 CVS :        $Id: types.h,v 1.3 2003/03/17 09:18:55 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2003/03/17 09:18:55 $
 ------------

**/

/*
 * Copyright (C) 2000 Paolo Gai
 *
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

/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <features.h>
#include <kernel/types.h>
#include <kernel/descr.h>

#include <bits/types.h>
#include <sys/htypes.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS
  
__DJ_dev_t
#undef __DJ_dev_t
#define __DJ_dev_t

__DJ_ino_t
#undef __DJ_ino_t
#define __DJ_ino_t

__DJ_mode_t
#undef __DJ_mode_t
#define __DJ_mode_t

__DJ_nlink_t
#undef __DJ_nlink_t
#define __DJ_nlink_t

__DJ_gid_t
#undef __DJ_gid_t
#define __DJ_gid_t

__DJ_off_t
#undef __DJ_off_t
#define __DJ_off_t

__DJ_pid_t
#undef __DJ_pid_t
#define __DJ_pid_t

__DJ_uid_t
#undef __DJ_uid_t
#define __DJ_uid_t

__END_DECLS

// for sched_param
#include <sched.h>

__BEGIN_DECLS

typedef struct {
  // This fields are specified by the posix standard, 13.4.1
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;

  // This are in 16.2.1.2
  size_t stacksize;
  void *stackaddr;
  int detachstate;

} pthread_attr_t;

typedef struct {
  // This are in 13.6.1.2
  int protocol;
  int prioceiling;
} pthread_mutexattr_t;

typedef mutex_t pthread_mutex_t;

typedef int pthread_once_t;

typedef PID pthread_t;

typedef task_key_t pthread_key_t;

typedef int pthread_condattr_t;

typedef cond_t pthread_cond_t;

#ifndef _POSIX_SOURCE

/* Allow including program to override.  */
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

typedef struct fd_set {
  unsigned char fd_bits [((FD_SETSIZE) + 7) / 8];
} fd_set;

#define FD_SET(n, p)    ((p)->fd_bits[(n) / 8] |= (1 << ((n) & 7)))
#define FD_CLR(n, p)	((p)->fd_bits[(n) / 8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n, p)	((p)->fd_bits[(n) / 8] & (1 << ((n) & 7)))
#define FD_ZERO(p)	memset ((void *)(p), 0, sizeof (*(p)))

#endif /* !_POSIX_SOURCE */

/* -- */

typedef __u_quad_t u_quad_t;
typedef __quad_t   quad_t;

__END_DECLS
#endif /* !__dj_include_sys_types_h_ */

