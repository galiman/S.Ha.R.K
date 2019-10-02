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
 * CVS :        $Id: intsem.c,v 1.3 2005/01/08 14:31:38 pj Exp $

 this test is a simple main() function with one other task.

 This test verify the correctness of the internal_sem functions.

*/

#include "kernel/kern.h"

#include <kernel/int_sem.h>

internal_sem_t s;

TASK pippo(void *a)
{
  int i=0;
  struct timespec t;

  do {
    sys_gettime(&t);

    if (i==0 && t.tv_sec == (int)a) {
      i = 1;
      cprintf("before internal_sem_wait %d\n",(int)a);
      internal_sem_wait(&s);
      cprintf("after internal_sem_wait %d\n",(int)a);
    }

    if (i==1 && t.tv_sec == 2+(int)a) {
      i = 2;
      cprintf("before internal_sem_post %d\n",(int)a);
      internal_sem_post(&s);
      cprintf("after internal_sem_post %d\n",(int)a);
      return 0;
    }


  } while (1);
}

int main(int argc, char **argv)
{
  NRT_TASK_MODEL m;
  PID p2,p3;

  struct timespec t;

  nrt_task_default_model(m);
  nrt_task_def_group(m,1);

  nrt_task_def_arg(m,(void *)1);
  p2 = task_create("pippo1", pippo, &m, NULL);
  if (p2 == NIL)
  { cprintf("Can't create pippo1 task...\n"); return 1; }

  nrt_task_def_arg(m,(void *)2);
  p3 = task_create("pippo2", pippo, &m, NULL);
  if (p3 == NIL)
  { cprintf("Can't create pippo2 task...\n"); return 1; }

  internal_sem_init(&s,1);

  group_activate(1);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);
                                                                                                                             
  exit(0);

  return 0;
}
