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
 CVS :        $Id: int_sem.c,v 1.4 2003/11/05 15:05:11 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2003/11/05 15:05:11 $
 ------------

 Internal semaphores.

 They are different from the Posix semaphores and the mutexes because:
 - internal_sem_wait is not a cancellation point
 - there are no limits on the semaphores that can be created
   (they works like a mutex_t...)
 - the queuing policy is FIFO
 - Be Careful!
   they are made to be fast... so not so many controls are done!!!

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


#include <kernel/int_sem.h>
#include <kernel/var.h>
#include <kernel/func.h>

/* Wait status for this library */
#define INTERNAL_SEM_WAIT LIB_STATUS_BASE


void internal_sem_init(internal_sem_t *s, int value)
{
  s->count = value;
  iq_init(&s->blocked,&freedesc,0);
}

void internal_sem_wait(internal_sem_t *s)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (s->count) {
    s->count--;
    kern_frestore(f);
    return;
  }
  else {                /* We must block exec task   */
    LEVEL l;            /* for readableness only */

    proc_table[exec_shadow].context = kern_context_save();

    kern_epilogue_macro();
  
    l = proc_table[exec_shadow].task_level;
    level_table[l]->public_block(l,exec_shadow);

    /* we insert the task in the semaphore queue */
    proc_table[exec_shadow].status = INTERNAL_SEM_WAIT;
    iq_insertlast(exec_shadow,&s->blocked);

    /* and finally we reschedule */
    exec = exec_shadow = -1;
    scheduler();

    ll_context_to(proc_table[exec_shadow].context);
    kern_after_dispatch();
    kern_frestore(f); 
  }
  
}

/* return 0 if the counter is decremented, -1 if not */
int internal_sem_trywait(internal_sem_t *s)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (s->count) {
    s->count--;
    kern_frestore(f);
    return 0;
  }

  return -1;
}


void internal_sem_post(internal_sem_t *s)
{
  proc_table[exec_shadow].context = kern_context_save();

  if (s->blocked.first != -1) {
    register PID p;
    register LEVEL l;

    p = iq_getfirst(&s->blocked);
    l = proc_table[p].task_level;
    level_table[l]->public_unblock(l,p);

    scheduler();
  }
  else
    s->count++;

  kern_context_load(proc_table[exec_shadow].context);
}

int internal_sem_getvalue(internal_sem_t *s)
{
  register int returnvalue;
  SYS_FLAGS f;

  f = kern_fsave();
  if (s->blocked.first == -1)
    returnvalue = s->count;
  else
    returnvalue = -1;

  kern_frestore(f);
  return returnvalue;
}
