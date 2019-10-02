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
 * CVS :        $Id: mdemo.c,v 1.4 2005/02/25 11:10:46 pj Exp $

 This test verify the correctness of the NOP module. It works with the
 PI, PC, SRP module, too.

 The test uses one mutex

 the main task (NRT) creates three tasks.

 J1 with PC priority 0
   starts at t=0.5 sec and lock m0

 J2 with PC priority 1
   starts at t=1 sec and doesn't lock any mutex

 J3 with PC priority 2
   it starts and locks m0
   at t=2 sec it unlocks m1


 The example is similar to the scheduling diagram shown at p. 188 of the
 book "Sistemi in tempo Reale", by Giorgio Buttazzo, Pitagora Editrice

*/

#include "kernel/kern.h"
#include "srp/srp/srp.h"

mutex_t  m0;


void startJ(void *a)
{
  task_activate((PID)a);
}

TASK j1()
{
  cprintf("J1: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J1: locked           m0\n");
  mutex_unlock(&m0);
  cprintf("J1: unlocked         m0, end task\n");
  return 0;
}


TASK j2()
{
  cprintf("J2: waiting t=1.5 sec\n");

  while (sys_gettime(NULL) < 1500000);

  cprintf("J2: end task\n");
  return 0;
}


TASK j3()
{
  cprintf("J3: before locking   m0\n");
  mutex_lock(&m0);
  cprintf("J3: locked           m0, waiting to t=2 sec\n");

  while (sys_gettime(NULL) < 2000000);

  cprintf("J3: t = 1 sec reached, unlocking m0\n");
  mutex_unlock(&m0);
  cprintf("J3: unlocked         m0, end task\n");
  return 0;
}

int main(int argc, char **argv)
{
  struct timespec t;

  HARD_TASK_MODEL m;
  PID      p0,p1,p2;

  PC_mutexattr_t  a;
  PI_mutexattr_t  a2;
  NOP_mutexattr_t a3;
  SRP_mutexattr_t a4;
  NPP_mutexattr_t a5;

  PC_RES_MODEL r;
  SRP_RES_MODEL srp;

  /* ---------------------------------------------------------------------
     Mutex creation
     --------------------------------------------------------------------- */

  PC_mutexattr_default(a,0);
  PI_mutexattr_default(a2);
  NOP_mutexattr_default(a3);
  SRP_mutexattr_default(a4);
  NPP_mutexattr_default(a5);
  mutex_init(&m0,&a4);

  /* ---------------------------------------------------------------------
     Task creation
     --------------------------------------------------------------------- */

  hard_task_default_model(m);
  hard_task_def_wcet(m,20000);
  hard_task_def_mit(m,10000000);
  PC_res_default_model(r,0);
  SRP_res_default_model(srp,3);
  p0 = task_createn("J1", j1, (TASK_MODEL *)&m, &r, &srp, SRP_usemutex(&m0), NULL);
  if (p0 == NIL)
  { cprintf("Can't create J1 task...\n"); return 1; }

  hard_task_def_wcet(m,1600000);
  hard_task_def_mit(m,21000000);
  PC_res_default_model(r,1);
  SRP_res_default_model(srp,2);
  p1 = task_createn("J2", j2, (TASK_MODEL *)&m, &r, &srp, NULL);
  if (p1 == NIL)
  { cprintf("Can't create J2 task...\n"); return 1; }

  hard_task_def_wcet(m,3000000);
  hard_task_def_mit(m,100000000);
  PC_res_default_model(r,2);
  SRP_res_default_model(srp,1);
  p2 = task_createn("J3", j3, (TASK_MODEL *)&m, &r, &srp, SRP_usemutex(&m0), NULL);
  if (p2 == NIL)
  { cprintf("Can't create J3 task...\n"); return 1; }


  /* ---------------------------------------------------------------------
     Event post
     --------------------------------------------------------------------- */

  t.tv_sec = 0;
  t.tv_nsec = 500000000;

  kern_cli();
  kern_event_post(&t,startJ,(void *)p0);

  t.tv_sec = 1;
  kern_event_post(&t,startJ,(void *)p1);
  kern_sti();

  task_activate(p2);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 60);
                                                                                                                             
  exit(0);

  return 0;
}
