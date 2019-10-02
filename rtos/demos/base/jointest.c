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
 * CVS :        $Id: jointest.c,v 1.3 2005/01/08 14:31:38 pj Exp $

 This test verify the correctness of the task_join primitive. (that
 function is the same as pthread_join... someday I will change the
 names...)

 There are 4 taks, J1, J2, J3, are created as joinable, J4 as detached

 The main task:
   Creates J1 and J2, locks m1 (a PI mitex), creates C3.
   at t=0.8 sec it calls a task_join on J3 (that returns EDEADLK),
   it unlocks m1, then it makes task_join on J3 another time.
   Next it creates J4 as detached and finally it does a task_join on J4
   (that returns EINVAL).

 J1:
   at t=0.2 sec it calls task_join on J2, the it ends.

 J2:
   it simply waits t=0.4 sec and it ends.

 J3:
   First, it calls task_join on J1.
   Then, at t=0.6 sec it locks m1, then unlocks it

 J4:
   it simply waits t=1 sec and it ends.

*/

#include "kernel/kern.h"

PID j0, j1, j2, j3, j4;
mutex_t  m1;

TASK J1()
{
  int err;
  void *ret;

  cprintf("J1: started, waiting 0.2 sec\n");

  while (sys_gettime(NULL) < 200000);

  cprintf("J1: 0.2 sec reached, joining J2\n");

  err = task_join(j2, &ret);

  cprintf("J1: join J2 returns %d error %d, exiting\n",
              (int)ret,err);
  return (void *)11;
}

TASK J2()
{
  cprintf("J2: started, waiting 0.4 sec\n");

  while (sys_gettime(NULL) < 400000);

  cprintf("J2: 0.4 sec reached, exiting\n");

  return (void *)22;
}

TASK J3()
{
  int err;
  void *ret;

  cprintf("J3: started, joining J1\n");

  err = task_join(j1, &ret);

  cprintf("J3: join J1 returns %d error %d, waiting 0.6sec\n", (int)ret, err);

  while (sys_gettime(NULL) < 600000);

  cprintf("J1: 0.6 sec reached, locking m1\n");

  mutex_lock(&m1);

  cprintf("J3: locked m1, unlocking m1\n");

  mutex_unlock(&m1);

  cprintf("J3: unlocked m1, exiting\n");

  return (void *)33;
}

TASK J4()
{
  cprintf("J4: started, waiting 1 sec\n");

  while (sys_gettime(NULL) < 1000000);

  cprintf("J4: 1 sec reached, exiting\n");

  return (void *)44;
}

int main(int argc, char **argv)
{
  NRT_TASK_MODEL m;

  PI_mutexattr_t a;

  int err;
  void *ret;

  struct timespec t;

  j0 = exec_shadow;
  nrt_task_default_model(m);
  nrt_task_def_joinable(m);

  /* ---------------------------------------------------------------------
     Mutex creation
     --------------------------------------------------------------------- */

  PI_mutexattr_default(a);
  mutex_init(&m1,&a);


  /* ---------------------------------------------------------------------
     Let's go !!!!
     --------------------------------------------------------------------- */

  cprintf("main: creating J1,J2,J3, locking m1\n");

  j1 = task_create("J1", J1, &m, NULL);
  if (j1 == NIL) { cprintf("Can't create J1 task...\n"); return 1; }
  task_activate(j1);

  j2 = task_create("J2", J2, &m, NULL);
  if (j2 == NIL) { cprintf("Can't create J2 task...\n"); return 1; }
  task_activate(j2);

  mutex_lock(&m1);

  j3 = task_create("J3", J3, &m, NULL);
  if (j3 == NIL) { cprintf("Can't create J3 task...\n"); return 1; }
  task_activate(j3);

  cprintf("main: waiting t=0.8 sec\n");

  while (sys_gettime(NULL) < 800000);

  err = task_join(j3, NULL);

  cprintf("main: join J3 error %d, unlocking m1\n",err);

  mutex_unlock(&m1);

  err = task_join(j3, &ret);

  cprintf("main: join J3 returns %d error %d, unlocked m1, creating J4\n",
            (int)ret,err);

  nrt_task_def_unjoinable(m);
  j4 = task_create("J4", J4, &m, NULL);
  if (j4 == NIL) { cprintf("Can't create J4 task...\n"); return 1; }

  task_activate(j4);

  err = task_join(j4,&ret);

  cprintf("main: join J4 returns %d error %d, exiting\n", (int)ret, err);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);
                                                                                                                             
  exit(0);

  return 0;

}
