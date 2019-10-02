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
 CVS :        $Id: time.h,v 1.2 2004/06/21 16:50:37 anton Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2004/06/21 16:50:37 $
 ------------

 time.h

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

/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

#ifndef __include_time_h_
#define __include_time_h_

#ifdef __cplusplus
extern "C" {
#endif

/* timespec defined in ll/sys/ll/time.h */
#include <ll/time.h>
#include <signal.h>


#include <sys/htypes.h>

// never used, required by posix
#define CLK_TCK 100

__DJ_clock_t
#undef __DJ_clock_t
#define __DJ_clock_t

__DJ_clockid_t
#undef __DJ_clockid_t
#define __DJ_clockid_t

__DJ_time_t
#undef __DJ_time_t
#define __DJ_time_t

__DJ_timer_t
#undef __DJ_timer_t
#define __DJ_timer_t

struct itimerspec {
  struct timespec it_interval;
  struct timespec it_value;
};

/*Note that the CLOCK_REALTIME doesn't start from the epoch!!!*/
#define CLOCK_REALTIME 0

#define TIMER_ABSTIME 1

int clock_settime(clockid_t clock_id, const struct timespec *tp);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_getres(clockid_t clock_id, struct timespec *res);

int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_settime(timer_t timerid, int flags, const struct itimerspec *value,
                  struct itimerspec *ovalue);
int timer_gettime(timer_t timerid, struct itimerspec *value);
int timer_getoverrun(timer_t timerid);

// look at nanoslp.c
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
unsigned int sleep(unsigned int seconds);
unsigned int usleep(unsigned int usec);

void TIMER_register_module();


#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_time_h_ */
