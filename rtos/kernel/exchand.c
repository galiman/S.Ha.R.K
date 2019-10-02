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
 CVS :        $Id: exchand.c,v 1.9 2005/01/08 14:48:59 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.9 $
 Last update: $Date: 2005/01/08 14:48:59 $
 ------------
**/

/*
 * Copyright (C) 2000-2003 Paolo Gai
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

#include <kernel/kern.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>

static int             myflag;
static siginfo_t       myinfo;
static struct timespec mytime;

static void thehandler(int signo, siginfo_t *info, void *extra);
static void theend(void *arg);

const char *const _sys_exclist[] = {
  "not used",
  "not used",
  "invalid kill of a task with a shadow pointer set",          /* 2 */
  "cleanup push: no more cleanups handlers",
  "invalid operation for a task",
  "not used",
  "no more OSLib events posted (see oslib/kl/event.c)",
  "deadline miss",
  "wcet violation",
  "task activated at a wrong time (too early?)",
  "mutex owner killed",                                        /* 10 */
  "SRP: invalid lock",
  "Dummy task: invalid operation",
  "Sporadic Server: invalid replenishment",
  "ARP: table full",
  "Netbuff: init",
  "Netbuff: get",
  "Netbuff: already free",
  "Netbuff: release",
  "UDP: Bad checksum"
};

#define SHUTDOWN_BUFFER 1000

static char shutdown_message_buffer[SHUTDOWN_BUFFER];
static int myflag_shutdown = 0;

/*
   This exception handler should be good for text applications that do NOT
   use graphics
*/
int set_default_exception_handler(void)
{
  struct sigaction action;
  int i;

  myflag = 0;

  for(i=0;i<SHUTDOWN_BUFFER;i++) shutdown_message_buffer[i] = 0;

  sys_atrunlevel(theend, NULL, RUNLEVEL_AFTER_EXIT);

  /* Init the standard S.Ha.R.K. exception handler */
  action.sa_flags = SA_SIGINFO;            /* Set the signal action */
  action.sa_sigaction = thehandler;
  action.sa_handler = 0;
  sigfillset(&action.sa_mask); /* we block all the other signals... */

  return sigaction(SIGHEXC, &action, NULL); /* set the signal */
}

int remove_default_exception_handler(void)
{
  struct sigaction action;
                                                                                                                             
  action.sa_flags = 0;
  action.sa_sigaction = NULL;
  action.sa_handler = SIG_IGN;
  sigfillset(&action.sa_mask);
                                                                                                                             
  return sigaction(SIGHEXC, &action, NULL); /* set the signal */
}

int sys_shutdown_message(char *fmt,...)
{
  char temp[100];
  va_list parms;
  int result = -1;
  
  myflag_shutdown = 1;
  va_start(parms,fmt);
  result = vsprintf(temp,fmt,parms);
  va_end(parms);

  if ((strlen(shutdown_message_buffer) + strlen(temp)) < SHUTDOWN_BUFFER)
  	strcat(shutdown_message_buffer,temp);
    
  return(result);

}

static void thehandler(int signo, siginfo_t *info, void *extra)
{
  if (!myflag) {
    myflag = 1;
    myinfo = *info;
    sys_gettime(&mytime),
    exit(AHEXC);
  }
}

static void theend(void *arg)
{

  if (myflag) {
    cprintf("S.Ha.R.K. Exception raised!!!\n");
    cprintf("Time (s:ns)     :%ld:%ld\n", mytime.tv_sec, mytime.tv_nsec);
    cprintf("Exception number:%d (%s)\n", myinfo.si_value.sival_int, 
	    _sys_exclist[myinfo.si_value.sival_int]);
    cprintf("PID             :%d (%s)\n", myinfo.si_task, 
	    proc_table[myinfo.si_task].name);
    cprintf("Avail time      : %d\n", proc_table[myinfo.si_task].avail_time);
    
  }
  if (myflag_shutdown) {
    cprintf("\nShutdown message:\n%s\n", shutdown_message_buffer);
  }

}



