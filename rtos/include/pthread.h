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
 CVS :        $Id: pthread.h,v 1.2 2003/03/13 13:41:04 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:41:04 $
 ------------

 pthread.h

 the commented functions are not implemented yet...

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

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <kernel/const.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <kernel/func.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

void PTHREAD_register_module(LEVEL sched, RLEVEL pi, RLEVEL pc);

#define PTHREAD_CANCELED                TASK_CANCELED
#define PTHREAD_CANCEL_ASYNCHRONOUS     TASK_CANCEL_ASYNCHRONOUS 
#define PTHREAD_CANCEL_DEFERRED         TASK_CANCEL_DEFERRED     
#define PTHREAD_CANCEL_DISABLE          TASK_CANCEL_DISABLE 
#define PTHREAD_CANCEL_ENABLE           TASK_CANCEL_ENABLE
#define PTHREAD_COND_INITIALIZER  {NIL,NULL}
#define PTHREAD_CREATE_DETACHED     0
#define PTHREAD_CREATE_JOINABLE     1
#define PTHREAD_EXPLICIT_SCHED      0
#define PTHREAD_INHERIT_SCHED       1
#define PTHREAD_MUTEX_INITIALIZER {0,(void *)NULL}
#define PTHREAD_ONCE_INIT           0
#define PTHREAD_PRIO_INHERIT        1
#define PTHREAD_PRIO_NONE           0
#define PTHREAD_PRIO_PROTECT        2
// NRQ for PSE52 #define PTHREAD_PROCESS_PRIVATE
// NRQ for PSE52 #define PTHREAD_PROCESS_SHARED

/* there is no difference in PSE52!!! (only one process...) */
#define PTHREAD_SCOPE_PROCESS 0
#define PTHREAD_SCOPE_SYSTEM  0

// NRQ for PSE52 pthread_atfork
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policyt);
int pthread_attr_setscope(pthread_attr_t *attr, int *contentionscope);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_cancel(pthread_t thread);
void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_condattr_destroy(pthread_condattr_t *attr);
// NRQ for PSE52 pthread_condattr_getpshared
int pthread_condattr_init(pthread_condattr_t *attr);
// NRQ for PSE52 pthread_condattr_setpshared
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);
int pthread_equal(pthread_t t1, pthread_t t2);
void pthread_exit(void *value_ptr);
void *pthread_getspecific(pthread_key_t key);
int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
int pthread_join(pthread_t thread, void **value_ptr);
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
int pthread_kill(PID p, int signo);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling);
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);
// NRQ for PSE52 pthread_mutexattr_getpshared
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
// NRQ for PSE52 pthread_mutexattr_setpshared
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));
pthread_t pthread_self(void);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
int pthread_setspecific(pthread_key_t key, const void *value);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset);
void pthread_testcancel(void);


/* Some of the pthread functions are implemented inline */

/*---------------------------------------------------------------------*/
/* 3.3.5 Examine and Change Blocked Signals                            */
/*---------------------------------------------------------------------*/
extern __inline__ int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
  return task_sigmask(how,set,oset);
}

/*---------------------------------------------------------------------*/
/* 3.3.10 Send a signal to a thread                                    */
/*---------------------------------------------------------------------*/
extern __inline__ int pthread_kill(PID p, int signo)
{
  return task_signal(p,signo);
}

/*---------------------------------------------------------------------*/
/* 11.3.1 Mutex Initialization Attributes                              */
/*---------------------------------------------------------------------*/
extern __inline__ int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
  attr->protocol    = PTHREAD_PRIO_NONE;
  attr->prioceiling = sched_get_priority_min(SCHED_RR);

  return 0;
}

extern __inline__ int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
  // nothing!!!
  return 0;
}


/*---------------------------------------------------------------------*/
/* 11.3.2 Initializing and Destroying a Mutex                          */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  return mutex_destroy(mutex);
}


/*---------------------------------------------------------------------*/
/* 11.3.2 Locking and Unlocking a mutex                                */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_mutex_lock(pthread_mutex_t *mutex)
{
  return mutex_lock(mutex);
}

extern __inline__ int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  return mutex_trylock(mutex);
}

extern __inline__ int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  return mutex_unlock(mutex);
}

/*---------------------------------------------------------------------*/
/* 11.4.1 Condition Variable initialization Attributes                 */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_condattr_init(pthread_condattr_t *attr)
{
  return 0;
}

extern __inline__ int pthread_condattr_destroy(pthread_condattr_t *attr)
{
  return 0;
}

/*---------------------------------------------------------------------*/
/* 11.4.2 Initializing and Destroying Condition Variables              */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
  return cond_init(cond);
}

extern __inline__ int pthread_cond_destroy(pthread_cond_t *cond)
{
  return cond_destroy(cond);
}

/*---------------------------------------------------------------------*/
/* 11.4.3 Broadcasting and Signaling a Condition                       */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_cond_broadcast(pthread_cond_t *cond)
{
  return cond_broadcast(cond);
}

extern __inline__ int pthread_cond_signal(pthread_cond_t *cond)
{
  return cond_signal(cond);
}

/*---------------------------------------------------------------------*/
/* 11.4.4 Waiting on a Condition                                       */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  return cond_wait(cond, mutex);
}

extern __inline__ int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                           const struct timespec *abstime)
{
  return cond_timedwait(cond, mutex, abstime);
}


/*---------------------------------------------------------------------*/
/* 13.5.1 Thread Creation Scheduling Attributes                        */
/*---------------------------------------------------------------------*/
extern __inline__ int pthread_attr_getinheritsched(pthread_attr_t *attr, int *inheritsched)
{
  *inheritsched = attr->inheritsched;
  return 0;
}

extern __inline__ int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
  param->sched_priority = attr->schedparam.sched_priority;
  return 0;
}

extern __inline__ int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
  *policy = attr->schedpolicy;
  return 0;
}

extern __inline__ int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope)
{
  *contentionscope = attr->contentionscope;
  return 0;
}

extern __inline__ int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
  if (inheritsched != PTHREAD_INHERIT_SCHED &&
      inheritsched != PTHREAD_EXPLICIT_SCHED   ) return (EINVAL);

  attr->inheritsched = inheritsched;
  return 0;
}

extern __inline__ int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
  // ehm... no controls :-)
  attr->schedparam = *param;
  return 0;
}

extern __inline__ int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
  if (policy != SCHED_RR && policy != SCHED_FIFO && policy != SCHED_OTHER)
    return (EINVAL);

  attr->schedpolicy = policy;
  return 0;
}

extern __inline__ int pthread_attr_setscope(pthread_attr_t *attr, int *contentionscope)
{
  if (contentionscope != PTHREAD_SCOPE_SYSTEM &&
      contentionscope != PTHREAD_SCOPE_PROCESS   ) return (EINVAL);

  attr->contentionscope = *contentionscope;
  return 0;
}

/*---------------------------------------------------------------------*/
/* 13.6.1 Mutex Initialization Scheduling Attributes                   */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
  *prioceiling = attr->prioceiling;
  return 0;
}

extern __inline__ int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
  *protocol = attr->protocol;
  return 0;
}

extern __inline__ int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
  if (prioceiling < sched_get_priority_min(SCHED_RR) ||
      prioceiling > sched_get_priority_max(SCHED_RR)   ) {
    return EINVAL;
  }

  attr->prioceiling = prioceiling;
  return 0;
}

extern __inline__ int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
  if (protocol != PTHREAD_PRIO_NONE &&
      protocol != PTHREAD_PRIO_INHERIT &&
      protocol != PTHREAD_PRIO_PROTECT) {
    return EINVAL;
  }

  attr->protocol = protocol;
  return 0;
}

/*---------------------------------------------------------------------*/
/* 16.2.1 Thread Creation attributes                                   */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_attr_destroy(pthread_attr_t *attr)
{
  // nothing!!!
  return 0;
}

extern __inline__ int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
  *detachstate = attr->detachstate;
  return 0;
}

extern __inline__ int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
  *stackaddr = attr->stackaddr;
  return 0;
}

extern __inline__ int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
  *stacksize = attr->stacksize;
  return 0;
}

extern __inline__ int pthread_attr_init(pthread_attr_t *attr)
{
  // note that the contention scope is not used, because the behaviour
  // is the same...
  attr->contentionscope           = PTHREAD_SCOPE_PROCESS;
  attr->inheritsched              = PTHREAD_EXPLICIT_SCHED;
  attr->schedpolicy               = SCHED_RR;
  attr->schedparam.sched_priority = sched_get_priority_min(SCHED_RR);
  attr->detachstate               = PTHREAD_CREATE_JOINABLE;

  attr->stacksize                 = STACK_SIZE;
  attr->stackaddr                 = NULL;

  return 0;
}

extern __inline__ int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
  if (detachstate != PTHREAD_CREATE_JOINABLE &&
      detachstate != PTHREAD_CREATE_DETACHED   ) return (EINVAL);

  attr->detachstate = detachstate;
  return 0;
}

extern __inline__ int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
  attr->stackaddr = stackaddr;
  return 0;
}

extern __inline__ int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
  if (stacksize < STACK_SIZE) return (EINVAL);

  attr->stacksize = stacksize;
  return 0;
}

/*---------------------------------------------------------------------*/
/* 16.2.3 Wait for thread termination                                  */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_join(pthread_t thread, void **value_ptr)
{
  return task_join(thread, value_ptr);
}

/*---------------------------------------------------------------------*/
/* 16.2.4 Detaching a thread                                           */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_detach(pthread_t thread)
{
  return task_unjoinable(thread);
}

/*---------------------------------------------------------------------*/
/* 16.2.5 Thread Termination                                           */
/*---------------------------------------------------------------------*/

extern __inline__ void pthread_exit(void *value_ptr)
{
  task_abort(value_ptr);
}

/*---------------------------------------------------------------------*/
/* 16.2.6 Get thread ID                                                */
/*---------------------------------------------------------------------*/

extern __inline__ pthread_t pthread_self(void)
{
  return exec_shadow;
}

/*---------------------------------------------------------------------*/
/* 16.2.7 Compare Threads ID                                           */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_equal(pthread_t t1, pthread_t t2)
{
  return t1 == t2;
}


/*---------------------------------------------------------------------*/
/* 17.1.1 Thread specific data key creation                            */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
  return task_key_create(key,destructor);
}

/*---------------------------------------------------------------------*/
/* 17.1.2 Thread Specific Data Management                              */
/*---------------------------------------------------------------------*/

extern __inline__ void *pthread_getspecific(pthread_key_t key)
{
  return task_getspecific(key);
}

extern __inline__ int pthread_setspecific(pthread_key_t key, const void *value)
{
  return task_setspecific(key,value);
}

/*---------------------------------------------------------------------*/
/* 17.1.3 Thread-Specific Data key deletion                            */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_key_delete(pthread_key_t key)
{
  return task_key_delete(key);
}

/*---------------------------------------------------------------------*/
/* 18.2.2 Setting Cancelability State                                  */
/*---------------------------------------------------------------------*/

extern __inline__ int pthread_setcancelstate(int state, int *oldstate)
{
  return task_setcancelstate(state, oldstate);
}

extern __inline__ int pthread_setcanceltype(int type, int *oldtype)
{
  return task_setcanceltype(type, oldtype);
}

extern __inline__ void pthread_testcancel(void)
{
  task_testcancel();
}

/*---------------------------------------------------------------------*/
/* 18.2.2 Setting Cancelability State                                  */
/*---------------------------------------------------------------------*/

#define pthread_cleanup_push task_cleanup_push
#define pthread_cleanup_pop  task_cleanup_pop

__END_DECLS

#endif

