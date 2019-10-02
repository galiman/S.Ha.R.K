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
 * CVS :        $Id: semdemo.c,v 1.3 2005/01/08 14:31:38 pj Exp $

 This is a really simple semaphore demo.

*/

#include "kernel/kern.h"

#include "semaphore.h"

sem_t s;

void *goofy(void *a)
{
  struct timespec t;
  char *n = proc_table[exec_shadow].name;

  cprintf("Task %s: Locking semaphore...\n", n);

  sem_wait(&s);
  cprintf("Task %s: Semaphore locked...\n", n);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < (int)a);

  cprintf("Task %s: Unlocking semaphore...\n", n);

  sem_post(&s);

  cprintf("Task %s: Semaphore unlocked...\n", n);

  return 0;
}

int main(int argc, char **argv)
{
  NRT_TASK_MODEL m;
  PID p2,p3;

  struct timespec t;

  nrt_task_default_model(m);
  nrt_task_def_group(m,1);

  nrt_task_def_arg(m,(void *)1);
  p2 = task_create("goofy1", goofy, &m, NULL);
  if (p2 == NIL)
  { cprintf("Can't create goofy1 task...\n"); return 1; }

  nrt_task_def_arg(m,(void *)2);
  p3 = task_create("goofy2", goofy, &m, NULL);
  if (p3 == NIL)
  { cprintf("Can't create goofy2 task...\n"); return 1; }
  
  cprintf("Initializing semaphore...\n");

  sem_init(&s,0,1);

  group_activate(1);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);

  exit(0);

  return 0;
}
