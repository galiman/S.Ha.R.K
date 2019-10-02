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

/*
 * Copyright (C) 2000 Giorgio Buttazzo, Paolo Gai
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
 *
 * CVS :        $Id: condtest.c,v 1.3 2005/01/08 14:31:38 pj Exp $

 This test verify the correctness of the condition variables.
 (... it doesn't test all...)

 The test uses 1 mutex

 the main task (NRT) creates three tasks.

 J0, J1, J3
   starts, lock the mutex, and wait on a condition variable

 J2
   at t = 0.5 lock the mutex and call cond_signal
   at t = 1   lock the mutex and call cond_signal

*/

#include "kernel/kern.h"

mutex_t  m0;
cond_t   c0;

int number = 0;

PID p0,p1,p2,p3;

TASK j0()
{
  cprintf("J0: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J0: locked           m0, waiting on c0, number =%d\n", number);
  while (!number) {
    cond_wait(&c0,&m0);
    cprintf("J0: number = %d, if >0 unlocking m0\n",number);
  }
  number--;
  mutex_unlock(&m0);
  cprintf("J0: unlocked         m0, end task\n");
  return 0;
}


TASK j1()
{
  cprintf("J1: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J1: locked           m0, waiting on c0, number =%d\n", number);
  while (!number) {
    cond_wait(&c0,&m0);
    cprintf("J1: number = %d, if >0 unlocking m0\n",number);
  }
  number--;
  mutex_unlock(&m0);
  cprintf("J1: unlocked         m0, end task\n");
  return 0;
}


TASK j2()
{

  cprintf("J2: started, waiting t=0.5 sec\n");
  while (sys_gettime(NULL) < 500000);

  cprintf("J2: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J2: locked           m0, number++ (was %d), cond_signal\n", number);

  number++;
  cond_signal(&c0);

  cprintf("J2: unlocking m0\n");
  mutex_unlock(&m0);

  cprintf("J2: waiting t=1 sec\n");
  while (sys_gettime(NULL) < 1000000);

  cprintf("J2: Killing J3\n");
  task_kill(p3);
  cprintf("J2: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J2: locked           m0, number++ (was %d), cond_signal\n", number);

  number++;
  cond_signal(&c0);

  cprintf("J2: unlocking m0\n");
  mutex_unlock(&m0);
  cprintf("J2: unlocked         m0, end task\n");
  return 0;
}

void cleanup_lock(void *arg)
{
  cprintf("J3: KILL!!!\n");
  mutex_unlock(&m0);
  cprintf("J3: unlocked m0 by the cleanup function\n");
}

TASK j3()
{
  cprintf("J3: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J3: locked           m0, waiting on c0, number =%d\n", number);
  task_cleanup_push(cleanup_lock, (void *)&m0);
  while (!number) {
    cond_wait(&c0,&m0);
    cprintf("J3: number = %d, if >0 unlocking m0\n",number);
  }
  task_cleanup_pop(0);
  // I hope this task never reach this point... it is killed by J2!!!
  number--;
  mutex_unlock(&m0);
  cprintf("J3: unlocked         m0, end task\n");
  return 0;
}


int main(int argc, char **argv)
{
  struct timespec t;

  NRT_TASK_MODEL m;

  PI_mutexattr_t a;

  /* ---------------------------------------------------------------------
     Task creation
     --------------------------------------------------------------------- */

  nrt_task_default_model(m);
  nrt_task_def_group(m,1);
  p0 = task_create("J0", j0, &m, NULL);
  if (p0 == NIL)
  { cprintf("Can't create J0 task...\n"); return 1; }

  p1 = task_create("J1", j1, &m, NULL);
  if (p1 == NIL)
  { cprintf("Can't create J1 task...\n"); return 1; }

  p2 = task_create("J2", j2, &m, NULL);
  if (p2 == NIL)
  { cprintf("Can't create J2 task...\n"); return 1; }

  p3 = task_create("J3", j3, &m, NULL);
  if (p3 == NIL)
  { cprintf("Can't create J3 task...\n"); return 1; }

  /* ---------------------------------------------------------------------
     Mutex creation
     --------------------------------------------------------------------- */

  PI_mutexattr_default(a);
  mutex_init(&m0,&a);

  cond_init(&c0);

  /* ---------------------------------------------------------------------
     Event post
     --------------------------------------------------------------------- */

  group_activate(1);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);
                                                                                                                             
  exit(1);

  return 0;

}
