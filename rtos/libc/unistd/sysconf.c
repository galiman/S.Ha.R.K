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
 CVS :        $Id: sysconf.c,v 1.1.1.1 2002/03/29 14:12:53 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1.1.1 $
 Last update: $Date: 2002/03/29 14:12:53 $
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


#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

long sysconf(int name)
{
  switch (name) {
    case _SC_AIO_LISTIO_MAX               : return AIO_LISTIO_MAX;
    case _SC_AIO_MAX                      : return AIO_MAX;
    case _SC_AIO_PRIO_DELTA_MAX           : return AIO_PRIO_DELTA_MAX;
    case _SC_ARG_MAX                      : return ARG_MAX;
    case _SC_CHILD_MAX                    : return -1; // !!!
    case _SC_CLK_TICK                     : return CLK_TCK;
    case _SC_DELAYTIMER_MAX               : return DELAYTIMER_MAX;
    case _SC_GETGR_R_SIZE_MAX             : return -1; // !!!
    case _SC_GETPW_R_SIZE_MAX             : return -1; // !!!
    case _SC_LOGIN_NAME_MAX               : return LOGIN_NAME_MAX;
    case _SC_MQ_OPEN_MAX                  : return MQ_OPEN_MAX;
    case _SC_MQ_PRIO_MAX                  : return MQ_PRIO_MAX;
    case _SC_NGROUPS_MAX                  : return NGROUPS_MAX;
    case _SC_OPEN_MAX                     : return OPEN_MAX;
    case _SC_PAGESIZE                     : return PAGESIZE;
    case _SC_RTSIG_MAX                    : return RTSIG_MAX;
    case _SC_SEM_NSEMS_MAX                : return SEM_NSEMS_MAX;
    case _SC_SEM_VALUE_MAX                : return SEM_VALUE_MAX;
    case _SC_SIGQUEUE_MAX                 : return SIGQUEUE_MAX;
    case _SC_STREAM_MAX                   : return STREAM_MAX;
    case _SC_THREAD_DESTRUCTOR_ITERATIONS : return PTHREAD_DESTRUCTOR_ITERATIONS;
    case _SC_THREAD_KEYS_MAX              : return PTHREAD_KEYS_MAX;
    case _SC_THREAD_STACK_MIN             : return PTHREAD_STACK_MIN;
    case _SC_THREAD_THREADS_MAX           : return PTHREAD_THREADS_MAX;
    case _SC_TIMER_MAX                    : return TIMER_MAX;
    case _SC_TTY_NAME_MAX                 : return TTY_NAME_MAX;
    case _SC_TZNAME_MAX                   : return TZNAME_MAX;

    case _SC_ASYNCHRONOUS_IO              : return -1;
    case _SC_FSYNC                        : return -1;
    case _SC_JOB_CONTROL                  : return -1;
    case _SC_MAPPED_FILES                 : return -1;
    case _SC_MEMLOCK                      : return -1;
    case _SC_MEMLOCK_RANGE                : return -1;
    case _SC_MEMORY_PROTECTION            : return -1;
    case _SC_MESSAGE_PASSING              : return 0;
    case _SC_PRIORITIZED_IO               : return -1;
    case _SC_PRIORITY_SCHEDULING          : return -1;
    case _SC_REALTIME_SIGNALS             : return 0;
    case _SC_SAVED_IDS                    : return -1;
    case _SC_SEMAPHORES                   : return 0;
    case _SC_SHARED_MEMORY_OBJECTS        : return -1;
    case _SC_SYNCHRONIZED_IO              : return -1;
    case _SC_TIMERS                       : return 0;
    case _SC_THREADS                      : return 0;
    case _SC_THREAD_ATTR_STACKADDR        : return 0;
    case _SC_THREAD_ATTR_STACKSIZE        : return 0;
    case _SC_THREAD_PRIORITY_SCHEDULING   : return 0;
    case _SC_THREAD_PRIO_INHERIT          : return 0;
    case _SC_THREAD_PRIO_PROTECT          : return 0;
    case _SC_THREAD_PROCESS_SHARED        : return -1;
    case _SC_THREAD_SAFE_FUNCTIONS        : return -1;
    case _SC_VERSION                      : return _POSIX_VERSION;

    default                               : errno = EINVAL; return -1;
  }
}
