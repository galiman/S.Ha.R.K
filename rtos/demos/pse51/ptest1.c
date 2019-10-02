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
 CVS :        $Id: ptest1.c,v 1.4 2005/01/08 14:36:11 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:36:11 $
 ------------

 Posix test 1:

   the main task create 4 tasks, J1, J2, J3, J4
     at t = 0.2 sec. it raise a signal to J1
     at t = 0.4 sec. it raise a signal to J2
     at t = 0.8 sec. it kill J4

   J1: it simply calls sigwait

   J2: it simply calls sigwaitinfo

   J3: it simply calls sigtimedwait with a timeout of 0.5 sec.

   J4: it simply calls sigtimedwait with a -long- timeout
       (J4 will be killed by main)

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
#include <signal.h>

#include <kernel/kern.h>

void *J1(void *arg)
{
  int sig, err;
  sigset_t mask;

  cprintf("J1 starts and call sigwait(31)\n");

  sigemptyset(&mask);
  sigaddset(&mask,31);
  err = sigwait(&mask,&sig);

  cprintf("J1 exit from sigwait(), err=%d, sig=%d\n", err, sig);

  return 0;
}

void *J2(void *arg)
{
  int err;
  siginfo_t info;
  sigset_t mask;

  cprintf("J2 starts and call sigwaitinfo(30)\n");

  sigemptyset(&mask);
  sigaddset(&mask,30);
  err = sigwaitinfo(&mask,&info);

  cprintf("J2 exit from sigwaitinfo(), err=%d, signo=%d code=%d value=%d\n",
              err, info.si_signo, info.si_code, info.si_value.sival_int);

  return 0;
}

void *J3(void *arg)
{
  int err;
  siginfo_t info;
  sigset_t mask;
  struct timespec t;

  cprintf("J3 starts and call sigtimedwait(29)\n");

  sigemptyset(&mask);
  sigaddset(&mask,29);
  t.tv_sec = 0;
  t.tv_nsec = 300000000;
  err = sigtimedwait(&mask,&info,&t);

  cprintf("J3 exit from sigtimedwait(), err=%d, signo=%d code=%d value=%d\n",
              err, info.si_signo, info.si_code, info.si_value.sival_int);

  return 0;
}

void uscitaJ4(void *arg)
{
  cprintf("J4: AAAARRRRGGGHHH!!! killed by someone...\n");
}

void *J4(void *arg)
{
  int err;
  siginfo_t info;
  sigset_t mask;
  struct timespec t;

  cprintf("J4 starts and call sigtimedwait(28)\n");

  sigemptyset(&mask);
  sigaddset(&mask,28);
  t.tv_sec = 10;
  t.tv_nsec = 0;

  pthread_cleanup_push(uscitaJ4,NULL);
  err = sigtimedwait(&mask,&info,&t);
  pthread_cleanup_pop(0);

  cprintf("J4 exit from sigtimedwait(), err=%d, signo=%d code=%d value=%d\n",
              err, info.si_signo, info.si_code, info.si_value.sival_int);

  return 0;
}

int main(int argc, char **argv)
{
  int err;
  sigset_t mask;
  pthread_t j1, j2, j3, j4;
  union sigval value;


  /* main blocks signals for all the tasks */
  sigfillset(&mask);
  pthread_sigmask(SIG_BLOCK, &mask, NULL);

  cprintf("main: creating J1\n");
  err = pthread_create(&j1, NULL, J1, NULL);
  if (err) cprintf("Error creating J1\n");

  cprintf("main: creating J2\n");
  err = pthread_create(&j2, NULL, J2, NULL);
  if (err) cprintf("Error creating J2\n");

  cprintf("main: creating J3\n");
  err = pthread_create(&j3, NULL, J3, NULL);
  if (err) cprintf("Error creating J3\n");

  cprintf("main: creating J4\n");
  err = pthread_create(&j4, NULL, J4, NULL);
  if (err) cprintf("Error creating J4\n");

  cprintf("main: waiting 0.2 sec\n");
  while (sys_gettime(NULL) < 200000);

  cprintf("main: kill(31), then wait until t=0.4 sec \n");
  kill(0, 31);

  while (sys_gettime(NULL) < 400000);
  cprintf("main: sigqueue(30), then wait until t=0.8 sec \n");
  value.sival_int = 300;
  sigqueue(0, 30, value);

  while (sys_gettime(NULL) < 800000);
  cprintf("main: kill(J4)\n");
  pthread_cancel(j4);

  cprintf("main: ending...\n");

  return 0;
}
