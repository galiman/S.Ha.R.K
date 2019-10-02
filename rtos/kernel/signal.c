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
 CVS :        $Id: signal.c,v 1.16 2007/05/04 10:53:30 fabio Exp $

 File:        $File$
 Revision:    $Revision: 1.16 $
 Last update: $Date: 2007/05/04 10:53:30 $
 ------------

 This file contains:

 Signal Handling

 - Data structures
 - sigset_t handling functions

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



#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/i386/pic.h>
#include <signal.h>
#include <errno.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/* look at nanoslp.c */
int nanosleep_interrupted_by_signal(PID i);


/*---------------------------------------------------------------------*/
/* Data structures                                                     */
/*---------------------------------------------------------------------*/

/*+ A flag, see kern_raise +*/
static int active_exc = 0;

/*+ The signal table... +*/
static struct sigaction    sigactions[SIG_MAX];

/*+ There is a global (or "process") set of pending signals.
    kill() and sigqueue() affect the process pending set.
+*/
static sigset_t	           procsigpending;

/*
 * A queue of all threads waiting in sigwait.
 * It is not static because it is used into the task_kill...� */
static  IQUEUE               sigwaiters;


/*+ An array of queues of pending signals posted with sigqueue(). +*/
static SIGQ                sigqueued[SIG_MAX];

/*+ We avoid malloc in interrupt handlers by preallocating the queue
    entries for sig_queued above.
    it is used also in kernel/time.c +*/
SIGQ                sigqueue_free;

/*+ this is the signal queue... +*/
sig_queue_entry     sig_queue[SIGQUEUE_MAX];

/*+ alarm stuffs +*/
static struct timespec alarm_time;
static int alarm_timer;


/* returns the first non-zero bit... */
static int ffs(int value)
{
  int x;

  for (x=0; value; x++, value = value>>1)
    if (value & 1)
      return x;
  return 0;
}

/*---------------------------------------------------------------------*/
/* interruptable function registration...                              */
/*---------------------------------------------------------------------*/


/*+ this structure contains the functions to be called to test if a
    task is blocked on a cancellation point +*/
static struct {
  int (*test)(PID p, void *arg);
  void *arg;
} interruptable_table[MAX_SIGINTPOINTS];

static int interruptable_points = 0;


/*+ This function register a cancellation point into the system.
    Be careful!!! no check are performed... +*/
void register_interruptable_point(int (*func)(PID p, void *arg), void *arg)
{
  interruptable_table[interruptable_points].test = func;
  interruptable_table[interruptable_points].arg = arg;
  interruptable_points++;
}

static void test_interruptable_points(PID i)
{
  int j;

  /* check if the task is blocked on a cancellation point */
  for (j=0; j<interruptable_points; j++)
     if (interruptable_table[j].test(i,interruptable_table[j].arg))
       break;
}


/*---------------------------------------------------------------------*/
/* sigset_t handling functions                                         */
/*---------------------------------------------------------------------*/

/* These functions will become soon macros... */
int sigemptyset(sigset_t *set)
{
  *set = 0;

  return 0;
}

int sigfillset(sigset_t *set)
{
  *set=0xFFFFFFFFUL;

  return 0;
}

int sigaddset(sigset_t *set, int signo)
{
  if (signo < 0 || signo >= SIG_MAX)
  {
    errno = EINVAL;
    return -1;
  }

  *set |= 1 << signo;
  return 0;
}


int sigdelset(sigset_t *set, int signo)
{
  if (signo < 0 || signo >= SIG_MAX)
  {
    errno = EINVAL;
    return -1;
  }

  *set &= ~(1 << signo);
  return 0;
}

int sigismember(const sigset_t *set, int signo)
{
  if (signo < 0 || signo >= SIG_MAX)
  {
    errno = EINVAL;
    return -1;
  }

  return *set & (1 << signo );
}


/*---------------------------------------------------------------------*/
/* Finally, the public functions                                       */
/*---------------------------------------------------------------------*/

/*
 * Prototypes.
 */
void	really_deliver_signal(int sig, siginfo_t *code);
void    kern_deliver_async_signal(int sig);
void	kern_deliver_process_signal(int sig);

int task_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
        proc_des *task;    /* current executing task... */
	int      err = 0;
        SYS_FLAGS f;

        f = kern_fsave();

        task = &proc_table[exec_shadow];

	if (oset)
		*oset = task->sigmask;

	if (set) {
		switch (how) {
		case SIG_BLOCK:
			task->sigmask |= *set;
			break;
		case SIG_UNBLOCK:
			task->sigmask &= ~*set;
			break;
		case SIG_SETMASK:
			task->sigmask = *set;
			break;
		default:
			err = EINVAL;
		}
	}

	/*
	 * Look for process pending signals that are unblocked, and deliver.
	 */
	while (procsigpending & ~task->sigmask) {
		int sig = ffs(procsigpending & ~task->sigmask);
		kern_deliver_process_signal(sig);
	}

	/*
	 * Look for task pending signals that are unblocked, and deliver.
	 */
	while (task->sigpending & ~task->sigmask) {
		int sig = ffs(task->sigpending & ~task->sigmask);
		kern_deliver_async_signal(sig);
	}

        kern_frestore(f);
	return err;
}

/*
 * This can be called out of an interrupt handler, say from an alarm
 * expiration.
 */
int
task_signal(PID p, int signo)
{
//	int       enabled;
        SYS_FLAGS f;
 
	/* Error check? Sure! */
	if (!signo)
		return 0;

	if (signo < 0 || signo >= SIG_MAX)
		return EINVAL;

	if (proc_table[p].status == FREE)
		return EINVAL;

        f = kern_fsave();

	/*
	 * Look at the process sigactions. If the "process" is ignoring
	 * the signal, then the signal is not placed in the pending list.
	 */
	if (!(sigactions[signo].sa_flags & SA_SIGINFO) &&
            sigactions[signo].sa_handler == SIG_IGN) {
                kern_frestore(f);
		return 0;
        }

        /*
	 * Add the signal to list of pending signals for the target task.
	 */
	sigaddset(&proc_table[p].sigpending, signo);

        /* check for an interruptable function!!! */
        test_interruptable_points(p);

        if (proc_table[p].status == WAIT_SIGSUSPEND) {
            LEVEL l;

            /* Reactivate the task... */
            iq_extract(p, &sigwaiters);

            l = proc_table[p].task_level;
            level_table[l]->public_unblock(l,p);

        }


	/*
	 * If not in an interrupt, use this opportunity to deliver
	 * pending unblocked signals to the current thread.
	 */
	if (!ll_ActiveInt()) {
		kern_deliver_pending_signals();
	}

        kern_frestore(f);
	return 0;
}

/*
 * sigaction
 */
int
sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
        int sos; /* used to empty the sigqueue... */
        SYS_FLAGS f;


	if (sig < 0 || sig >= SIG_MAX)
		return errno = EINVAL, -1;

        f = kern_fsave();

	if (oact)
		*oact = sigactions[sig];
	if (act)
		sigactions[sig] = *act;

	/*
	 * If the action for this signal is being set to SIG_IGN or SIG_DFL,
	 * and that signal is process pending, then clear it.
	 */
	if (act && !(act->sa_flags & SA_SIGINFO) &&
            (act->sa_handler == SIG_IGN || act->sa_handler == SIG_DFL)) {
                sos = sigqueued[sig];
		while (sos != -1) {
                        /* Remove the first entry and put it to the free
                           queue */
                        sos = sig_queue[sigqueued[sig]].next;

                        if (sig_queue[sigqueued[sig]].flags & USED_FOR_TIMER)
                          sig_queue[sigqueued[sig]].flags &= ~SIGNAL_POSTED;
                        else {
                          sig_queue[sigqueued[sig]].next = sigqueue_free;
                          sigqueue_free = sigqueued[sig];
                        }
                }
                sigqueued[sig] = -1;
		sigdelset(&procsigpending, sig);
	}
	
	kern_frestore(f);
	return 0;
}

/*
 * sigprocmask. this is just task_sigmask
 */
int
sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	return task_sigmask(how, set, oset);
}

/*
 * raise. this is just task_signal on itself.
 */
int
raise(int sig)
{
	return task_signal(exec_shadow, sig);
}

/*
 * kill. What does it mean to kill() in a multithreaded program? The POSIX
 * spec says that a signal sent to a "process" shall be delivered to only
 * one task. If no task has that signal unblocked, then the first
 * task to unblock the signal is the lucky winner. Well, that means we
 * need to have a global procsigpending to record process pending signals.
 */
int
kill(pid_t pid, int signo)
{
        PID                     task;
	PID			i;
        SYS_FLAGS               f;
	struct sigaction	act;

	/* Error check? Sure! */
	if (!signo)
		return 0;

	if (signo < 0 || signo >= SIG_MAX)
		return EINVAL;


        f = kern_fsave();

	act = sigactions[signo];
	
	if (!(act.sa_flags & SA_SIGINFO) && act.sa_handler == SIG_IGN) {
		kern_frestore(f);
		return 0;
	}

        /*
	 * Kill does not queue. If the signal is already pending, this
	 * one is tossed.
	 */
	if (sigismember(&procsigpending, signo)) {
		kern_frestore(f);
		return 0;
	}

	/*
	 * Make the signal process pending.
	 */
	sigaddset(&procsigpending, signo);

	/*
	 * Look through the threads in sigwait to see if any of them
	 * is waiting for the signal. This is done as a separate pass
	 * since the value of the pthread sigmask is ignored (threads
	 * in sigwait will have blocked the signals being waited for).
	 */

        for (task = iq_query_first(&sigwaiters);
             task != NIL;
             task = iq_query_next(task, &sigwaiters)) {
          if (sigismember(&proc_table[task].sigwaiting, signo)) {
            LEVEL l;

            if (proc_table[task].status == WAIT_SIGSUSPEND)
               sigaddset(&proc_table[task].sigpending, signo);

            /* Reactivate the task... */
            iq_extract(task, &sigwaiters);
            l = proc_table[task].task_level;
            level_table[l]->public_unblock(l,task);

            if (proc_table[task].delay_timer != -1) {
              kern_event_delete(proc_table[task].delay_timer);
              proc_table[task].delay_timer = -1;
            }

            kern_frestore(f);
            return 0;
          }
        }

	/*
	 * No threads in sigwait. Too bad. Must find another thread to
	 * deliver it to.
	 */
	for (i = 1; i < MAX_PROC; i++) {
		if (proc_table[i].status != FREE) {
			if (! sigismember(&proc_table[i].sigmask, signo)) {
                                /* Add the signal to list of pending
                                   signals for the target task. */
                                sigaddset(&proc_table[i].sigpending, signo);

                                /* check for an interruptable function!!! */
                                test_interruptable_points(i);
				break;
			}
		}
	}

        /*
	 * If not in an interrupt, use this opportunity to deliver
	 * pending unblocked signals to the current thread.
	 */
	if (! ll_ActiveInt()) {
		kern_deliver_pending_signals();
	}

        kern_frestore(f);
	return 0;
}

/*
 * sigqueue internal: accept also the SI_XXX value
 */
int
sigqueue_internal(pid_t pid, int signo, const union sigval value, int si_code)
{
        PID                     task;
	SYS_FLAGS		f;
	int			i;

	int	                thingie; /* an element of the signal queue */
        int                     sos;     /* used when inserting thinghie in
                                            the signal queue */
	struct sigaction	act;

	/* Error check? Sure! */
	if (!signo)
		return 0;

	if (signo < 0 || signo >= SIG_MAX)
		return EINVAL;


        f = kern_fsave();
	/*
	 * Look at the process sigactions. If the "process" is ignoring
	 * the signal, then the signal is not placed in the pending list.
	 */
	act = sigactions[signo];
	
	if (!(act.sa_flags & SA_SIGINFO) && act.sa_handler == SIG_IGN) {
		kern_frestore(f);
		return 0;
	}


        /*
	 * If the flags does not include SA_SIGINFO, and there is already
	 * a signal pending, this new one is dropped.
	 */
	if ((! (act.sa_flags & SA_SIGINFO)) &&
	    sigismember(&procsigpending, signo)) {
		kern_frestore(f);
		return 0;
	}

	/*
	 * Gotta have space for the new signal.
	 */
	if (sigqueue_free == -1) {
		kern_frestore(f);
		return EAGAIN;
	}

	/*
	 * Create a queue entry.
	 */
	thingie = sigqueue_free;
        sigqueue_free = sig_queue[sigqueue_free].next;

	sig_queue[thingie].info.si_signo = signo;
        sig_queue[thingie].info.si_code  = si_code;
	sig_queue[thingie].info.si_value = value;
        sig_queue[thingie].info.si_task  = exec_shadow;
	sig_queue[thingie].next          = -1;

	/*
	 * Queue the signal on the process.
	 */

        /* we insert the signal at the queue's tail */
        if (sigqueued[signo] == -1)
                sigqueued[signo] = thingie;
        else {
                sos = sigqueued[signo];
                while (sig_queue[sos].next != -1) sos = sig_queue[sos].next;
                sig_queue[sos].next = thingie;
        }
	sigaddset(&procsigpending, signo);

	/*
	 * Look through the threads in sigwait to see if any of them
	 * is waiting for the signal. This is done as a separate pass
	 * since the value of the pthread sigmask is ignored (threads
	 * in sigwait will have blocked the signals being waited for).
	 * If we find one, wakeup that thread. Note that POSIX says that
	 * if multiple threads are sigwaiting for the same signal number,
	 * exactly one thread is woken up. The problem is how to maintain
	 * the FIFO order, and how to prevent lost signals in the case that
	 * a thread calls sigwait before the woken thread runs and gets it.
	 */
        for (task = iq_query_first(&sigwaiters);
             task != NIL;
             task = iq_query_next(task, &sigwaiters)) {
          if (sigismember(&proc_table[task].sigwaiting, signo)) {
            LEVEL l;

            if (proc_table[task].status == WAIT_SIGSUSPEND)
               sigaddset(&proc_table[task].sigpending, signo);

            /* Reactivate the task... */
            iq_extract(task, &sigwaiters);

            l = proc_table[task].task_level;
            level_table[l]->public_unblock(l,task);

            if (proc_table[task].delay_timer != -1) {
              kern_event_delete(proc_table[task].delay_timer);
              proc_table[task].delay_timer = -1;
            }

            kern_frestore(f);
            return 0;

          }
        }

	/*
	 * Need to find a thread to deliver the signal to. Look for the
	 * first thread that is not blocking the signal, and send it the
	 * signal. It is my opinion that any program that is using sigwait,
	 * and has not blocked signals in all of its threads, is bogus. The
	 * same is true if the program is not using sigwait, and has the
	 * signal unblocked in more than one thread.
	 * Why? You might wake up a thread, but not have an actual queue
	 * entry left by the time it runs again and looks, since another
	 * thread could call sigwait and get that queue entry, or if there
	 * are multiple threads that can take the signal, one thread could
	 * get all the entries. This could result in an interrupted thread,
	 * but with no signal to deliver. Well, not much to do about it.
	 * Lets just queue the signal for the process, and let the chips
	 * fall where they may. 
	 */
	for (i = 1; i < MAX_PROC; i++) {
		if (proc_table[i].status != FREE) {
			if (! sigismember(&proc_table[i].sigmask, signo)) {
                                /* Add the signal to list of pending
                                   signals for the target task. */
                                sigaddset(&proc_table[i].sigpending, signo);

                                /* check for an interruptable function!!! */
                                test_interruptable_points(i);

				break;
			}
		}
	}

        /*
	 * If not in an interrupt, use this opportunity to deliver
	 * pending unblocked signals to the current thread.
         * (NB: a discussion on the flag active_exc is near the function
         * kern_raise() )
	 */
	if (! ll_ActiveInt() && active_exc == 0) {
		kern_deliver_pending_signals();
	}

        kern_frestore(f);
	return 0;
}

static void sigwait_timer(void *arg)
{
  PID p = (PID)arg;
  LEVEL l;

  /* reset the event timer */
  proc_table[p].delay_timer = -1;

  /* set the timeout flag */
  proc_table[p].control |= SIGTIMEOUT_EXPIRED;

  /* insert the task into the ready queue and extract it from the waiters */
  iq_extract(p, &sigwaiters);

  l = proc_table[p].task_level;
  level_table[l]->public_unblock(l,p);

  event_need_reschedule();
}

/*
 * Sigwait. Sigwait overrides the state of the pthread sigmask and the global
 * sigactions. The caller *must* block the set of signals in "set", before
 * calling sigwait, otherwise the behaviour is undefined (which means that
 * the caller will take an async signal anyway, and sigwait will return EINTR.
 */
int
kern_sigwait_internal(const sigset_t *set,
                      siginfo_t *info, const struct timespec *timeout)
{
	proc_des *pthread = &proc_table[exec_shadow];
	int			thissig;

        LEVEL l;
        SYS_FLAGS f;

	task_testcancel();

	/* siglock and pthread siglock are taken from an interrupt handler */
        f = kern_fsave();

	/*
	 * First check for process pending signals. Must take and hold
	 * the global siglock to prevent races with kill() and sigqueue().
	 */
	if (procsigpending & *set) {
		int sos;
		
		thissig = ffs(procsigpending & *set);

		/*
		 * Sent with kill(). Using sigwait and kill is Bogus!
		 */
		if (sigqueued[thissig] == -1) {
			info->si_signo           = thissig;
			info->si_code            = SI_USER;
			info->si_value.sival_int = 0;

			sigdelset(&pthread->sigpending, thissig);
			sigdelset(&procsigpending, thissig);
			kern_frestore(f);
			return 0;
		}

		/*
		 * Grab the first queue entry.
		 */
                sos = sigqueued[thissig];
                sigqueued[thissig] = sig_queue[sos].next;

		/*
		 * If that was the last one, reset the process procsigpending.
		 */
		if (sigqueued[thissig] == -1)
			sigdelset(&procsigpending, thissig);
		sigdelset(&pthread->sigpending, thissig);

		/*
		 * Copy the information and free the queue entry.
		 */
		info->si_signo           = sig_queue[sos].info.si_signo;
		info->si_code            = sig_queue[sos].info.si_code;
		info->si_value.sival_int = sig_queue[sos].info.si_value.sival_int;

                if (sig_queue[sos].flags & USED_FOR_TIMER)
                  sig_queue[sos].flags &= ~SIGNAL_POSTED;
                else {
                  sig_queue[sos].next = sigqueue_free;
                  sigqueue_free = sos;
                }
		kern_frestore(f);
		return 0;
	}
		
	/*
	 * Now check for pthread pending signals.
	 */
	if (pthread->sigpending & *set) {
		thissig = ffs(pthread->sigpending & *set);
		info->si_signo           = thissig;
		info->si_code            = SI_USER;
		info->si_value.sival_int = 0;
		sigdelset(&pthread->sigpending, thissig);
		kern_frestore(f);
		return 0;
	}

	/*
	 * For timed wait, if nothing is available and the timeout value
	 * is zero, its an error.
	 */
	if (timeout && timeout->tv_sec == 0 && timeout->tv_nsec == 0) {
                kern_frestore(f);
		return EAGAIN;
	}

	/*
	 * Grab the wait lock and set the sigwaiting mask. Once that is done,
	 * release the thread siglock; Another thread can try and wake this
	 * thread up as a result of seeing it in sigwait, but the actual
	 * wakeup will be delayed until the waitlock is released in the switch
	 * code.
	 */
	pthread->sigwaiting = *set;

        /* now, we really block the task... */
        proc_table[exec_shadow].context = kern_context_save();
      
	kern_epilogue_macro();

        l = proc_table[exec_shadow].task_level;
        level_table[l]->public_block(l,exec_shadow);
      
	/*
	 * Add this thread to the list of threads in sigwait. Once that is
	 * done, it is safe to release the global siglock, which will allow
	 * another thread to scan the sigwaiters list. As above, it might
	 * find a thread in sigwait, but it will not be able to wake it up
	 * until the waitlock is released in the switch code.
	 */
	iq_insertfirst(exec_shadow, &sigwaiters);
        proc_table[exec_shadow].status = WAIT_SIG;

        if (timeout) {
          /* we can use the delaytimer because if we are here we are not in a
             task_delay */
          struct timespec t, abstime;
          kern_gettime(&t);
          ADDTIMESPEC(&t, timeout, &abstime);

          proc_table[exec_shadow].delay_timer =
            kern_event_post(&abstime,sigwait_timer,(void *)exec_shadow);
        }
      
        /* and finally we reschedule */
        exec = exec_shadow = -1;
        scheduler();
        ll_context_to(proc_table[exec_shadow].context);

        task_testcancel();

	pthread->sigwaiting = 0;

	/*
	 * Look for timeout.
	 */
	if (proc_table[exec_shadow].control & SIGTIMEOUT_EXPIRED) {
                kern_frestore(f);
		return EAGAIN;
	}

	/*
	 * Look for a wakeup to deliver a queued signal. This would come
	 * either from kill() or from sigqueue().
	 */
	if (procsigpending & *set) {
		int sos;
		
		thissig = ffs(procsigpending & *set);

		/*
		 * Sent with kill(). Using sigwait and kill is Bogus!
		 */
		if (sigqueued[thissig] == -1) {
			info->si_signo           = thissig;
			info->si_code            = SI_USER;
			info->si_value.sival_int = 0;

			sigdelset(&procsigpending, thissig);
			kern_frestore(f);
			return 0;
		}

		/*
		 * Grab the first queue entry.
		 */
                sos = sigqueued[thissig];
                sigqueued[thissig] = sig_queue[sos].next;

		/*
		 * If that was the last one, reset the process procsigpending.
		 */
		if (sigqueued[thissig] == -1)
			sigdelset(&procsigpending, thissig);

		/*
		 * Copy the information and free the queue entry.
		 */
		info->si_signo           = sig_queue[sos].info.si_signo;
		info->si_code            = sig_queue[sos].info.si_code;
		info->si_value.sival_int = sig_queue[sos].info.si_value.sival_int;

                if (sig_queue[sos].flags & USED_FOR_TIMER)
                  sig_queue[sos].flags &= ~SIGNAL_POSTED;
                else {
                  sig_queue[sos].next = sigqueue_free;
                  sigqueue_free = sos;
                }

                kern_frestore(f);
		return 0;
	}
	
	/*
	 * Well, at the moment I am going to assume that if this thread
	 * wakes up, and there is no signal pending in the waitset, the
	 * thread wait was interrupted for some other reason. Return EINTR.
	 */
	if (! (pthread->sigpending & *set)) {
                kern_frestore(f);
		return EINTR;
	}

	/*
	 * Otherwise, get the first signal and return it.
	 */
	thissig = ffs(pthread->sigpending & *set);
	info->si_signo           = thissig;
	info->si_code            = SI_USER;
	info->si_value.sival_int = 0;
	sigdelset(&pthread->sigpending, thissig);
        kern_frestore(f);
	return 0;
}

/*
 * Sigwait.
 */
int
sigwait(const sigset_t *set, int *sig)
{
	siginfo_t       info;
	int             rc;

	memset(&info, 0, sizeof(info));

	rc = kern_sigwait_internal(set, &info, 0);

	if (rc)
		return rc;

	*sig = info.si_signo;
	return 0;
}

/*
 * Sigwaitinfo. 
 */
int
sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	return kern_sigwait_internal(set, info, 0);
}

/*
 * Sigtimedwait.
 */
int
sigtimedwait(const sigset_t *set,
	     siginfo_t *info, const struct timespec *timeout)
{
	if (! timeout)
		return EINVAL;
	
	return kern_sigwait_internal(set, info, timeout);
}

/*
 * Signal
 */
void (*signal(int signum, void (*handler)(int)))(int)
{
  struct sigaction act, oact;
  int olderrno;
  void (*retvalue)(int);

  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  olderrno = errno;
  if (sigaction(signum, &act, &oact))
    retvalue = SIG_ERR;
  else
    if (oact.sa_flags & SA_SIGINFO)
      retvalue = SIG_ERR;
    else
      retvalue = oact.sa_handler;

  errno = olderrno;

  return retvalue;

}


/*
 * sigpending
 */
int sigpending(sigset_t *set)
{
  *set = procsigpending | proc_table[exec_shadow].sigpending;
  return 0;
}


/*
 * sigsuspend
 */
int sigsuspend(const sigset_t *set)
{
	proc_des *pthread = &proc_table[exec_shadow];
	SYS_FLAGS f;
        LEVEL l;

	task_testcancel();

        f = kern_fsave();
		
	/*
	 * Now check for pthread pending signals.
	 */
	if (pthread->sigpending & *set) {
          kern_deliver_pending_signals();
          kern_frestore(f);
	  return 0;
	}

	/*
	 * Grab the wait lock and set the sigwaiting mask. Once that is done,
	 * release the thread siglock; Another thread can try and wake this
	 * thread up as a result of seeing it in sigwait, but the actual
	 * wakeup will be delayed until the waitlock is released in the switch
	 * code.
	 */
	pthread->sigwaiting = *set;

        /* now, we really block the task... */
        proc_table[exec_shadow].context = kern_context_save();
      
	kern_epilogue_macro();
        l = proc_table[exec_shadow].task_level;
        level_table[l]->public_block(l,exec_shadow);

	iq_insertfirst(exec_shadow, &sigwaiters);
        proc_table[exec_shadow].status = WAIT_SIGSUSPEND;

        /* and finally we reschedule */
        exec = exec_shadow = -1;
        scheduler();
        ll_context_to(proc_table[exec_shadow].context);

        task_testcancel();
	
	/*
	 * Well, at the moment I am going to assume that if this thread
	 * wakes up, and there is no signal pending in the waitset, the
	 * thread wait was interrupted for some other reason. Return EINTR.
	 */
	if (! (pthread->sigpending & *set)) {
                kern_frestore(f);
		return EINTR;
	}

	/*
	 * Otherwise, deliver the signals.
	 */
        kern_deliver_pending_signals();
        kern_frestore(f);
	return 0;
}


void timer_alarmfire(void *arg)
{
  alarm_timer = -1;

  kill(0, SIGALRM);

  event_need_reschedule();
}

/*
 * alarm
 */
unsigned int alarm(unsigned int seconds)
{
  struct timespec returnvalue, temp;
  SYS_FLAGS f;

  f = kern_fsave();

  kern_gettime(&temp);

  if (alarm_timer == -1)
    returnvalue.tv_sec = 0;
  else {
    SUBTIMESPEC(&alarm_time, &temp, &returnvalue);

    kern_event_delete(alarm_timer);
  }

  if (seconds) {
    temp.tv_sec += seconds;
    TIMESPEC_ASSIGN(&alarm_time, &temp);
    alarm_timer = kern_event_post(&temp, timer_alarmfire, NULL);
  }
  else
    alarm_timer = -1;

  kern_frestore(f);

  return returnvalue.tv_sec;
}

int pause(void)
{
  sigset_t set;

  sigfillset(&set);
  return sigsuspend(&set);
}

/*
 * Internal stuff.
 */

/*
 * Deliver an asynchronous signal. This must be called with interrupts
 * blocked and the pthread siglock held. 
 */
void
kern_deliver_async_signal(int sig)
{
	siginfo_t		siginfo;

	siginfo.si_signo           = sig;
	siginfo.si_code            = SI_USER;
	siginfo.si_value.sival_int = 0;
        siginfo.si_task            = exec_shadow;

	really_deliver_signal(sig, &siginfo);
}

/*
 * Deliver a process signals. This must be called with interrupts
 * blocked and the siglock and pthread siglock held.
 */
void
kern_deliver_process_signal(int sig)
{
	siginfo_t		siginfo;
	int                     thingie;

	/*
	 * Sent with kill(). Using sigwait and kill is Bogus!
	 */
	if (sigqueued[sig] == -1) {
		siginfo.si_signo           = sig;
		siginfo.si_code            = SI_USER;
		siginfo.si_value.sival_int = 0;
                siginfo.si_task            = exec_shadow;

		sigdelset(&procsigpending, sig);
		goto deliver;
	}

	/*
	 * Grab the first queue entry.
	 */
        thingie = sigqueued[sig];
        sigqueued[sig] = sig_queue[sigqueued[sig]].next;

	/*
	 * If that was the last one, reset the process sigpending.
	 */
	if (sigqueued[sig] == -1)
		sigdelset(&procsigpending, sig);

	/*
	 * Copy the information and free the queue entry.
	 */
	siginfo.si_signo           = sig_queue[thingie].info.si_signo;
	siginfo.si_code            = sig_queue[thingie].info.si_code;
	siginfo.si_value.sival_int = sig_queue[thingie].info.si_value.sival_int;
        siginfo.si_task            = sig_queue[thingie].info.si_task;

        if (sig_queue[thingie].flags & USED_FOR_TIMER)
          sig_queue[thingie].flags &= ~SIGNAL_POSTED;
        else {
          sig_queue[thingie].next = sigqueue_free;
          sigqueue_free = thingie;
        }

 deliver:
	really_deliver_signal(sig, &siginfo);

}

/*
 * Deliver any pending signals. Called out of the context switch code
 * when a task switches in, and there are pending signals.
 *
 * Interrupts are blocked...
 */
void
kern_deliver_pending_signals(void)
{
        proc_des *task;    /* current executing task... */

        task = &proc_table[exec_shadow];

        /* we have to check if the task was descheduled while serving
           signals... if so, it is useless the call to this function...
           because the task is already in it!!! (NB: the task can be
           descheduled because the signal handlers are executed with
           interrupts enabled...) */
        if (task->control & TASK_DOING_SIGNALS)
          return;

        task->control |= TASK_DOING_SIGNALS;

	/*
	 * Look for process pending signals that are unblocked, and deliver.
	 */
	while (procsigpending & ~task->sigmask) {
                /* NB: the while test should be indipendent from any local
                   variable... because when we process signals there can be
                   some context_change before we return from the
                   kern_deliver-signals...
                */
		int sig = ffs(procsigpending & ~task->sigmask);
	
		/* Call with siglock and thread siglock locked */
		kern_deliver_process_signal(sig);
	}

	/*
	 * Now deliver any pthread pending signals that are left.
         * NB: the pthread pending signals are NOT sent via sigqueue!!!
	 */
	while (task->sigpending & ~task->sigmask) {
		int sig = ffs(task->sigpending & ~task->sigmask);

		/* Call at splhigh and thread locked */
		kern_deliver_async_signal(sig);
	}
        task->control &= ~TASK_DOING_SIGNALS;
}

/*
 * Actually deliver the signal to the task. At this point the signal
 * is going to be delivered, so it no longer matters if it is blocked.
 */
void
really_deliver_signal(int sig, siginfo_t *info)
{
        proc_des *task;    /* current executing task... */

	sigset_t		sigmask, oldmask;
	struct sigaction	act;
	SYS_FLAGS               f;

        f = kern_fsave();

        task = &proc_table[exec_shadow];

	act = sigactions[sig];

        //kern_printf("Ci sono!!!flags=%d hand=%d sigaction=%d mask=%d",act.sa_flags,
        //          (int)act.sa_handler, (int)act.sa_sigaction, (int)act.sa_mask);

	/*
	 * Ignored?
	 */
	if (!(act.sa_flags & SA_SIGINFO) && (act.sa_handler == SIG_IGN ||
                                             act.sa_handler == SIG_ERR)  ) {
	    sigdelset(&task->sigpending, sig);
            kern_frestore(f);
            return;
        }

	if (!(act.sa_flags & SA_SIGINFO) && act.sa_handler == SIG_DFL) {
		/* Default action for all signals is termination */
                //kern_printf("\nSignal number %d...\n",sig);
                if (act.sa_flags & SA_SIGINFO)
                  //kern_printf("with value : %d\n",info->si_value.sival_int);
		exit(ASIG_DEFAULT_ACTION);
	}

	/*
	 * Set the signal mask for calling the handler.
	 */
	oldmask = sigmask = task->sigmask;
	sigaddset(&sigmask, sig);
	sigmask |= act.sa_mask;
	sigdelset(&task->sigpending, sig);
	task->sigmask = sigmask;

        kern_sti();
	/*
	 * and call the handler ...
	 */
	if (act.sa_flags & SA_SIGINFO)
		act.sa_sigaction(sig, info, NULL);
	else
		((void (*)(int, int, void *))act.sa_handler)
			(sig, info->si_value.sival_int, NULL);

        /* NB: when we pass the kern_cli(), there can be the case that
           an irq (and/or a timer...) fired... and do a context change.
           so, we return here after an indefinite time... */
	kern_cli();
        task->sigmask = oldmask;

        kern_frestore(f);
}


/*---------------------------------------------------------------------*/
/* S.HA.R.K. exceptions handling                                       */
/*---------------------------------------------------------------------*/

void kern_raise(int n, PID p)
{
  union sigval v;
  SYS_FLAGS f;
  PID sos;         /* temp. PID */

  v.sival_int = n;
  //kern_printf("RAISE");

  /* sigqueue set the p field to exec_shadow... so whe change it for a
     little... because sigqueue fill descriptor with exec_shadow... */
  f = kern_fsave();
  sos = exec_shadow;
  exec_shadow = p;

  active_exc = 1;  // see (*)
  sigqueue(0, SIGHEXC, v);
  active_exc = 0;

  exec_shadow = sos;
  kern_frestore(f);

  /* (*)
     when we are in an exception, we don't have to call the
     really_deliver signal.
     For example, when the capacity of a task is exausted, an OSLib event is
     called. this event simply call scheduler, that call the public_epilogue.

     the public_epilogue checks the capacity and raise an exception, BUT
     we don't have to deliver this exception immediately.

     Why? because the task pointed by exec_shadow was extracted from the
     ready queue (as sigqueue do normally...) and the exception does not have
     to be delivered to that task. It must be delivered
     only after we exit from the kern_raise (because the signal handler
     in SIGHEXC may be long and another timer interrupt can fire...), to
     another task...
  */

}


/*---------------------------------------------------------------------*/
/* S.Ha.R.K. interrupts handling                                       */
/*---------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Interrupt table management. The following function install the fast  */
/* handler and the sporadic task linked to the interrupt no.            */
/* If the fast parameter is NULL, no handler is called.                 */
/* If the pi parameter is NIL no task is installed                      */
/*----------------------------------------------------------------------*/

/* Interrupt handling table */
static struct int_des {
	void (*fast)(int n);
	void (*intdrv)(int n);
	PID proc_index;
	BYTE isUsed;
	BYTE irqLock;
} int_table[16];

/* Warning the interrupt can cause a preemption!                */
/* The fast handler is a standard piece of code which runs with */
/* interrupts enabled to allow interrupt nesting                */

extern int add_interrupt_job(int no);
extern int invalidate_pending_jobs(int no);

void irq_fasthandler(void *n)
{
	int no = *(int *)n;
	PID p;

	//kern_printf("(irq_fasthandler: no %d)",no);
	/* tracer stuff */
	TRACER_LOGEVENT(FTrace_EVT_interrupt_start,(unsigned short int)no,0);
  
	if (int_table[no].fast != NULL) {
		if (int_table[no].irqLock == FALSE)
			kern_sti();
		(int_table[no].fast)(no);
		if (int_table[no].irqLock == FALSE)
			kern_cli();
	}

	if (int_table[no].intdrv != NULL) {
		add_interrupt_job(no);
	}

	TRACER_LOGEVENT(FTrace_EVT_interrupt_end,(unsigned short int)no,0);

	/* If a sporadic process is linked,activate it */
	p = int_table[no].proc_index;
	task_activate(p); // no problem if p == nil
}

/*----------------------------------------------------------------------*/
/* Interrupt table management. The following function install the fast  */
/* handler and the sporadic task linked to the interrupt no.            */
/* If the fast parameter is NULL, no handler is called.                 */
/* If the pi parameter is NIL no task is installed                      */
/*----------------------------------------------------------------------*/
int handler_set(int no, void (*fast)(int n), BYTE lock, PID pi, void (*intdrv)(int n))
{
	SYS_FLAGS f;

	if ((no < 1) || (no > 15)) {
		errno = EWRONG_INT_NO;
		return -1;
	}

	f = kern_fsave();

	//kern_printf("(handler_set: no %d pid %d)",no, pi);

	if (int_table[no].isUsed == TRUE) {
		kern_frestore(f);
		errno = EUSED_INT_NO;
		return -1;
	}
	int_table[no].fast = fast;
	int_table[no].intdrv = intdrv;
	int_table[no].proc_index = pi;
	int_table[no].isUsed = TRUE;
	int_table[no].irqLock = lock;

	irq_bind(no, irq_fasthandler, INT_FORCE);
	irq_unmask(no);
	kern_frestore(f);

	return 1;
}

int handler_remove(int no)
{
	SYS_FLAGS f;

	if (no < 1 || no > 15) {
		errno = EWRONG_INT_NO;
		return -1;
	}

	f = kern_fsave();

	//kern_printf("(handler_remove: no %d )",no);

	if (int_table[no].isUsed == FALSE) {
		kern_frestore(f);
		errno = EUNUSED_INT_NO;
		return -1;
	}

	if (int_table[no].intdrv != NULL)
		invalidate_pending_jobs(no);

	int_table[no].fast = NULL;
	int_table[no].intdrv = NULL;
	int_table[no].proc_index = NIL;
	int_table[no].isUsed = FALSE;
	int_table[no].irqLock = FALSE;

	irq_bind(no,NULL, INT_PREEMPTABLE);
	irq_mask(no);
	kern_frestore(f);

	return 1;
}

void *handler_get_intdrive(int no)
{
	return int_table[no].intdrv;
}

/* this is the test that is done when a task is being killed
   and it is waiting on a sigwait */
static int signal_cancellation_point(PID i, void *arg)
{
	LEVEL l;

	if (proc_table[i].status == WAIT_SIG) {

		if (proc_table[i].delay_timer != -1) {
			kern_event_delete(proc_table[i].delay_timer);
			proc_table[i].delay_timer = -1;
		}

		iq_extract(i, &sigwaiters);

		l = proc_table[i].task_level;
		level_table[l]->public_unblock(l,i);

		return 1;
	} else if (proc_table[i].status == WAIT_SIGSUSPEND) {
		l = proc_table[i].task_level;
		level_table[l]->public_unblock(l,i);

		return 1;
	}

	return 0;
}

void signals_init()
{
	int i;

	/* Initialize the default signal actions and the signal queue headers. */
	for (i = 0; i < SIG_MAX; i++) {
		sigactions[i].sa_handler = SIG_DFL;
		sigactions[i].sa_flags = 0;
		sigactions[i].sa_mask = 0;
		sigactions[i].sa_sigaction = 0;
		sigqueued[i] = -1;
	}

	/* Initialize the signal queue */
	for (i=0; i < SIGQUEUE_MAX-1; i++) {
		sig_queue[i].next = i+1;
		sig_queue[i].flags = 0;
	}
	sig_queue[SIGQUEUE_MAX-1].next = NIL;
	sig_queue[SIGQUEUE_MAX-1].flags = 0;
	sigqueue_free = 0;

	procsigpending = 0;

	iq_init(&sigwaiters, &freedesc, 0);
	alarm_timer = -1;

	/* Interrupt handling init */
	for (i=0; i<16; i++) {
		int_table[i].fast = NULL;
		int_table[i].intdrv = NULL;
		int_table[i].proc_index = NIL;
		int_table[i].isUsed = FALSE;
		int_table[i].irqLock = FALSE;
	}

	register_cancellation_point(signal_cancellation_point, NULL);
}


