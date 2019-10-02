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
 CVS :        $Id: pthread.c,v 1.4 2005/02/25 10:36:52 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/02/25 10:36:52 $
 ------------

 pthread.c

 This file contains the implementation of various posix primitives:

 refer to POSIX standard...

 maybe in the near future this functions will become inline functions...

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

#include <kernel/const.h>
#include <kernel/func.h>
#include <pthread.h>
#include <errno.h>
#include <pi/pi/pi.h>
#include <pc/pc/pc.h>
#include <posix/posix/posix.h>

// this fields are used to store the levels used in the pthread_functions...
static LEVEL level_sched;
static RLEVEL level_PI;
static RLEVEL level_PC;

// This function register the POSIX interface into the system...
void PTHREAD_register_module(LEVEL sched, RLEVEL pi, RLEVEL pc)
{
  level_sched = sched;
  level_PI    = pi;
  level_PC    = pc;
}


/*---------------------------------------------------------------------*/
/* 11.3.2 Initializing and Destroying a Mutex                          */
/*---------------------------------------------------------------------*/

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  if (!attr || attr->protocol == PTHREAD_PRIO_NONE) {
    NOP_mutexattr_t m_nop;

    NOP_mutexattr_default(m_nop);
    return mutex_init(mutex,&m_nop);
  }
  else if (attr->protocol == PTHREAD_PRIO_INHERIT) {
    PI_mutexattr_t m_pi;

    PI_mutexattr_default(m_pi);
    return mutex_init(mutex,&m_pi);
  }
  else {   // PTHREAD_PRIO_PROTECT
    PC_mutexattr_t m_pc;

    PC_mutexattr_default(m_pc, sched_get_priority_max(SCHED_RR)-attr->prioceiling);
    return mutex_init(mutex,(mutexattr_t *)&m_pc);
  }
}



/*---------------------------------------------------------------------*/
/* 13.3.5 Yield processor                                              */
/*---------------------------------------------------------------------*/

int sched_yield(void)
{
  return POSIX_sched_yield(level_sched);
}

/*---------------------------------------------------------------------*/
/* 13.3.6 Get Scheduling Parameter Limits                              */
/*---------------------------------------------------------------------*/

int sched_get_priority_max(int policy)
{
  return POSIX_get_priority_max(level_sched);
}

int sched_rr_get_interval(pid_t pid, struct timespec *interval)
{
  return POSIX_rr_get_interval(level_sched);
}

/*---------------------------------------------------------------------*/
/* 13.5.2 Dynamic Thread Schedu ling Parameters Access                  */
/*---------------------------------------------------------------------*/

int pthread_getschedparam(pthread_t thread, int *policy,
                          struct sched_param *param)
{
  int returnvalue;
  SYS_FLAGS f;

  f = kern_fsave();
  returnvalue = POSIX_getschedparam(level_sched, thread, policy,
                                    &param->sched_priority);
  kern_frestore(f);
  return returnvalue;
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
  int returnvalue;
  SYS_FLAGS f;

  f = kern_fsave();

  returnvalue = POSIX_setschedparam(level_sched, thread, policy,
                                    param->sched_priority);

  if (!returnvalue)
    PC_set_task_ceiling(level_PC, thread,
                        sched_get_priority_max(SCHED_RR) -
                          param->sched_priority);

  kern_frestore(f);
  return returnvalue;
}


/*---------------------------------------------------------------------*/
/* 13.6.2 Change the priority Ceiling of a Mutex                       */
/*---------------------------------------------------------------------*/

int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling,
                                 int *old_ceiling)
{
  int returnvalue = 0;
  int max = sched_get_priority_max(SCHED_RR);

  if (prioceiling > max || prioceiling < sched_get_priority_min(SCHED_RR))
    return EINVAL;

  mutex_lock(mutex);
  if (PC_set_mutex_ceiling(mutex,
                           max - prioceiling,
                           (DWORD *)old_ceiling))
    returnvalue = EINVAL;

  if (old_ceiling)
    *old_ceiling = max - *old_ceiling;

  mutex_unlock(mutex);

  return returnvalue;
}

int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex,
                                 int *prioceiling)
{
  PC_get_mutex_ceiling(mutex, (DWORD *)prioceiling);
  *prioceiling = sched_get_priority_max(SCHED_RR) - *prioceiling;

  return 0;
}

/*---------------------------------------------------------------------*/
/* 16.2.2 Thread Creation                                              */
/*---------------------------------------------------------------------*/

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg)
{
  NRT_TASK_MODEL m;   // the scheduling model
  PC_RES_MODEL   pc;  // the resource model used for Priority Ceiling
  int olderrno;       // the pthread_create don't modify errno

  nrt_task_default_model(m);
  nrt_task_def_arg(m,arg);
  nrt_task_def_usemath(m);
  nrt_task_def_ctrl_jet(m);
  nrt_task_def_save_arrivals(m);
  nrt_task_def_level(m, level_sched);

  if (attr) {
    nrt_task_def_stack(m,attr->stacksize);
    nrt_task_def_stackaddr(m,attr->stackaddr);
    if (attr->detachstate == PTHREAD_CREATE_JOINABLE)
      nrt_task_def_joinable(m);
    else
      nrt_task_def_unjoinable(m);

    nrt_task_def_weight(m, attr->schedparam.sched_priority);
    PC_res_default_model(pc, sched_get_priority_max(SCHED_RR) - attr->schedparam.sched_priority);

    nrt_task_def_policy(m,attr->schedpolicy);
    nrt_task_def_inherit(m,attr->inheritsched);
  }
  else {
    nrt_task_def_stack(m,PTHREAD_STACK_MIN);
    nrt_task_def_stackaddr(m,NULL);
    nrt_task_def_joinable(m);

    nrt_task_def_weight(m, sched_get_priority_min(SCHED_RR));
    PC_res_default_model(pc, sched_get_priority_max(SCHED_RR));

    nrt_task_def_policy(m,SCHED_RR);
    nrt_task_def_inherit(m,PTHREAD_EXPLICIT_SCHED);
  }

  olderrno = errno;
  *thread = task_createn("Posix task", start_routine,
                                       (TASK_MODEL *)&m,
                                       &pc, NULL);
  if (*thread != NIL)
    task_activate(*thread);

  errno = olderrno;

  if (*thread == NIL)
    return EINVAL;
  else
    return 0;
}
/*---------------------------------------------------------------------*/
/* 16.2.8 Dynamic Package initialization                               */
/*---------------------------------------------------------------------*/

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
  SYS_FLAGS f;

  f = kern_fsave();
  if (!(*once_control)) {
    *once_control = 1;
    kern_frestore(f);
    init_routine();
  }
  else
    kern_frestore(f);
  return 0;
}



/*---------------------------------------------------------------------*/
/* 18.2.1 Canceling Execution of a Thread                              */
/*---------------------------------------------------------------------*/

int pthread_cancel(pthread_t thread)
{
   int olderrno; // pthread_cancel doesn't modify errno
   int rvalue;   // the return value

   olderrno = errno;
   rvalue = task_kill(thread);
   errno = olderrno;

   if (rvalue)
     return (ESRCH);
   else
     return 0;
}



