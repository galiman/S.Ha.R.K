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
 CVS :        $Id: ptest2.c,v 1.4 2005/01/08 14:36:11 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:36:11 $
 ------------

 Posix test 2:

   pthread_once + thread_specific_data

   the main task:
     creates a key
     creates 2 tasks, J1, J2
     at t = 0.4 sec. it kills J1 and J2

   J1 and J2 will set and check the thread specific data
     and when the die, a destructor is called (twice, because the value
     is not set to null...)

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

#include <kernel/kern.h>

pthread_key_t prova_key;

pthread_once_t once = PTHREAD_ONCE_INIT;

void once_init()
{
  cprintf("ONCE: (pid=%d)\n", exec_shadow);
}

void destr_key(void *arg)
{
  cprintf("J (pid=%d) destructor called with value %d\n", exec_shadow,(int)arg);
  pthread_setspecific(prova_key,(void *)((int)arg/100));
}

void print_test()
{
  int val;

  val = (int)pthread_getspecific(prova_key);
  cprintf("J (pid=%d) printtest value=%d\n", exec_shadow, val);
}

void *J(void *arg)
{
  pthread_once(&once, once_init);
  cprintf("J (pid=%d) starts and call setspecific\n", exec_shadow);
  pthread_setspecific(prova_key,arg);
  print_test();
  cprintf("J (pid=%d) exits\n", exec_shadow);

  return 0;
}

int main(int argc, char **argv)
{
  int err;
  pthread_t j1, j2;

  cprintf("main: creating prova_key\n");
  pthread_key_create(&prova_key, destr_key);

  cprintf("main: provakey =%d\n", prova_key);

  cprintf("main: creating J1\n");
  err = pthread_create(&j1, NULL, J, (void *)1414);
  if (err) cprintf("Error creating J1\n");
  cprintf("main: J1 has PID %d\n",j1);

  cprintf("main: creating J2\n");
  err = pthread_create(&j2, NULL, J, (void *)3141);
  if (err) cprintf("Error creating J2\n");
  cprintf("main: J2 has PID %d\n",j2);

  cprintf("main: waiting 0.4 sec\n");
  while (sys_gettime(NULL) < 400000);

  cprintf("main: kill J1 and J2\n");
  pthread_cancel(j1);
  pthread_cancel(j2);

  cprintf("main: ending...\n");

  return 0;
}
