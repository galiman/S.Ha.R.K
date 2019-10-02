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
 CVS :        $Id: ptest4.c,v 1.4 2005/01/08 14:36:11 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:36:11 $
 ------------

 Posix test 4:

   the main task create 3 tasks, J1, J2, J3
     at t = 1 sec. it raise a signal to J1
     at t = 2 sec. it kills J2

   J1,J2,J3: it simply calls nanosleep

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
#include <time.h>

#include <kernel/kern.h>

void uscitaJ(void *arg)
{
  cprintf("J: (pid=%d) AAAARRRRGGGHHH!!! killed by someone...\n", exec_shadow);
}

void *J(void *arg)
{
  struct timespec t1, t2;
  int err;

  cprintf("J (pid=%d) starts and call nanosleep\n",exec_shadow);

  t1.tv_sec = 3;
  t1.tv_nsec = 0;
  NULL_TIMESPEC(&t2);
  pthread_cleanup_push(uscitaJ,NULL);
  err = nanosleep(&t1, &t2);
  pthread_cleanup_pop(0);

  cprintf("J (pid=%d) ending, nanosleep returns errno=%d, t2=%ld.%ld\n",
              exec_shadow, err, t2.tv_sec, t2.tv_nsec/1000);

  return 0;
}

void signal_handler(int signo, siginfo_t *info, void *extra)
{
  cprintf("SIGNAL HANDLER: pid=%d\n",exec_shadow);
}


int main(int argc, char **argv)
{
  int err;
  pthread_t j1, j2, j3;
  struct sigaction sig_act;

  sig_act.sa_sigaction = (void *) signal_handler;
  sig_act.sa_flags = SA_SIGINFO;
  sigemptyset(&sig_act.sa_mask);

  sigaction(31, &sig_act, NULL);

  cprintf("main: creating J1\n");
  err = pthread_create(&j1, NULL, J, NULL);
  if (err) cprintf("Error creating J1\n");

  cprintf("main: creating J2\n");
  err = pthread_create(&j2, NULL, J, NULL);
  if (err) cprintf("Error creating J2\n");

  cprintf("main: creating J3\n");
  err = pthread_create(&j3, NULL, J, NULL);
  if (err) cprintf("Error creating J3\n");

  cprintf("main: waiting 1 sec\n");
  while (sys_gettime(NULL) < 1000000);

  cprintf("main: pthread_kill on j1, then wait until t=2 sec \n");
  pthread_kill(j1, 31);

  while (sys_gettime(NULL) < 2000000);
  cprintf("main: pthread_cancel(J2)\n");
  pthread_cancel(j2);

  cprintf("main: ending...\n");

  return 0;
}
