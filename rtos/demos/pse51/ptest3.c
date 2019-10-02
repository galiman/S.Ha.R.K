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
 CVS :        $Id: ptest3.c,v 1.4 2005/01/08 14:36:11 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:36:11 $
 ------------

 Posix test 3:

   timers...
   it creates two periodic timers that queues signals, a periodic timer
   that create tasks and an one-shot timer.

 non standard function used:
   cprintf
   sys_gettime
   keyboard stuffs

**/

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
 */

#include <sys/types.h>
#include <pthread.h>
#include <time.h>

#include <kernel/kern.h>

int count25 = 0, count26 = 0;

void signal_handler(int signo, siginfo_t *info, void *extra)
{
  switch (signo) {
    case 25:
      count25++;
      break;
    case 26:
      count26++;
      break;
  }

  cprintf("Signal %d code=%s value=%d task=%d count25=%d count26=%d time=%ldusec\n",
              info->si_signo,
              (info->si_code == SI_TIMER) ? "Timer" : "Other",
              info->si_value.sival_int,
              info->si_task,
              count25,
              count26,
              sys_gettime(NULL));
}

void task_timer(union sigval value)
{
  cprintf("task_timer: value = %d, time = %ldusec\n",
              value.sival_int, sys_gettime(NULL));
}

int main(int argc, char **argv)
{
  int err;
  timer_t timer1, timer2, timer3;
  struct itimerspec timeout1, timeout2, timeout3, nulltimeout;
  struct sigaction sig_act;
  struct sigevent ev25, ev26, evtask;
  pthread_attr_t task_attr;
  struct sched_param task_param;

  sig_act.sa_sigaction = (void *) signal_handler;
  sig_act.sa_flags = SA_SIGINFO;
  sigemptyset(&sig_act.sa_mask);

  sigaction(25, &sig_act, NULL);
  sigaction(26, &sig_act, NULL);

  // set ev25, ev26, evtask
  ev25.sigev_notify           = SIGEV_SIGNAL;
  ev25.sigev_signo            = 25;
  ev25.sigev_value.sival_int  = 555;

  ev26.sigev_notify           = SIGEV_SIGNAL;
  ev26.sigev_signo            = 26;
  ev26.sigev_value.sival_int  = 666;

  evtask.sigev_notify            = SIGEV_THREAD;
  evtask.sigev_value.sival_int   = 777;
  evtask.sigev_notify_function   = task_timer;
  evtask.sigev_notify_attributes = &task_attr;

  pthread_attr_init(&task_attr);
  pthread_attr_setdetachstate(&task_attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setschedpolicy(&task_attr, SCHED_FIFO);
  task_param.sched_priority = 10;
  pthread_attr_setschedparam(&task_attr, &task_param);

  // set timeout1, timeout2, nulltimeout
  timeout1.it_interval.tv_sec  = 0;
  timeout1.it_interval.tv_nsec = 500000000;
  timeout1.it_value.tv_sec     = 3;
  timeout1.it_value.tv_nsec    = 0;

  timeout2.it_interval.tv_sec  = 0;
  timeout2.it_interval.tv_nsec = 200000000;
  timeout2.it_value.tv_sec     = 7;
  timeout2.it_value.tv_nsec    = 0;

  timeout3.it_interval.tv_sec  = 0;
  timeout3.it_interval.tv_nsec = 300000000;
  timeout3.it_value.tv_sec     = 5;
  timeout3.it_value.tv_nsec    = 0;

  NULL_TIMESPEC(&nulltimeout.it_value);
  NULL_TIMESPEC(&nulltimeout.it_interval);

  // create the timers
  err = timer_create(CLOCK_REALTIME, &ev25, &timer1);
  if (err == -1) { cprintf("main: unable to create timer 1\n"); }

  err = timer_create(CLOCK_REALTIME, &ev26, &timer2);
  if (err == -1) { cprintf("main: unable to create timer 2\n"); }

  err = timer_create(CLOCK_REALTIME, &evtask, &timer3);
  if (err == -1) { cprintf("main: unable to create timer 3\n"); }

  // arm the timers
  err = timer_settime(timer1, TIMER_ABSTIME, &timeout1, NULL);
  if (err == -1) { cprintf("main: unable to set timer 1\n"); }

  err = timer_settime(timer2, 0, &timeout2, NULL);
  if (err == -1) { cprintf("main: unable to set timer 2\n"); }

  err = timer_settime(timer3, TIMER_ABSTIME, &timeout3, NULL);
  if (err == -1) { cprintf("main: unable to set timer 3\n"); }

  cprintf("main: waiting signals...\n");
  while (sys_gettime(NULL) < 8500000) {
    //kern_deliver_pending_signals();
  }

  cprintf("main: disarm the timer2\n");
  err = timer_settime(timer2, 0, &nulltimeout, &timeout2);
  if (err == -1) { cprintf("main: unable to disarm timer 2\n"); }

  cprintf("main: timer2 disarmed, itvalue=%ld.%ld\n",
              timeout2.it_value.tv_sec,timeout2.it_value.tv_nsec/1000);

  while (sys_gettime(NULL) < 10000000) {
    //kern_deliver_pending_signals();
  }

  cprintf("main: disarm the timer1\n");
  err = timer_settime(timer1, TIMER_ABSTIME, &nulltimeout, &timeout1);
  if (err == -1) { cprintf("main: unable to disarm timer 1\n"); }

  cprintf("main: timer1 disarmed, itvalue=%ld.%ld\n",
              timeout1.it_value.tv_sec,timeout1.it_value.tv_nsec/1000);

  while (sys_gettime(NULL) < 12000000) {
    //kern_deliver_pending_signals();
  }

  cprintf("main: arm timer1\n");
  timeout1.it_interval.tv_sec  = 0;
  timeout1.it_interval.tv_nsec = 0;
  timeout1.it_value.tv_sec     = 13;
  timeout1.it_value.tv_nsec    = 0;
  err = timer_settime(timer1, TIMER_ABSTIME, &timeout1, NULL);
  if (err == -1) { cprintf("main: unable to arm timer 1\n"); }

  while (sys_gettime(NULL) < 14000000) {
    //kern_deliver_pending_signals();
  }

  cprintf("main: ending...\n");

  return 0;
}
