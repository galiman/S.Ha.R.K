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
 * CVS :        $Id: srpdemo.c,v 1.5 2005/02/25 11:10:46 pj Exp $

 This test verify the correctness of the SRP module.

 There are 3 taks, Jh, Jm, Jl that uses 3 mutexes m1, m2, m3

 the main task (NRT) creates the three tasks.

 Jh with preemption level 3
   starts at t=1.5 sec and lock m3, lock m1, unlock m1, unlock m3

 Jm with preemption level 2
   starts at t=0.5 sec and lock m3, lock m2, unlock m2, unlock m3
   then lock and unlock m1

 Jl with preemption level 1
   it starts and locks m2
   at t=1 sec it locks m1
   at t=1.5 sec it unlocks m1
   then it unlocks m2, and finally it locks and unlocks m3


 The example is similar to the scheduling diagram shown at p. 210 of the
 book "Sistemi in tempo Reale", by Giorgio Buttazzo, Pitagora Editrice

*/

#include "kernel/kern.h"

#include "srp/srp/srp.h"

mutex_t  m1,m2,m3;

void startJ(void *a)
{
  task_activate((PID)a);
}

TASK Jlobby()
{
  cprintf("(*) JLobby!!!\n");
  return 0;
}

TASK Jh()
{
  PID l;
  HARD_TASK_MODEL m;
  SRP_RES_MODEL r;

  cprintf("JH: creating Jy before locking   m3\n");

  hard_task_default_model(m);
  hard_task_def_mit(m,30000);
  hard_task_def_wcet(m,1000);
  SRP_res_default_model(r,4);
  l = task_create("Jlobby",Jlobby,&m,&r);
  task_activate(l);

  mutex_lock(&m3);
  cprintf("JH: locked           m3, locking m1\n");
  mutex_lock(&m1);
  cprintf("JH: locked           m1, unlocking m1\n");
  mutex_unlock(&m1);
  cprintf("JH: unlocked         m1, unlocking m3\n");
  mutex_unlock(&m3);
  cprintf("JH: unlocked         m3, end task\n");
  return 0;
}


TASK Jm()
{
  cprintf("JM: before locking   m3\n");
  mutex_lock(&m3);
  cprintf("JM: locked           m3, locking m2\n");
  mutex_lock(&m2);
  cprintf("JM: locked           m2, unlocking m2\n");
  mutex_unlock(&m2);
  cprintf("JM: unlocked         m2, unlocking m3\n");
  mutex_unlock(&m3);
  cprintf("JM: unlocked         m3, locking m1\n");
  mutex_lock(&m1);
  cprintf("JM: locked           m1, unlocking m1\n");
  mutex_unlock(&m1);
  cprintf("JM: unlocked         m1, end task\n");
  return 0;
}


TASK Jl()
{
  cprintf("JL: before locking   m2\n");
  mutex_lock(&m2);
  cprintf("JL: locked           m2, waiting to t=1 sec\n");

  while (sys_gettime(NULL) < 1000000);

  cprintf("JL: t = 1 sec reached, locking m1\n");
  mutex_lock(&m1);
  cprintf("JL: locked           m1, waiting to t=2 sec\n");

  while (sys_gettime(NULL) < 2000000);

  cprintf("JL: t = 2 sec reached, unlocking m1\n");
  mutex_unlock(&m1);
  cprintf("JL: unlocked         m1, unlocking m2\n");

  mutex_unlock(&m2);

  cprintf("JL: unlocked         m2, locking m3\n");
  mutex_lock(&m3);
  cprintf("JL: locked           m3, unlocking m3\n");
  mutex_unlock(&m3);
  cprintf("JL: unlocked         m3, end task\n");
  return 0;
}

int main(int argc, char **argv)
{
  struct timespec t;

  HARD_TASK_MODEL m;
  PID      p0,p1,p2;

  SRP_mutexattr_t a;
  SRP_RES_MODEL r;

  PI_mutexattr_t a2;

  clear();
  
  cprintf("Stack resource Policy demo. It ends after 60 sec\n");

  /* ---------------------------------------------------------------------
     Mutex creation
     --------------------------------------------------------------------- */

  PI_mutexattr_default(a2);
  SRP_mutexattr_default(a);
  mutex_init(&m1,&a);
  mutex_init(&m2,&a);
  mutex_init(&m3,&a);

  /* ---------------------------------------------------------------------
     Task creation
     --------------------------------------------------------------------- */

  hard_task_default_model(m);
  hard_task_def_mit(m, 1000000);
  hard_task_def_wcet(m, 80000);
  SRP_res_default_model(r, 3);
  p0 = task_createn("JH", Jh, (TASK_MODEL *)&m, &r, SRP_usemutex(&m3), SRP_usemutex(&m1), NULL);
  if (p0 == NIL)
  { cprintf("Can't create JH task...\n"); return 1; }

  hard_task_default_model(m);
  hard_task_def_mit(m, 2100000);
  hard_task_def_wcet(m, 80000);
  SRP_res_default_model(r, 2);
  p1 = task_createn("JM", Jm, (TASK_MODEL *)&m, &r, SRP_usemutex(&m3), SRP_usemutex(&m1),
                                      SRP_usemutex(&m2), NULL);
  if (p1 == NIL)
  { cprintf("Can't create JM task...\n"); return 1; }

  hard_task_default_model(m);
  hard_task_def_mit(m, 10000000);
  hard_task_def_wcet(m, 3000000);
  SRP_res_default_model(r, 1);
  p2 = task_createn("JL", Jl, (TASK_MODEL *)&m, &r, SRP_usemutex(&m3), SRP_usemutex(&m1),
                                      SRP_usemutex(&m2), NULL);
  if (p2 == NIL)
  { cprintf("Can't create JL task...\n"); return 1; }

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
  } while (t.tv_sec < 60);
                                                                                                                             
  exit(0);

  return 0;
}
