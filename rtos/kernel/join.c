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
 CVS :        $Id: join.c,v 1.4 2003/11/05 15:05:11 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2003/11/05 15:05:11 $
 ------------

 task join and related primitives

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

static int taskjoin_once = 1;


/* this is the test that is done when a task is being killed
   and it is waiting on a sigwait */
static int taskjoin_cancellation_point(PID i, void *arg)
{
    LEVEL l;

    if (proc_table[i].status == WAIT_JOIN) {
      /* the task that have to be killed is waiting on a task_join.
         we reset the data structures set in task_join and then when the
         task will exit from task_join it will fall into a task_testcancel */
      proc_table[ proc_table[i].shadow ].waiting_for_me = NIL;
      proc_table[i].shadow = i;

      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    return 0;
}


/*+ this function suspends execution of the calling task until the target
    task terminates, unless the target task has already terminated.
    It works like the pthread_join +*/
int task_join(PID p, void **value)
{
  PID x;           /* used to follow the shadow chain */
  int blocked = 0; /* a flag */
  LEVEL l;

  /* task_join is a cancellation point... if the task is suspended
     the control on the status is done in task_kill */
  task_testcancel();

  /* some controls on the task p */
  if (p<0 || p>=MAX_PROC)                       return ESRCH;
  if (proc_table[p].status == FREE &&
      !(proc_table[p].control & WAIT_FOR_JOIN)) return ESRCH;
  if (!(proc_table[p].control & TASK_JOINABLE)) return EINVAL;

  proc_table[exec_shadow].context = kern_context_save();

  /* first, if it is the first time that the task_join is called,
     register the cancellation point */
  if (taskjoin_once) {
    taskjoin_once = 0;
    register_cancellation_point(taskjoin_cancellation_point, NULL);
  }

  if (proc_table[p].waiting_for_me != NIL) {
    kern_context_load(proc_table[exec_shadow].context);
    return EINVAL;
  }

  /* deadlock checks; we check the shadow chain to prevent cycles */
  x = p;
  do {
    x = proc_table[x].shadow;
    if (x == exec_shadow) {
      kern_context_load(proc_table[exec_shadow].context);
      return EDEADLK;
    }
  } while (x != proc_table[x].shadow);

  /* we prepare all the stuffs for joining the target task... */
  if (!(proc_table[p].control & WAIT_FOR_JOIN)) {
    /* the target task is already running... so we block yhe current task
       on it!!!

       Note: It is not correct to set only the shadow and reschedule, as done
       with the mutexes, because there is no inheritance with join...
       Normally we have to call task_extract because blocking on join is
       like blocking on a classic semaphore.
       Althought, we set the shadow because:
       - if the task call task_join when holding a mutex (AAARRRGGHHH!!!)
         the system continue working...
       - The deadlock detection strategy works on shadows...
       Setting shadows means also that implementation of mutexes that
       manage shadows in a strange way WILL NOT WORK with task_join
       (for example, the srp.c module doesn't work with task_join) */
    proc_table[p].waiting_for_me = exec_shadow;
    proc_table[exec_shadow].shadow = p;

    kern_epilogue_macro();

    /* now, we block the current task, waiting the end of the target task */
    l = proc_table[exec_shadow].task_level;
    level_table[l]->public_block(l,exec_shadow);
    proc_table[exec_shadow].status = WAIT_JOIN;

    exec = exec_shadow = -1;
    scheduler();
    /* note that we don't use kern_context_load because when rescheduled
       we remain in kernel mode... */
    ll_context_to(proc_table[exec_shadow].context);

    blocked = 1;
  }
  /* task-join is a cancellation point; if the task is killed while it is
     waiting on a join, the task-kill set the kill-request bit and wake up
     the task, so it can die :-) */
  task_testcancel();

  /* the target task is terminated... we reset the WAIT_FOR_JOIN flag
     so the descriptor can be reused by task_create; if the descriptor was
     already discarded by the task_create, we reinsert it into the free
     queue */
  proc_table[p].control &= ~WAIT_FOR_JOIN;
  if (proc_table[p].control & DESCRIPTOR_DISCARDED)
    iq_insertfirst(p, &freedesc);

  if (value)
    *value = proc_table[p].return_value;

  if (blocked) {
    /* the ll_context_to is already done... */
    kern_deliver_pending_signals();
    sti();
  }
  else
    /* we did a kern_context_save before... */
    kern_context_load(proc_table[exec_shadow].context);

  return 0;
}

/*+ this function set the detach state of a task to joinable. This function
    is not present in Posix standard...
    returns ESRCH if p is non correct +*/
int task_joinable(PID p)
{
  SYS_FLAGS f;

  if (p<0 || p>=MAX_PROC)                       return ESRCH;
  if (proc_table[p].status == FREE)             return ESRCH;

  f = kern_fsave();
  proc_table[p].control |= TASK_JOINABLE;
  kern_frestore(f);
  return 0;
}

/*+ this function set the detach state of a task to detached. This function
    works as the posix's pthread_detach
    returns EINVAL if p can't be joined (or currently a task has done a
    join on it (condition not provided in posix)
    ESRCH if p is not correct +*/
int task_unjoinable(PID p)
{
  SYS_FLAGS f;

  if (p<0 || p>=MAX_PROC)                       return ESRCH;
  if (proc_table[p].status == FREE)             return ESRCH;

  f = kern_fsave();

  if (!(proc_table[p].control & TASK_JOINABLE) ||
      proc_table[p].waiting_for_me != NIL) {
    kern_frestore(f);
    return EINVAL;
  }

  proc_table[p].control |= TASK_JOINABLE;

  kern_frestore(f);
  return 0;
}
