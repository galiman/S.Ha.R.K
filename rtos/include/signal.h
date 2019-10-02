/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
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
 CVS :        $Id: signal.h,v 1.3 2005/01/08 14:54:19 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2005/01/08 14:54:19 $
 ------------

 This file contains the Signal Handling stuffs...

 All the signals have a unique identifier through the system.

 The signals numbers are from 0 to 255. All the signals are queued as
 they are in Posix Realtime extension.

 All the modules, libraries, and task models can define a signal with
 a unique signal handler.

 It's better if the schedule levels don't define any signal, because
 in this case the application will depend to the level.
 The signal numbers have instead to be defined in the task modules...

 Index of this include file:

 - Signal codes
 - Signal handlers and data structures
 - Signal functions & primitives

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

/*
 * some functions are inspired on the implementation of OsKit..
 *
 * Copyright (c) 1997, 1998, 1999 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * [...] The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */


#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <kernel/const.h>
#include <kernel/types.h>
#include <sys/types.h>
//#include <time.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

__DJ_pid_t
#undef __DJ_pid_t
#define __DJ_pid_t


/*---------------------------------------------------------------------*/
/* Signal codes                                                        */
/*---------------------------------------------------------------------*/

/*+ Posix Signal codes: +*/
#define SIGABRT       0UL  /*+ Abnormal termination   +*/
#define SIGALRM       1UL  /*+ Alarm timeout          +*/
#define SIGFPE        2UL  /*+ Math error             +*/
#define SIGILL        3UL  /*+ Illegal opcode         +*/
#define SIGKILL       4UL  /*+ Kill!!!                +*/
#define SIGSEGV       5UL  /*+ Segmantation Violation +*/
#define SIGTERM       6UL  /*+ Termination signal     +*/
#define SIGUSR1       7UL
#define SIGUSR2       8UL

#define SIGHEXC       9UL  /*+ S.Ha.R.K. exception    +*/
#define SIGARPFULL   10UL  /*+ ARP table Full, see drivers/net/arp.c +*/


#define SIGRTMIN     11UL
#define SIGRTMAX     31UL

/*+ Maximum signal number in the system +*/
#define SIG_MAX      32UL

/*+ codes used in pthread_sigmask (alias task_sigmask) +*/
#define SIG_BLOCK      1
#define SIG_UNBLOCK    2
#define SIG_SETMASK    3

/*+ codes used in sigaction structure +*/
#define SIG_DFL (void (*)(int))0
#define SIG_IGN (void (*)(int))1
#define SIG_ERR (void (*)(int))-1


/*+ codes used in sigevent structure +*/
#define SIGEV_NONE    0
#define SIGEV_SIGNAL  1
#define SIGEV_THREAD  2


/*+ signal used by timers if the sigevent parameter is NULL +*/
#define DEFAULT_TIMER_SIGNAL 31

/*---------------------------------------------------------------------*/
/* Signal handlers and data structures                                 */
/*---------------------------------------------------------------------*/


/*+ sigval is the value passed to the exception handler +*/
union sigval {
  int    sival_int;
  void * sival_ptr;
};

/*+ siginfo_t contains some information about the signal... +*/
typedef struct siginfo {
  int          si_signo;
  int          si_code;
  union sigval si_value;
  PID          si_task;         /* the sending task... */
} siginfo_t;

/*+ this structure is used in the realtime timers +*/
struct sigevent {
  int                sigev_notify;
  int                sigev_signo;
  union sigval       sigev_value;
  void               (*sigev_notify_function)(union sigval);
  pthread_attr_t     *sigev_notify_attributes;
};

/*+ si_code values: +*/
#define SI_USER     0  /*+ signal sent by kill, alarm +*/
#define SI_QUEUE    1  /*+ signal sent by sigqueue +*/
#define SI_TIMER    2  /*+ signal generated by the expiration of a timer +*/
#define SI_ASYNCIO  3  /*+ signal gen. by the completion of an async I/O +*/
#define SI_MESGQ    4  /*+ a message is arrived on an empty queue +*/


/*+ signal Handler: there is only this definition. It corresponds to
    a Realtime Signal handler. The Posix signals are called with a dirty
    call... in effect these signals have only the signo parameter, but when
    the system call them he puts all the three parameters, and then
    the handler use only the first. It works well because the
    activation frame of the two tasks are similar!!! +*/
typedef void (*SIG_HANDLER)(int signo, siginfo_t *info, void *c);


/*+ a 32 bit signal mask.
    If modified, modify also descr.h :-( +*/
typedef int sigset_t;

/*+ sigaction structure: used with sigaction()  to set a signal handler.
    Normally, we have to set sa_flags = SA_SIGINFO and sa_sigaction =
    the signal handler.
    sa_mask, sa_handler are used only with Posix signals!!!
+*/
struct sigaction {
  void (*sa_handler)(int);
  sigset_t sa_mask;
  int sa_flags;
  SIG_HANDLER sa_sigaction;
};

/*+ bit values for the sa_flags structure +*/
#define SA_SIGINFO         1


/*---------------------------------------------------------------------*/
/* Internal stuffs                                                     */
/*---------------------------------------------------------------------*/

/* placed here because this data structures and definitions are used
   in signals *and* timer handlers
   (maybe in the near future they become another include file...
*/

/*+ a signal queue +*/
typedef int SIGQ;

/*+ the sigqueue structure is allocated to a timer +*/
#define USED_FOR_TIMER 1

/*+ the signal is posted via a sigqueue call and still pending... +*/
#define SIGNAL_POSTED    2

/*+ These are the items that go on the sigqueue. +*/
typedef struct {
  SIGQ next;
  siginfo_t info;
  int flags;
} sig_queue_entry;

/*+ We avoid malloc in interrupt handlers by preallocating the queue
    entries for sig_queued above.
    it is used also in kernel/time.c +*/
extern SIGQ                sigqueue_free;

/*+ this is the signal queue... +*/
extern sig_queue_entry     sig_queue[SIGQUEUE_MAX];

/* this version of sigqueue accept also the si_code parameter, useful
   for timers and message queues */
int
sigqueue_internal(pid_t pid, int signo, const union sigval value, int si_code);



/*---------------------------------------------------------------------*/
/* Signal functions & primitives                                       */
/*---------------------------------------------------------------------*/

/*
  Posix signals primitives:
  (they require sys/types.h and signal.h )

  IMP int kill(pid_t pid, int sig);
     . pid is a non-sense

  IMP int sigemptyset(sigset_t *set);
  IMP int sigfillset(sigset_t *set);
  IMP int sigaddset(sigset_t *set, int signo);
  IMP int sigdelset(sigset_t *set, int signo);
  IMP int sigismember(const sigset_t *set, int signo);

  IMP int sigaction(int sig, const struct sigaction *act,
                    struct sigaction *oact);

  IMP int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset);
      -> also renamed as task_sigmask;
  IMP int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

  IMP int sigpending(sigset_t *set);

  IMP int sigsuspend(const sigset_t *sigmask);

  IMP int sigwait(const sigset_t *set, int *sig);
  IMP int sigwaitinfo(const sigset_t *set, siginfo_t *info);
  IMP int sigtimedwait(const sigset_t *set, siginfo_t *info,
                       const struct timespec *timeout);

  IMP int sigqueue(pid_t pid, int signo, const union sigval value);

  IMP int pthread_kill(pthread_t thread, int sig);
      -> also renamed as task_signal

  IMP unsigned int alarm(unsigned int seconds);

  IMP int pause(void);

  IMP unsigned int sleep(unsigned int seconds);

  IMP int raise(int sig)

  IMP void (*signal(int signum, void (*handler)(int)))(int);

+*/



/*+ this function is called by __kernel_init__ and initialize the signal
    subsystem. +*/
void signals_init(void);

/*+ this function handles the pending signals...
    it is called only in kern_context_load... see func.h +*/
void kern_deliver_pending_signals();


/*+ Posix sigset_t management functions +*/
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signo);
int sigdelset(sigset_t *set, int signo);
int sigismember(const sigset_t *set, int signo);


int kill(pid_t pid, int sig);

/*+ This function allow the calling task to examine or specify (or both)
    the action to be associated with a specific signal +*/
int sigaction(int sig, const struct sigaction *act,
              struct sigaction *oact);

int task_sigmask(int how, const sigset_t *set, sigset_t *oset);

int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

int sigsuspend(const sigset_t *sigmask);

/* the sigwait & co. functions simply returns ENOSYS */
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info,
                       const struct timespec *timeout);

/*+ This function causes the signal specified by signo to be sent with the
    value specified by value to the task specified by p. it is slightly
    different to the Posix implementation +*/
int sigqueue(pid_t p, int signo, const union sigval value);

int task_signal(PID p, int signo);

/*+ raise is like task_signal on the current task... +*/
int raise(int sig);

/*+ set a signal handler; it uses internally sigaction +*/
void (*signal(int signum, void (*handler)(int)))(int);

/*+ store into *set the pending signals either for the process or the
    current task +*/
int sigpending(sigset_t *set);

/*+ stops the task until it will execute a signal handler +*/
int sigsuspend(const sigset_t *sigmask);

/*+ S.Ha.R.K. exception raise...
    This function uses sigqueue to put a signal to the task pointed
    by p.
+*/
void kern_raise(int n, PID p);



/*
 * sigqueue.
 */
extern __inline__ int
sigqueue(pid_t pid, int signo, const union sigval value)
{
  return sigqueue_internal(pid, signo, value, SI_QUEUE);
}

__END_DECLS
#endif
