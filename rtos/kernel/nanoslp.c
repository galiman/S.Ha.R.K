/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
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
 CVS :        $Id: nanoslp.c,v 1.4 2003/05/22 14:10:05 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2003/05/22 14:10:05 $
 ------------

 This file contains the nanosleep function (posix 14.2.5) and related
 functions

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

#include <stdarg.h>
#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>

static int nanosleep_once = 1;

/* this table is used to store the wakeup time of the task,
   because if the rmtp argument is specified, the remaining sleep time
   have to be returned. */
struct timespec nanosleep_table[MAX_PROC];


/* this is the test that is done when a task is being killed
   and it is waiting on a sigwait */
static int nanosleep_cancellation_point(PID i, void *arg)
{
    LEVEL l;

    if (proc_table[i].status == WAIT_NANOSLEEP) {
      /* the task that have to be killed is waiting on a nanosleep */

      /* the nanosleep event have to be removed */
      kern_event_delete(proc_table[i].delay_timer);
      proc_table[i].delay_timer = -1;

      /* and the task have to be reinserted into the ready queues, so it
         will fall into task_testcancel */
      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    return 0;
}

int nanosleep_interrupted_by_signal(PID i, void *arg)
{
    LEVEL l;
    struct timespec t1, t2;

    if (proc_table[i].status == WAIT_NANOSLEEP) {
      /* the task is waiting on a nanosleep and it is still receiving a
         signal... */
      kern_gettime(&t1);
      SUBTIMESPEC(&nanosleep_table[i], &t1, &t2);
      TIMESPEC_ASSIGN(&nanosleep_table[i], &t2);

      /* the nanosleep event have to be removed */
      kern_event_delete(proc_table[i].delay_timer);
      proc_table[i].delay_timer = -1;

      /* and the task have to be reinserted into the ready queues, so it
         will fall into task_testcancel */
      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    return 0;
}

static void nanosleep_timer(void *arg)
{
  PID p = (PID)arg;
  LEVEL l;

  NULL_TIMESPEC(&nanosleep_table[p]);

  proc_table[p].delay_timer = -1;

  l = proc_table[p].task_level;
  level_table[l]->public_unblock(l,p);

  event_need_reschedule();
}


int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
  LEVEL l;

  if (rqtp->tv_sec < 0 || rqtp->tv_nsec > 1000000000)
    return EINVAL;

  proc_table[exec_shadow].context = kern_context_save();

  /* first, if it is the first time that the nanosleep is called,
     register the cancellation point */
  if (nanosleep_once) {
    nanosleep_once = 0;
    register_cancellation_point(nanosleep_cancellation_point, NULL);
    register_interruptable_point(nanosleep_interrupted_by_signal, NULL);
  }

  kern_epilogue_macro();

  /* now, we block the current task, waiting the end of the target task */
  l = proc_table[exec_shadow].task_level;
  level_table[l]->public_block(l,exec_shadow);
  proc_table[exec_shadow].status = WAIT_NANOSLEEP;

  ADDTIMESPEC(&schedule_time, rqtp, &nanosleep_table[exec_shadow]);

  /* we can use the delaytimer because if we are here we are not in a
     task_delay */
  proc_table[exec_shadow].delay_timer =
    kern_event_post(&nanosleep_table[exec_shadow],
                    nanosleep_timer,
                    (void *)exec_shadow);

  exec = exec_shadow = -1;
  scheduler();
  kern_context_load(proc_table[exec_shadow].context);

  task_testcancel();

  /* if the nanosleep_table[exec_shadow] != {0,0}, the nanosleep was
     interrupted by a signal... */
  if (nanosleep_table[exec_shadow].tv_sec  != 0 ||
      nanosleep_table[exec_shadow].tv_nsec != 0    ) {
    if (rmtp)
      TIMESPEC_ASSIGN(rmtp, &nanosleep_table[exec_shadow]);

    return (EINTR);
  }
  else
    return 0;
}

unsigned int sleep(unsigned int seconds)
{
  struct timespec t, t2;

  t.tv_sec = seconds;
  t.tv_nsec = 0;
  nanosleep(&t, &t2);

  return t2.tv_sec;
}

unsigned int usleep(unsigned int usec)
{
  struct timespec t, t2;

  t.tv_sec = usec / 1000000;
  t.tv_nsec = usec % 1000000 * 1000;
  nanosleep(&t, &t2);

  return t2.tv_sec;
}
