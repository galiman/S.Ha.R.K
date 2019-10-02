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
 CVS :        $Id: conditio.c,v 1.5 2003/11/06 08:55:53 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2003/11/06 08:55:53 $
 ------------

This file contains the condition variables handling functions.

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



#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <errno.h>
#include <kernel/iqueue.h>

/*---------------------------------------------------------------------*/
/* Condition variables                                                 */
/*---------------------------------------------------------------------*/

static int condition_once = 1;

/* this is the test that is done when a task is being killed
   and it is waiting on a condition */
static int condition_cancellation_point(PID i, void *arg)
{
    LEVEL l;

    if (proc_table[i].status == WAIT_COND) {
    /* if the task is waiting on a condition variable, we have to extract it
       from the waiters queue, then set the KILL_REQUEST flag, and reinsert
       the task into the ready queue so it can reaquire the mutex and die */
      iq_extract(i,&proc_table[i].cond_waiting->waiters);
      if (iq_isempty(&proc_table[i].cond_waiting->waiters))
        proc_table[i].cond_waiting->used_for_waiting = NULL;
      proc_table[i].cond_waiting = NULL;

      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);
      /* then, the kill_request flag is set, and when the task is rescheduled
         it autokill itself... */

      return 1;
    }

    return 0;
}


int cond_init(cond_t *cond)
{
  /* first, if it is the first time that the cond_init is called,
     register the cancellation point */
  if (condition_once) {
    condition_once = 0;
    register_cancellation_point(condition_cancellation_point, NULL);
  }

  iq_init (&cond->waiters, &freedesc, 0);

  cond->used_for_waiting = NULL;

  return 0;
}

int cond_destroy(cond_t *cond)
{
  if (!iq_isempty(&cond->waiters))
    return (EBUSY);

  return 0;
}

int cond_signal(cond_t *cond)
{
  LEVEL l;
  PID p;

  proc_table[exec_shadow].context = kern_context_save();

  if (!iq_isempty(&cond->waiters)) {
    p = iq_getfirst(&cond->waiters);

    l = proc_table[p].task_level;
    level_table[l]->public_unblock(l,p);

    scheduler();
  }

  kern_context_load(proc_table[exec_shadow].context);
  return 0;
}

int cond_broadcast(cond_t *cond)
{
  LEVEL l;
  PID p;

  proc_table[exec_shadow].context = kern_context_save();

  if (!iq_isempty(&cond->waiters)) {
    do {
      p = iq_getfirst(&cond->waiters);
  
      l = proc_table[p].task_level;
      level_table[l]->public_unblock(l,p);
    } while(!iq_isempty(&cond->waiters));

    scheduler();
  }
  kern_context_load(proc_table[exec_shadow].context);
  return 0;
}

int cond_wait(cond_t *cond, mutex_t *mutex)
{
  LEVEL l;

  /* Why I used task_nopreempt???... because we have to unlock the mutex,
     and we can't call mutex_unlock after kern_context_save (the unlock
     could call context_save itself...) */
  task_nopreempt();

  /* First, the cond_wait is a cancellation point... */
  task_testcancel();

  /* all the task that uses NOW this condition are waiting on the same
     mutex??? */
  if (cond->used_for_waiting) {
    if (cond->used_for_waiting != mutex) {
      task_preempt();
      return (EINVAL);
    }
  }
  else
    cond->used_for_waiting = mutex;

  /* If the task is not canceled with testcancel, we block it now... */

  /* The mutex can't be destroyed while we are waiting on a condition,
     so we tell the mutex that a task is using it althought it is not
     busy (the cond_wait have to unlock the mutex!!!)... */
  mutex->use++;
  if (mutex_unlock(mutex)) {
    /* some problems unlocking the mutex... */
    mutex->use--;
    task_preempt();
    return (EINVAL);
  }

  /* now, we really block the task... */
  proc_table[exec_shadow].context = kern_context_save();

  kern_epilogue_macro();

  l = proc_table[exec_shadow].task_level;
  level_table[l]->public_block(l,exec_shadow);

  /* we insert the task in the condition queue */
  proc_table[exec_shadow].status = WAIT_COND;
  iq_priority_insert(exec_shadow,&cond->waiters);

  /* then, we set into the processor descriptor the condition on that
     the task is blocked... (if the task is killed while it is waiting
     on the condition, we have to remove it from the waiters queue, so
     we need the condition variable...) */
  proc_table[exec_shadow].cond_waiting = cond;

  /* and finally we reschedule */
  exec = exec_shadow = -1;
  scheduler();
  ll_context_to(proc_table[exec_shadow].context);
  kern_sti();

  /* when we arrive here:
     - the task did't die while it was waiting on the condition
       (normally, the cancelability state is set to deferred;
        if someone kills the task, we have first to lock the mutex,
        and then die. Furthermore no cond_signal can be catched by a task
        that have to die, so in the makefree function we extract the
        task from the waiters queue)
     - the task still in the non-preemptive state
     - the task have to reaquire the mutex to test again the condition
     - the task have to reset the cond_waiting pointer set before
  */
  if (proc_table[exec_shadow].cond_waiting != NULL) {
    proc_table[exec_shadow].cond_waiting = NULL;

    if (iq_isempty(&cond->waiters)) cond->used_for_waiting = NULL;
  }
  task_preempt();

  mutex_lock(mutex);
  mutex->use--;

  task_testcancel();

  return 0;
}


/* if this event fires the task passed as argument is blocked on a condition
   with a cond_timedwait.
   We have to:
   - extract the task from the waiters queue, because the task has to be
     woken up and must not use any cond_signal
   - reset the delaytimer...
   - call the task-insert...
*/
void condition_timer(void *arg)
{
  PID p = (PID)arg;
  LEVEL l;

  iq_extract(p,&proc_table[p].cond_waiting->waiters);
  if (iq_isempty(&proc_table[p].cond_waiting->waiters))
    proc_table[p].cond_waiting->used_for_waiting = NULL;
  proc_table[p].cond_waiting = NULL;

  proc_table[p].delay_timer = -1;

  l = proc_table[p].task_level;
  level_table[l]->public_unblock(l,p);

  event_need_reschedule();
}

int cond_timedwait(cond_t *cond, mutex_t *mutex,
                   const struct timespec *abstime)
{
  LEVEL l;
  int returnvalue = 0;

  /* Why I used task_nopreempt???... because we have to unlock the mutex,
     and we can't call mutex_unlock after kern_context_save (the unlock
     could call context_save itself...) */
  task_nopreempt();

  /* First, the cond_wait is a cancellation point... */
  task_testcancel();

  /* all the task that uses NOW this condition are waiting on the same
     mutex??? */
  if (cond->used_for_waiting) {
    if (cond->used_for_waiting != mutex) {
      task_preempt();
      return (EINVAL);
    }
  }
  else
    cond->used_for_waiting = mutex;

  /* If the task is not canceled with testcancel, we block it now... */

  /* The mutex can't be destroyed while we are waiting on a condition,
     so we tell the mutex that a task is using it althought it is not
     busy (the cond_wait have to unlock the mutex!!!)... */
  mutex->use++;
  if (mutex_unlock(mutex)) {
    /* some problems unlocking the mutex... */
    mutex->use--;
    task_preempt();
    return (EINVAL);
  }

  /* now, we really block the task... */
  proc_table[exec_shadow].context = kern_context_save();

  kern_epilogue_macro();

  l = proc_table[exec_shadow].task_level;
  level_table[l]->public_block(l,exec_shadow);

  /* we insert the task in the condition queue */
  proc_table[exec_shadow].status = WAIT_COND;
  iq_priority_insert(exec_shadow,&cond->waiters);

  /* then, we set into the processor descriptor the condition on that
     the task is blocked... (if the task is killed while it is waiting
     on the condition, we have to remove it from the waiters queue, so
     we need the condition variable...) */
  proc_table[exec_shadow].cond_waiting = cond;

  /* we can use the delaytimer because if we are here we are not in a
     task_delay */
  proc_table[exec_shadow].delay_timer =
    kern_event_post(abstime,condition_timer,(void *)exec_shadow);

  /* and finally we reschedule */
  exec = exec_shadow = -1;
  scheduler();
  ll_context_to(proc_table[exec_shadow].context);

  if (proc_table[exec_shadow].delay_timer != -1)
    kern_event_delete(proc_table[exec_shadow].delay_timer);

  kern_sti();
  /* when we arrive here:
     - the task did't die while it was waiting on the condition
       (normally, the cancelability state is set to deferred;
        if someone kills the task, we have first to lock the mutex,
        and then die. Furthermore no cond_signal can be catched by a task
        that have to die, so in the makefree function we extract the
        task from the waiters queue)
     - the task still in the non-preemptive state
     - the task have to reaquire the mutex to test again the condition
     - the task have to reset the cond_waiting pointer set before
     Note that cond_wait has to be called with cancelability set to
     deferred... so we insert a testcancel after the mutex_lock...
  */
  if (proc_table[exec_shadow].cond_waiting != NULL) {
    proc_table[exec_shadow].cond_waiting = NULL;

    if (iq_isempty(&cond->waiters)) cond->used_for_waiting = NULL;
  }
  else
    /* cond_waiting == NULL if the task is killed or the timer has fired */
    returnvalue = ETIMEDOUT;

  task_preempt();

  mutex_lock(mutex);
  mutex->use--;

  task_testcancel();

  return returnvalue;
}
