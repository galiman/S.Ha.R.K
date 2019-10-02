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

/**
 ------------
 CVS :        $Id: sig.c,v 1.4 2005/01/08 14:31:38 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:31:38 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai and Giorgio Buttazzo
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
 */

/*
 Test Number 5:

 this test is a simple main() function with one other task

 This test can be useful to test functions like:

 sys_gettime
 sigemptyset
 sigaddset
 hartik_deliver_pending_signals
 task_sigmask
 sigaction
 sigqueue
 task_signal
*/

#include "kernel/kern.h"


TASK goofy(void *arg)
{
  struct timespec t;

  cprintf("Goofy: waiting 2 secs...\n");

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 2); // wait until 2 sec
  
  cprintf("Goofy: ok, I'm ready :-)\n");

  return 0;
}

void catchit_RT(int signo, siginfo_t *info, void *extra)
{
  cprintf("RT signal: Current Running Task = %d signo=%d code=%d value=%d from pid=%d\n",
            exec_shadow,
            info->si_signo, info->si_code,
            info->si_value.sival_int, info->si_task);
}

void catchit(int signo)
{
  cprintf("RT signal: Current Running Task = %d signo=%d\n", 
	  exec_shadow, signo);
}

int main(int argc, char **argv)
{
  struct timespec t;
  NRT_TASK_MODEL m;
  PID p2;

  sigset_t newmask;
  sigset_t oldmask;
  struct sigaction action;
  union sigval sval;

  clear();

  /* Set the signal action */
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = catchit_RT;
  action.sa_handler = 0;
  action.sa_mask = 0;

  if (sigaction(SIGUSR1, &action, NULL) == -1) {
    perror("Error using sigaction.");
    return -1;
  }

  action.sa_flags = 0;
  action.sa_handler = (void (*)(int))catchit;

  if (sigaction(SIGILL, &action, NULL) == -1) {
    perror("Error using sigaction.");
    return -1;
  }

  /* create another task */
  nrt_task_default_model(m);
  nrt_task_def_group(m,1);

  p2 = task_create("goofy", goofy, &m, NULL);
  if (p2 == NIL)
  { 
    cprintf("Can't create goofy task...\n"); 
    return 1; 
  }

  group_activate(1);

  /* block the signal for the main task */
  sigemptyset(&newmask);
  sigaddset(&newmask,SIGUSR1);
  task_sigmask(SIG_BLOCK, &newmask, &oldmask); // pthread_sigmask

  cprintf("main: Sending 2 signals ...\n");

  sval.sival_int = 123;
  sigqueue(0,SIGUSR1,sval);
  sval.sival_int = 999;
  sigqueue(0,SIGUSR1,sval);

  cprintf("main: Now sending a signal to myself,"
	  " then wait until 4 secs...\n");

  task_signal(0 /* main */, SIGILL); // pthread_kill

  NULL_TIMESPEC(&t);
  do {
    sys_gettime(&t);
  } while (t.tv_sec < 4); // wait until 4 s

  cprintf("main: ending...\n");

  return 0;
}
