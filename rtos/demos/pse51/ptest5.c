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
 CVS :        $Id: ptest5.c,v 1.3 2005/01/08 14:36:11 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2005/01/08 14:36:11 $
 ------------

 Posix test 5:
   an alarm test

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
#include <unistd.h>

#include <kernel/kern.h>

void signal_handler(int signo, siginfo_t *info, void *extra)
{
  cprintf("SIGNAL HANDLER: pid=%d\n",exec_shadow);
}

int main(int argc, char **argv)
{
  struct sigaction sig_act;

  sig_act.sa_sigaction = (void *) signal_handler;
  sig_act.sa_flags = SA_SIGINFO;
  sigemptyset(&sig_act.sa_mask);

  sigaction(SIGALRM, &sig_act, NULL);

  cprintf("main: alarm(5), waiting t=2 sec\n");
  alarm(5);

  while (sys_gettime(NULL) < 2000000);

  cprintf("main: alarm(3) return %d, waiting t=6 sec\n",alarm(3));

  pause();
//  while (sys_gettime(NULL) < 6000000);

  cprintf("main: ending...\n");

  return 0;
}
