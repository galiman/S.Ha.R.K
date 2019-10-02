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
 * CVS :        $Id: pidemo.c,v 1.4 2005/01/08 14:31:38 pj Exp $

 This test verify the correctness of the PI module.

 the main task (NRT) lock a PI mutex.
 then 2 tasks arrives, with priority higher than the main

 the first try to lock the mutex, but it can't, so the main inherit
 his priority. The second simply prints a string.

 If all works, the string of the second task is printed after the end of
 the first task.

*/

#include "kernel/kern.h"

mutex_t        m1;


TASK goofy1(void *a)
{
  cprintf("goofy1 before mutex_lock\n");
  mutex_lock(&m1);
  cprintf("goofy1 after mutex_lock\n");

  mutex_unlock(&m1);
  cprintf("goofy1 after mutex_unlock\n");

  return 0;
}

TASK goofy2()
{
 cprintf("goofy2 inside goofy2\n");
 return 0;
}

int main(int argc, char **argv)
{
  HARD_TASK_MODEL m;
  PID p2,p3;

  struct timespec t;

  PI_mutexattr_t a;

  hard_task_default_model(m);
  hard_task_def_mit(m,50000);
  hard_task_def_wcet(m,20000);
  hard_task_def_group(m,1);

  hard_task_def_arg(m,(void *)1);
  p2 = task_create("goofy1", goofy1, &m, NULL);
  if (p2 == NIL)
  { cprintf("Can't create goofy1 task...\n"); return 1; }

  hard_task_def_mit(m,100000);
  p3 = task_create("goofy2", goofy2, &m, NULL);
  if (p3 == NIL)
  { cprintf("Can't create goofy2 task...\n"); return 1; }

  PI_mutexattr_default(a);
  mutex_init(&m1,&a);

  cprintf("main before mutex_lock\n");
  mutex_lock(&m1);
  cprintf("main after mutex_lock\n");

  group_activate(1);

  cprintf("main after group_activate\n");
  mutex_unlock(&m1);
  cprintf("main after mutex_unlock\n");

  mutex_destroy(&m1);
  cprintf("main after mutex_destroy\n");

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10); 

  exit(0);

  return 0;

}
