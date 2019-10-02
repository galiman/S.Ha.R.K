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
 CVS :        $Id: ptest6.c,v 1.3 2005/01/08 14:36:11 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2005/01/08 14:36:11 $
 ------------

 Posix test 6:

   message queues

   main thread:
     set a sigevent to sigev_thread on a mailbox (that creates thread 2)
     creates thread 1
     waits t=3.5 sec.
     pthread_cancel(T4)

   thread 1:
     send a msg to the mailbox (the event fires and thread 2 is created)

   thread 2:
     receive the msg sent by thread 1
     set the event to a signal
     creates thread 3 and 4
     waits t = 1 sec
     send another msg

   thread 3:
     receive the msg sent by 2 (it blocks!)
     waits t = 2 sec
     send 5 msgs (with different priorities!!!

   thread 4:
     receives 5 msgs every 0.5 sec.
     then receive another message that never will arrive...

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
#include <mqueue.h>

#include <kernel/kern.h>

struct sigevent ev25, evtask;
pthread_attr_t task_attr;
struct sched_param task_param;
mqd_t mq;
pthread_t T1,T2,T3,T4,T5;

#define MESSAGE_LENGTH 100

void *t1(void *arg)
{
  cprintf("T1: started, sending a message\n");
  if (mq_send(mq,"Donald Duck",12,1))
  { cprintf("T1: mq_send returns errno %d\n",errno); return 0; }

  cprintf("T1: ending...\n");
  return 0;
}

void *t4(void *arg);
void *t3(void *arg);

void t2(union sigval value)
{
  ssize_t x;
  char buf[MESSAGE_LENGTH];
  int prio;

  cprintf("T2: value = %d, receiving a message\n", value.sival_int);

  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);

  cprintf("T2: received message: length=%ld, prio=%d, text=°%s°, notify...\n",
              x,prio,buf);

  if (mq_notify(mq, &ev25))
  { cprintf("T2: mq_notify returns errno %d\n",errno); exit(1); }

  cprintf("T2: waiting t = 1 sec.\n");
  while (sys_gettime(NULL)<1000000);
  cprintf("T2: 1 sec. reached, sending another message and creating T3 and T4, \n");

  if (mq_send(mq,"Mickey Mouse",13,1))
  { cprintf("T2: mq_send returns errno %d\n",errno); exit(1); }

  pthread_create(&T3, NULL, t3, NULL);
  pthread_create(&T4, NULL, t4, NULL);

  cprintf("T2: ending...\n");
}

void *t3(void *arg)
{
  ssize_t x;
  char buf[MESSAGE_LENGTH];
  int prio;

  cprintf("T3: waiting a message...\n");

  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);

  // mickey mouse
  cprintf("T3: received message: length=%ld, prio=%d, text=°%s°\n",
              x,prio,buf);

  cprintf("T3: waiting t = 1.5 sec.\n");
  while (sys_gettime(NULL)<1500000);
  cprintf("T3: 2 sec. reached, sending 5 messages\n");

  if (mq_send(mq,"Goofy",6,1))
  { cprintf("T3: mq_send1 returns errno %d\n",errno); exit(1); }
  cprintf("Û");

  if (mq_send(mq,"Minnie",7,1))
  { cprintf("T3: mq_send2 returns errno %d\n",errno); exit(1); }
  cprintf("Û");

  if (mq_send(mq,"Pluto",6,2))  // NB: different priority!!!
  { cprintf("T3: mq_send3 returns errno %d\n",errno); exit(1); }
  cprintf("Û");

  if (mq_send(mq,"Rocker Duck",12,2))  // NB: different priority!!!
  { cprintf("T3: mq_send4 returns errno %d\n",errno); exit(1); }
  cprintf("Û");

  if (mq_send(mq,"Oncle Scroodge",15,2))  // NB: different priority!!!
  { cprintf("T3: mq_send5 returns errno %d\n",errno); exit(1); }
  cprintf("Û");

  cprintf("T3: ending...\n");

  return 0;
}

void t4exit(void *arg)
{
  cprintf("T4: AAAARRRRGGGHHH!!! killed by someone...\n");
}

void *t4(void *arg)
{
  ssize_t x;
  char buf[MESSAGE_LENGTH];
  int prio;

  cprintf("T4: waiting t = 2.2 sec.\n");

  while (sys_gettime(NULL)<2200000);
  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);
  cprintf("T4: received message: length=%ld, prio=%d, text=°%s°\n",x,prio,buf);

  while (sys_gettime(NULL)<2400000);
  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);
  cprintf("T4: received message: length=%ld, prio=%d, text=°%s°\n",x,prio,buf);
  while (sys_gettime(NULL)<2600000);

  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);
  cprintf("T4: received message: length=%ld, prio=%d, text=°%s°\n",x,prio,buf);
  while (sys_gettime(NULL)<2800000);

  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);
  cprintf("T4: received message: length=%ld, prio=%d, text=°%s°\n",x,prio,buf);
  while (sys_gettime(NULL)<3000000);

  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);
  cprintf("T4: received message: length=%ld, prio=%d, text=°%s°\n",x,prio,buf);

  pthread_cleanup_push(t4exit,NULL);
  x = mq_receive(mq,buf,MESSAGE_LENGTH,&prio);
  cprintf("T4: received message: length=%ld, prio=%d, text=°%s°\n",x,prio,buf);
  pthread_cleanup_pop(0);

  return 0;
}

void signal_handler(int signo, siginfo_t *info, void *extra)
{
  cprintf("Signal %d code=%s value=%d task=%d time=%ldusec\n",
              info->si_signo,
              (info->si_code == SI_TIMER) ? "Timer" : "Other",
              info->si_value.sival_int,
              info->si_task,
              sys_gettime(NULL));
}

int main(int argc, char **argv)
{
//  int err;
  struct sigaction sig_act;
  struct mq_attr attr;

  sig_act.sa_sigaction = (void *) signal_handler;
  sig_act.sa_flags = SA_SIGINFO;
  sigemptyset(&sig_act.sa_mask);
  sigaction(25, &sig_act, NULL);

  // set ev25, evtask
  ev25.sigev_notify           = SIGEV_SIGNAL;
  ev25.sigev_signo            = 25;
  ev25.sigev_value.sival_int  = 555;

  evtask.sigev_notify            = SIGEV_THREAD;
  evtask.sigev_value.sival_int   = 777;
  evtask.sigev_notify_function   = t2;
  evtask.sigev_notify_attributes = &task_attr;

  // set pthread attributes
  pthread_attr_init(&task_attr);
  pthread_attr_setdetachstate(&task_attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setschedpolicy(&task_attr, SCHED_FIFO);
  task_param.sched_priority = 10;
  pthread_attr_setschedparam(&task_attr, &task_param);

  // set mqueue attributes
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = 3;
  attr.mq_msgsize = MESSAGE_LENGTH;

  // create the message queue
  if ((mq = mq_open("mq", O_CREAT|O_RDWR, 0, &attr)) == -1)
  { cprintf("main: mq_open returns errno %d\n",errno); return 0; }

  if (mq_notify(mq, &evtask))
  { cprintf("main: mq_notify returns errno %d\n",errno); return 0; }

  cprintf("main: created mq, creating T1...\n");

  pthread_create(&T1, NULL, t1, NULL);

  cprintf("main: waiting t= 3.5 sec., then kill T4...\n");

  while (sys_gettime(NULL)<3500000);

  pthread_cancel(T4);

  cprintf("main: ending...\n");

  return 0;
}
