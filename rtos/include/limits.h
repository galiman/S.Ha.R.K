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
 CVS :        $Id: limits.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1.1.1 $
 Last update: $Date: 2002/03/29 14:12:51 $
 ------------

 limits.h

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
#ifndef __LIMITS_H__
#define __LIMITS_H__

#include <endian.h>
#include <bits/limits.h>
#include <kernel/const.h>

// table 2-3
#define _POSIX_AIO_LISTIO_MAX                   2
#define _POSIX_AIO_MAX                          1
#define _POSIX_ARG_MAX                          4096
#define _POSIX_CHILD_MAX                        6
#define _POSIX_DELAYTIMER_MAX                   32
#define _POSIX_LINK_MAX                         1
#define _POSIX_LOGIN_NAME_MAX                   9
#define _POSIX_MAX_CANON                        255
#define _POSIX_MAX_INPUT                        255
#define _POSIX_MQ_OPEN_MAX                      8
#define _POSIX_MQ_PRIO_MAX                      32
#define _POSIX_NAME_MAX                         FILENAME_MAX
#define _POSIX_NGROUPS_MAX                      0
#define _POSIX_OPEN_MAX                         MAXOPENFILES
#define _POSIX_PATH_MAX                         MAXPATHNAMELEN
#define _POSIX_PIPE_BUF                         512
#define _POSIX_RTSIG_MAX                        8

// table 5-2
#define _POSIX_ASYNC_IO                         0
#define _POSIX_CHOWN_RESTRICTED                 0
#define _POSIX_NO_TRUNC                         1 
#define _POSIX_PRIO_IO                          0 
#define _POSIX_SYNC_IO                          0
#define _POSIX_VDISABLE                         0

// must be equal to MAX_SEM defined in modules/hartsem.h
#define _POSIX_SEM_NSEMS_MAX                    256
#define _POSIX_SEM_VALUE_MAX                    32767
#define _POSIX_SIGQUEUE_MAX                     32
#define _POSIX_SSIZE_MAX                        32767
#define _POSIX_STREAM_MAX                       8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS     4
#define _POSIX_THREAD_KEYS_MAX                  128
#define _POSIX_THREAD_THREADS_MAX               64
#define _POSIX_TTY_NAME_MAX                     9
#define _POSIX_TIMER_MAX                        32
#define _POSIX_TZNAME_MAX                       3

// table 2-4
#define NGROUPS_MAX                             _POSIX_NGROUPS_MAX

// table 2-5
#define AIO_LISTIO_MAX                          _POSIX_AIO_LISTIO_MAX
#define AIO_MAX                                 _POSIX_AIO_MAX
#define AIO_PRIO_DELTA_MAX                      0
#define ARG_MAX                                 _POSIX_ARG_MAX
#define CHILD_MAX                               _POSIX_CHILD_MAX
#define DELAYTIMER_MAX                          _POSIX_DELAYTIMER_MAX
#define LOGIN_NAME_MAX                          _POSIX_LOGIN_NAME_MAX
#define MQ_OPEN_MAX                             _POSIX_MQ_OPEN_MAX
#define MQ_PRIO_MAX                             _POSIX_MQ_PRIO_MAX
#ifndef OPEN_MAX
#define OPEN_MAX                                _POSIX_OPEN_MAX
#endif
#define PAGESIZE                                1
#define PTHREAD_DESTRUCTOR_ITERATIONS           _POSIX_THREAD_DESTRUCTOR_ITERATIONS
#define PTHREAD_KEYS_MAX                        _POSIX_THREAD_KEYS_MAX
#define PTHREAD_STACK_MIN                       STACK_SIZE
#define PTHREAD_THREADS_MAX                     MAX_PROC
#define RTSIG_MAX                               22UL
#define SEM_NSEMS_MAX                           _POSIX_SEM_NSEMS_MAX
#define SEM_VALUE_MAX                           _POSIX_SEM_VALUE_MAX
#define SIGQUEUE_MAX                            64
#define STREAM_MAX                              _POSIX_STREAM_MAX
#define TIMER_MAX                               _POSIX_TIMER_MAX
#define TTY_NAME_MAX                            _POSIX_TTY_NAME_MAX
#define TZNAME_MAX                              _POSIX_TZNAME_MAX

// table 2-6
#define LINK_MAX                                _POSIX_LINK_MAX
#define MAX_CANON                               _POSIX_MAX_CANON
#define MAX_INPUT                               _POSIX_MAX_INPUT
#ifndef NAME_MAX
#define NAME_MAX                                _POSIX_NAME_MAX
#endif
#ifndef PATH_MAX
#define PATH_MAX                                _POSIX_PATH_MAX
#endif
#define PIPE_BUF                                _POSIX_PIPE_BUF

// table 2-7
#define SSIZE_MAX                               _POSIX_SSIZE_MAX

// table 2-7a
#define _POSIX_CLOCKRES_MIN                     20000000L

#endif
