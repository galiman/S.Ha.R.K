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
 * CVS :        $Id: aster1.c,v 1.4 2005/01/08 14:31:38 pj Exp $

 this is a reduced verion of the classic Hartik demo Aster.

 It uses:
 - EDF module
   . periodic tasks
 - an high number of task executing concurrently

 The demo ends after 6 seconds.

*/

/*
   Well, this is only a stupid demo which intend to show many
   HARTIK+ capabilities; the application is structured in the followig
   way: there is an ASTER task wich randomly creates some ASTEROID tasks
   which are displayed into the first window; each task is HARD/PERIODIC
   and auto-kills itself when it reaches the window end!
   Finally a CLOCK task is implemented to test system clock.
   Please note that usually the HARTIK+ application is made up of a task
   group which interacts among them, while the main() function, which
   became a task itself when the kernel is activated, is suspended until
   the system is ready to terminate; the MAIN task can also be used to make
   other background activities, but it should not be killed; when the
   application terminates, the control is passed to MAIN which kills
   everybody, shut down the system and can handle other operations using
   the services available with the previou operating system (I.E. the DOS).
   If you need to manage sudden abort/exception you should install your own
   exception handler and raise it through the exc_raise() primitive to
   make the system abort safely!
   Remember that the exit functions posted through sys_atexit() will be
   executed in both cases, to allow clean system shutdown.
*/

#include "kernel/kern.h"

int num_aster = 0;
#define ASTER_LIM	67
#define ASTER_MAX       90

TASK asteroide(void)
{
  int i = 1;
  int y = rand() % 20 + 1;
  while (i < ASTER_LIM) {
    puts_xy(i,y,WHITE,"*");
    task_endcycle();

    puts_xy(i,y,WHITE," ");
    i++;
  }
  num_aster--;
  return 0;
}

DWORD taskCreated = 0;

TASK aster(void)
{
  PID p;

  HARD_TASK_MODEL m;
  int r;

  hard_task_default_model(m);
  hard_task_def_wcet(m,500);

  srand(7);
  while (1) {
    if (num_aster < ASTER_MAX) {
      r = (rand() % 50) - 25;

      hard_task_def_arg(m,(void *)((rand() % 7)+1));
      hard_task_def_mit(m, (50+r)*1000);
      p = task_create("aaa",asteroide,&m,NULL);
      taskCreated++;
      task_activate(p);
      num_aster++;
    }

    task_endcycle();
  }
}

TASK clock()
{
  int s = 0, m = 0;

  while(1) {
    printf_xy(70,1,WHITE,"%2d : %2d",m,s);
    task_endcycle();

    if (++s > 59) {
      s = 0;
      m++;
    }
    printf_xy(70,1,WHITE,"%2d : %2d",m,s);
    task_endcycle();
  }
}

int main(int argc, char **argv)
{
  PID p1,p2;
  HARD_TASK_MODEL m;
  struct timespec t;

  clear();

  hard_task_default_model(m);
  hard_task_def_mit(m,10000);
  hard_task_def_wcet(m,2000);
  hard_task_def_group(m,1);

  p1 = task_create("Aster",aster,&m,NULL);
  if (p1 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <aster> ...");
    exit(1);
  }

  hard_task_def_mit(m,500000);
  p2 = task_create("Clock",clock,&m,NULL);
  if (p2 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <Clock> ...");
    exit(1);
  }

  group_activate(1);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 10);

  exit(0);
  return 0;
}

