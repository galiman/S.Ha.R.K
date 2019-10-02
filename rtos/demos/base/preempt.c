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
 * CVS :        $Id: preempt.c,v 1.4 2005/01/08 14:31:38 pj Exp $

 CBS test with preemption disabling

*/

#include "kernel/kern.h"

void *periodic(void *arg)
{
  int i;
  int y = (int)arg;

  for (i = 19; i < 60; i++) {
    puts_xy(i,y,7,"*");

    task_endcycle();
  }

  cprintf("Periodic: Task %d end.\n", exec_shadow);

  return 0;
}

void *blocker(void *arg)
{
  struct timespec t;

  task_nopreempt();
  cprintf("Blocker: Task nopreempt\n");

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 5);

  cprintf("Blocker: Task preempt\n");
  task_preempt();

  cprintf("Blocker: end\n");

  return 0;
}



int main(int argc, char **argv)
{
  struct timespec t;
  NRT_TASK_MODEL m;
  SOFT_TASK_MODEL m_aper;
  PID p;

  clear();
  cprintf("Preemption Test.\n");
  cprintf("Start time: two periodic tasks and a blocker are created and activated.\n");
  cprintf("2 seconds : blocker calls task_nopreempt.\n");
  cprintf("5 seconds : task_preempt is called.\n");
  cprintf("            The blocked task exec its pending activations.\n");
  cprintf("10 seconds: the test stops.\n");
  puts_xy(1,20,7,"save task:");
  puts_xy(1,21,7,"skip task:");

  nrt_task_default_model(m);

  soft_task_default_model(m_aper);
  soft_task_def_met(m_aper,10000);
  soft_task_def_period(m_aper,200000);
  soft_task_def_group(m_aper,1);
  soft_task_def_arg(m_aper, (void *)20);
  soft_task_def_periodic(m_aper);

  p = task_create("save", periodic, &m_aper, NULL);
  if (p == NIL)
    { 
      sys_shutdown_message("Can't create save task...\n"); 
      exit(1);
    }

  soft_task_def_skip_arrivals(m_aper);
  soft_task_def_arg(m_aper, (void *)21);

  p = task_create("skip", periodic, &m_aper, NULL);
  if (p == NIL)
    { 
      sys_shutdown_message("Can't create skip task...\n"); 
      exit(1);
    }

  p = task_create("blocker", blocker, &m, NULL);
  if (p == NIL)
    { 
      sys_shutdown_message("Can't create blocker task...\n"); 
      exit(1);
    }

  cprintf("main   : save & skip tasks activated.\n");
  group_activate(1);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 2);

  cprintf("main   : blocker activated.\n");
  task_activate(p);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);

  cprintf("main   : End!!!\n");

  return 0;
}
