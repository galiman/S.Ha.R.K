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
 CVS :        $Id: time.c,v 1.3 2003/11/05 15:05:12 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2003/11/05 15:05:12 $
 ------------

 This file contains the functions defined in time.h

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


#include <ll/ll.h>
#include <errno.h>
#include <kernel/func.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

struct internal_timer_struct {
  struct sigevent evp;     /* the sigevent MUST be allocated by the applic. */
  int event;               /* the timer event... the timer is disarmed when
                              event == -1 */
  struct timespec period;  /* period for periodic timers */
  struct timespec current; /* time at whitch the event is posted */

  /* these fields are used only if SIGEV_SIGNAL is specified */
  int signal;              /* the reserved signal entry for the timer */
  int overrun;             /* the signal overrun counter */

  int used;                /* 1 if the timer is used */
  int next;
};


/* the timer table */
static struct internal_timer_struct timer_table[TIMER_MAX];

/* the free timer pointer */
static int timerqueue_free;

void TIMER_register_module()
{
  int x;

  for (x = 0; x < TIMER_MAX; x++) {
    timer_table[x].event = -1; /* invalid value */
    NULL_TIMESPEC(&timer_table[x].period);
    timer_table[x].used = 0;
    timer_table[x].overrun = 0;
    // evp is not initialized
    timer_table[x].next = x+1;
  }
  timer_table[TIMER_MAX-1].next = -1;
  timerqueue_free = 0;

}

/*---------------------------------------------------------------------*/
/* 14.2.1 - Clocks                                                     */
/*---------------------------------------------------------------------*/

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
  if (clock_id != CLOCK_REALTIME)
    errno = EINVAL;
  else
    errno = EPERM;
  return -1;
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
  if (clock_id != CLOCK_REALTIME) {
    errno = EINVAL;
    return -1;
  }

  sys_gettime(tp);
  return 0;
}

int clock_getres(clockid_t clock_id, struct timespec *res)
{
  if (clock_id != CLOCK_REALTIME) {
    errno = EINVAL;
    return -1;
  }

  if (res) {
    /* 1 usec */
    res->tv_sec  = 0;
    res->tv_nsec = 1000;
  }

  return 0;
}

/*---------------------------------------------------------------------*/
/* 14.2.2 - Create a Per-Process Timer                                 */
/*---------------------------------------------------------------------*/

int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
  SYS_FLAGS f;

  if (clock_id != CLOCK_REALTIME) {
    errno = EINVAL;
    return -1;
  }

  f = kern_fsave();

  if (timerqueue_free == -1 || sigqueue_free == -1) {
    kern_frestore(f);
    errno = EAGAIN;
    return -1;
  }

  /* alloc a timer descriptor */
  *timerid = timerqueue_free;
  timerqueue_free = timer_table[timerqueue_free].next;
  timer_table[*timerid].used = 1;

  /* alloc a signal descriptor and fill the timer struct */
  if (!evp) {
    // set default data for the evp field
    timer_table[*timerid].evp.sigev_notify = SIGEV_SIGNAL;
    timer_table[*timerid].evp.sigev_signo  = DEFAULT_TIMER_SIGNAL;
    timer_table[*timerid].evp.sigev_value.sival_int = *timerid;
  }
  else
    timer_table[*timerid].evp = *evp;

  if (timer_table[*timerid].evp.sigev_notify == SIGEV_SIGNAL) {
    // alloc a signal descriptor
    timer_table[*timerid].signal = sigqueue_free;
    sig_queue[sigqueue_free].flags |= USED_FOR_TIMER;
    sigqueue_free = sig_queue[sigqueue_free].next;
  }

  timer_table[*timerid].event = -1;

  NULL_TIMESPEC(&timer_table[*timerid].period);
  NULL_TIMESPEC(&timer_table[*timerid].current);

  timer_table[*timerid].overrun = 0;

  kern_frestore(f);
  return 0;
}


/*---------------------------------------------------------------------*/
/* 14.2.3 - Delete a Per-Process Timer                                 */
/*---------------------------------------------------------------------*/

int timer_delete(timer_t timerid)
{
  SYS_FLAGS f;

  if (timerid < 0 || timerid >= TIMER_MAX) {
    errno = EINVAL;
    return -1;
  }

  f = kern_fsave();

  if (!timer_table[timerid].used) {
    kern_frestore(f);
    errno = EINVAL;
    return -1;
  }

  timer_table[timerid].used = 0;

  /* delete the event if the timer is armed */
  if (timer_table[timerid].event != -1)
    kern_event_delete(timer_table[timerid].event);

  if (timer_table[timerid].evp.sigev_notify == SIGEV_SIGNAL) {
    if (!(sig_queue[ timer_table[timerid].signal ].flags & SIGNAL_POSTED)) {
      /* if the signal is not pending, we insert it into the sigqueue_free.
         instead, if it is pending, it will be inserted into the queue when
         delivered */
      sig_queue[ timer_table[timerid].signal ].next = sigqueue_free;
      sigqueue_free = timer_table[timerid].signal;
    }
  
    /* reset the timer flags... */
    sig_queue[ timer_table[timerid].signal ].flags &=
      ~(USED_FOR_TIMER | SIGNAL_POSTED);
  }

  kern_frestore(f);
  return 0;
}

/*---------------------------------------------------------------------*/
/* 14.2.4 - Per-Process Timers                                         */
/*---------------------------------------------------------------------*/

void timer_timerfire(void *arg)
{
  /* Now, we queue the signal:
     - if the signal is already pending, only increment the pending
       activations
     - if the signal isn't pending,
       - we insert the reserved signal into
         the sigqueue_free (so it will be popped by the sigqueue)
       - we set the posted flag (it will be resetted when the signal
         will be dispatched)
  */

  int t = (int)arg;

//  kern_printf("*%d",t);
  // do the action required...
  if (timer_table[t].evp.sigev_notify == SIGEV_SIGNAL) {
    if (sig_queue[ timer_table[t].signal ].flags & SIGNAL_POSTED) {
      // the signal is already pending, increment the pending activations...
      if (timer_table[t].overrun != DELAYTIMER_MAX)
        timer_table[t].overrun++;
    }
    else {
      timer_table[t].overrun = 0;
      // there is no signal pending... post the signal!!!
      // This a dirty trick: The timer has allocated a signal descriptor,
      // then the timer put at the top of the free queue,
      // so sigqueue_internal pick the right number!!!
      sig_queue[ timer_table[t].signal ].next = sigqueue_free;
      sigqueue_free = timer_table[t].signal;
      sigqueue_internal(0,
                        timer_table[t].evp.sigev_signo,
                        timer_table[t].evp.sigev_value,
                        SI_TIMER);
      // setting this flag is used for counting overruns...
      sig_queue[ timer_table[t].signal ].flags |= SIGNAL_POSTED;
    }

  } else if (timer_table[t].evp.sigev_notify == SIGEV_THREAD) {
    /* a new thread must be created; note that the pthread_create
       calls task_createn and task_activate; task_activate works into
       signal handlers and calls event_need_reschedule */
    pthread_t new_thread;

    if (timer_table[t].evp.sigev_notify_attributes)
      pthread_create(&new_thread,
                     timer_table[t].evp.sigev_notify_attributes,
                     (void *(*)(void *))timer_table[t].evp.sigev_notify_function,
                     timer_table[t].evp.sigev_value.sival_ptr);
    else {
      pthread_attr_t new_attr;
      // the task must be created detached
      pthread_attr_init(&new_attr);
      pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);

      pthread_create(&new_thread,
                     &new_attr,
                     (void *(*)(void *))timer_table[t].evp.sigev_notify_function,
                     &timer_table[t].evp.sigev_value);
    }
  }

  if (timer_table[t].period.tv_sec != 0 ||
      timer_table[t].period.tv_nsec != 0) {
    struct timespec temp;

    TIMESPEC_ASSIGN(&temp,&timer_table[t].current);
    ADDTIMESPEC(&temp, &timer_table[t].period, &timer_table[t].current);

    timer_table[t].event =
      kern_event_post(&timer_table[t].current,
                      timer_timerfire,
                      (void *)t);
/*    kern_printf("(post e%d %d.%d)", t,
                timer_table[t].current.tv_sec,
                timer_table[t].current.tv_nsec/1000); */
  }
  else
    timer_table[t].event = -1;
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *value,
                  struct itimerspec *ovalue)
{
  struct timespec ct;  /* current time */
  int ct_read = 0;     /* we take the current time only once */
  SYS_FLAGS f;

  if (timerid < 0 || timerid >= TIMER_MAX || !value) {
    errno = EINVAL;
    return -1;
  }

  if (value->it_interval.tv_nsec < 0 ||
      value->it_interval.tv_nsec >= 1000000000) {
    errno = EINVAL;
    return -1;
  }

  if (value->it_value.tv_nsec < 0 ||
      value->it_value.tv_nsec >= 1000000000) {
    errno = EINVAL;
    return -1;
  }

  f = kern_fsave();

  if (!timer_table[timerid].used) {
    kern_frestore(f);
    errno = EINVAL;
    return -1;
  }

  if (ovalue) {
    if (timer_table[timerid].event == -1)
      /* the timer is disarmed, set it_value to 0 */
      NULL_TIMESPEC(&ovalue->it_value);
    else {
      /* the timer is armed, return the remaining expiration time */
      kern_gettime(&ct);
      ct_read = 1;
      SUBTIMESPEC(&timer_table[timerid].current, &ct, &ovalue->it_value);
    }
    /* and return the reactivation period */
    TIMESPEC_ASSIGN(&ovalue->it_interval, &timer_table[timerid].period);
  }

  /* if it_value is 0, the timer shall be disarmed; if != 0, the timer is
     armed: in all the cases, the event must be deleted... */
  if (timer_table[timerid].event != -1)
    kern_event_delete(timer_table[timerid].event);

  if (value->it_value.tv_sec != 0 || value->it_value.tv_nsec != 0) {
    /* it_value != 0 -> arm the timer! */
    TIMESPEC_ASSIGN(&timer_table[timerid].period, &value->it_interval);

    if (flags & TIMER_ABSTIME)
      /* the time is absolute */
      TIMESPEC_ASSIGN(&timer_table[timerid].current, &value->it_value);
    else {
      /* the time is relative to current time */
      if (!ct_read)
        kern_gettime(&ct);
      ADDTIMESPEC(&ct, &value->it_value, &timer_table[timerid].current);
    }
    timer_table[timerid].event =
      kern_event_post(&timer_table[timerid].current,
                      timer_timerfire,
                      (void *)timerid);
/*    kern_printf("(post e%d %d.%d)", timerid,
                timer_table[timerid].current.tv_sec,
                timer_table[timerid].current.tv_nsec/1000); */
  }

  kern_frestore(f);
  return 0;
}

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
  struct timespec ct;  /* current time */
  SYS_FLAGS f;

  if (timerid < 0 || timerid >= TIMER_MAX) {
    errno = EINVAL;
    return -1;
  }

  f = kern_fsave();

  if (!timer_table[timerid].used) {
    kern_frestore(f);
    errno = EINVAL;
    return -1;
  }

  if (timer_table[timerid].event == -1)
    /* the timer is disarmed, set it_value to 0 */
    NULL_TIMESPEC(&value->it_value);
  else {
    /* the timer is armed, return the remaining expiration time */
    kern_gettime(&ct);
    SUBTIMESPEC(&timer_table[timerid].current, &ct, &value->it_value);
  }
  /* and return the reactivation period */
  TIMESPEC_ASSIGN(&value->it_interval, &timer_table[timerid].period);

  kern_frestore(f);
  return 0;
}

int timer_getoverrun(timer_t timerid)
{
  int returnvalue;
  SYS_FLAGS f;

  if (timerid < 0 || timerid >= TIMER_MAX) {
    errno = EINVAL;
    return -1;
  }

  f = kern_fsave();

  if (!timer_table[timerid].used) {
    kern_frestore(f);
    errno = EINVAL;
    return -1;
  }

  returnvalue = timer_table[timerid].overrun;
  kern_frestore(f);
  return returnvalue;
}

