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
 *
 * CVS :        $Id: pcdemo.c,v 1.4 2005/01/08 14:31:38 pj Exp $

 This test verify the correctness of the PC module.

 The test uses 3 mutexes
 m0 with ceiling 0
 m1 with ceiling 0
 m2 with ceiling 1

 the main task (NRT) creates three tasks.

 J0 with PC priority 0
   starts at t=1.5 sec and lock m0, unlock m0, then lock and unlock m1

 J1 with PC priority 1
   starts at t=0.5 sec and try to lock m2

 J2 with PC priority 2
   it starts and locks m2
   at t=1 sec it locks m1
   at t=1.5 sec it unlocks m1


 The example is similar to the scheduling diagram shown at p. 197 of the
 book "Sistemi in tempo Reale", by Giorgio Buttazzo, Pitagora Editrice

*/

#include "kernel/kern.h"

mutex_t  m0,m1,m2;

void startJ(void *a)
{
  task_activate((PID)a);
}

TASK j0()
{
  cprintf("J0: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J0: locked           m0\n");
  mutex_unlock(&m0);
  cprintf("J0: unlocked         m0, locking m1\n");

  mutex_lock(&m1);
  cprintf("J0: locked           m1\n");
  mutex_unlock(&m1);
  cprintf("J0: unlocked         m1, end task\n");
  return 0;
}


TASK j1()
{
  cprintf("J1: before locking   m2\n");
  mutex_lock(&m2);
  cprintf("J1: locked           m2\n");
  mutex_unlock(&m2);
  cprintf("J1: unlocked         m2, end task\n");
  return 0;
}


TASK j2()
{
  cprintf("J2: before locking   m2\n");
  mutex_lock(&m2);
  cprintf("J2: locked           m2, waiting to t=1 sec\n");

  while (sys_gettime(NULL) < 1000000);

  cprintf("J2: t = 1 sec reached\n");
  mutex_lock(&m1);
  cprintf("J2: locked           m1, waiting to t=2 sec\n");

  while (sys_gettime(NULL) < 2000000);

  cprintf("J2: t = 2 sec reached\n");
  mutex_unlock(&m1);
  cprintf("J2: unlocked         m1\n");

  mutex_unlock(&m2);
  cprintf("J2: unlocked         m2, end task\n");
  return 0;
}

int main(int argc, char **argv)
{
  struct timespec t;

  HARD_TASK_MODEL m;
  PID      p0,p1,p2;

  PC_mutexattr_t a;
  PI_mutexattr_t a2;
  PC_RES_MODEL r;

  clear();

  cprintf("Priority Ceiling demo. It ends after 60 sec\n");

  /* ---------------------------------------------------------------------
     Task creation
     --------------------------------------------------------------------- */

  hard_task_default_model(m);
  hard_task_def_mit(m, 1000000);
  hard_task_def_wcet(m, 20000);
  PC_res_default_model(r,0);
  p0 = task_create("J0", j0, &m, &r);
  if (p0 == NIL)
  { cprintf("Can't create J0 task...\n"); return 1; }

  hard_task_default_model(m);
  hard_task_def_mit(m, 2100000);
  hard_task_def_wcet(m, 20000);
  PC_res_default_model(r,1);
  p1 = task_create("J1", j1, &m, &r);
  if (p1 == NIL)
  { cprintf("Can't create J1 task...\n"); return 1; }

  hard_task_default_model(m);
  hard_task_def_mit(m, 10000000);
  hard_task_def_wcet(m, 3000000);
  PC_res_default_model(r,2);
  p2 = task_create("J2", j2, &m, &r);
  if (p2 == NIL)
  { cprintf("Can't create J2 task...\n"); return 1; }

  /* ---------------------------------------------------------------------
     Mutex creation
     --------------------------------------------------------------------- */

  PI_mutexattr_default(a2);
  PC_mutexattr_default(a,0);
  mutex_init(&m0,(mutexattr_t *)&a);
  mutex_init(&m1,(mutexattr_t *)&a);

  PC_mutexattr_default(a,1);
  mutex_init(&m2,(mutexattr_t *)&a);

  /* ---------------------------------------------------------------------
     Event post
     --------------------------------------------------------------------- */

  t.tv_sec = 0;
  t.tv_nsec = 500000000;

  kern_cli();
  kern_event_post(&t,startJ,(void *)p1);

  t.tv_sec = 1;
  kern_event_post(&t,startJ,(void *)p0);
  kern_sti();

  task_activate(p2);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);
                                                                                                                             
  exit(0);

  return 0;

}
