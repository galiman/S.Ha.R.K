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
 CVS :        $Id: model.h,v 1.16 2006/03/09 11:55:10 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.16 $
 Last update: $Date: 2006/03/09 11:55:10 $
 ------------

 This file contains the definitions of the task and resource models.

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

#ifndef __KERNEL_MODEL_H__
#define __KERNEL_MODEL_H__

#include "ll/ll.h"
#include "kernel/types.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   TASK MODELS
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   ----------------------------------------------------------------------- */


/* -----------------------------------------------------------------------
   TASK_MODELS: the base struct
   ----------------------------------------------------------------------- */

/*+
 TASK_MODEL

 IMPORTANT: this structure shall not be used by the end-user!!!

 This structure is only used to group together a set of optional
 parameters describing the task model. This structure is passed
 to the task_create primitive.

 The control field is used to set special task processing
 functions.

 Currently it supports:

 - USE_FPU     bit
   If the hw architecture is not smart enough to allow automatic
   FPU context switch, this information is used at the VM level
   to perform transparently the preemption of a FPU-task

 - NO_KILL     bit
   If this bit is set, the task can't be killed via the task_kill
   function. To make a task unkillable for short periods of time,
   use the cancellability functions instead.

 - NO_PREEMPT  bit
   If this bit is set, the task can't be preempted. To set/reset it,
   use the primitive task_preempt/task_no_preempt

 - SYSTEM_TASK bit
   If this bit is set, the task is a system task. The whole system exit
   only when all the non-system tasks are terminated.

 - JET_ENABLED bit
   If this bit is set the Generic Kernel records the Job Execution Times
   for the task. See the jet_XXX functions...

 - TASK_JOINABLE bit
   If this bit is set the task is joinable with task_join, otherwise the
   task is detached...

 - STACKADDR_SPECIFIED bit
   This bit is set when the task was created if we specify in the model
   the stack address. When the task ends, if this bi is set, the stack
   is not freed.


 - KILL_*      bits
   These bits are used to memorize the cancelability state of the task.

 - CONTROL_CAP bit
   This flag has to be set in the scheduling modules (NOT by the end-user)
   only if the kernel has to check the capacity for the task. The kernel
   uses only the avail_time field of the process descriptor.

 - TASK_DOING_SIGNALS bit
   It is an internal flag used with signal handling. It is set only when
   the task is executing a signal handler

 - FREEZE_ACTIVATION
   If this bit is set, the task_activate primitive doesn't activate any task;
   instead, it increment a counter. See task_[un]block_activations in kern.c

 - WAIT_FOR_JOIN
   The flag is set when the task terminates; the descriptor is not
   freed because we wait a task_join

 - DESCRIPTOR_DISCARDED
   This bit is set when the task descriptor is discarded by task_createn
   (the wait_for_join flag is set and the task was inserted in the free
   queue by a scheduling level). The task will be reinserted into the
   free queue by task_join...

 All the models redefines the TASK_MODEL structure
 - adding new fields if needed
 - using a unique number in the pclass variable

 When the user wants to create a task, he must specify a task model.
 First, he have to define a XXX_TASK_MODEL, then he must initialize it with
 a XXX_task_default_model.

 Then, he can specify some attributes that characterize the model.

 Not all the fields of a task model are mandatory, but a scheduling level
 or an ahard server may require some of them.
 For example, if the user wants to create an ahard tasks, he may specify
 a wcet. The wcet is not required by a Deferrable Server, but it is mandatory
 for a TBS!!!.

+*/


typedef struct {
    WORD    pclass;
    LEVEL   level;
    size_t  stacksize;
    void    *stackaddr;
    WORD    group;
    void    *arg;
    DWORD   control;
} TASK_MODEL;

/*+ Value for the control field, It is set if +*/
#define USE_FPU              0x0001 /*+ the task use FPU registers +*/
#define NO_KILL              0x0002 /*+ the task isn't killable at all +*/
#define NO_PREEMPT           0x0004 /*+ the task isn't preemptable +*/
#define SYSTEM_TASK          0x0008 /*+ the task is a system task. +*/
#define JET_ENABLED          0x0010 /*+ execution time monitoring enabled +*/
#define TASK_JOINABLE        0x0020 /*+ the task is joinable (see task_join)+*/
#define STACKADDR_SPECIFIED  0x0040 /*+ the stackaddr was specified +*/
#define TRACE_TASK          0x20000 /*+ the task must be traced +*/

/*+ flags contained in the control field, usettables from the models: +*/
#define KILLED_ON_CONDITION  0x0080 /*+ the task is killed but it is waiting
                                        to die because it must reaquire
                                        the mutex +*/
#define KILL_ENABLED         0x0100 /*+ cancelability enabled +*/
#define KILL_DEFERRED        0x0200 /*+ cancelability type deferred/async. +*/
#define KILL_REQUEST         0x0400 /*+ kill issued but not executed +*/
#define CONTROL_CAP          0x0800 /*+ Capacity control enabled +*/
#define TASK_DOING_SIGNALS   0x1000 /*+ see kern_deliver_pending_signals
                                        in signal.c +*/
#define FREEZE_ACTIVATION    0x2000 /*+ see task_block_activation in kern.c +*/

/* flags used in the implementation of the task_join */
#define WAIT_FOR_JOIN        0x4000 /*+ the task is terminated, but the
                                        descriptor is not freed because we wait
                                        a task_join +*/
#define DESCRIPTOR_DISCARDED 0x8000 /*+ the task descriptor is discarded by
                                        task_createn because the wait_for_join
                                        flag is set and it was inserted in the
                                        free queue by a scheduling level +*/

/* flag used in the implementation of the sig_timedwait */
#define SIGTIMEOUT_EXPIRED   0x10000 /*+ if the sigwait timer expires this
                                        flag is set... +*/

/* flag to avoid task_makefree called 2 times */
#define TASK_MAKEFREE       0x100000

/* Some macros to set various task-model parameters */
#define task_default_model(m,p) (m).pclass = (p), \
                                (m).level = 0; \
                                (m).stacksize = 4096, \
                                (m).stackaddr = NULL, \
                                (m).group = 0, \
                                (m).arg = NULL,\
                                (m).control = 0
#define task_def_level(m,l)     (m).level = (l)
#define task_def_arg(m,a)       (m).arg = (a)
#define task_def_stack(m,s)     (m).stacksize = (s)
#define task_def_stackaddr(m,s) (m).stackaddr = (s)
#define task_def_group(m,g)     (m).group = (g)
#define task_def_usemath(m)     (m).control |= USE_FPU
#define task_def_system(m)      (m).control |= SYSTEM_TASK
#define task_def_nokill(m)      (m).control |= NO_KILL
#define task_def_ctrl_jet(m)    (m).control |= JET_ENABLED
#define task_def_joinable(m)    (m).control |= TASK_JOINABLE
#define task_def_unjoinable(m)  (m).control &= ~TASK_JOINABLE
#define task_def_trace(m)       (m).control |= TRACE_TASK
#define task_def_notrace(m)     (m).control &= ~TRACE_TASK




/* -----------------------------------------------------------------------
   PCLASS values
   ----------------------------------------------------------------------- */

/* These are the value for the pclass field */

#define DUMMY_PCLASS        0
#define HARD_PCLASS         1
#define SOFT_PCLASS         2
#define NRT_PCLASS          3
#define JOB_PCLASS          4
#define ELASTIC_PCLASS      5
#define INTERRUPT_PCLASS    6

/* -----------------------------------------------------------------------
   Useful stuffs
   ----------------------------------------------------------------------- */

#define PERIODIC       0
#define APERIODIC      1

#define SAVE_ARRIVALS  0
#define SKIP_ARRIVALS  1

/* -----------------------------------------------------------------------
   DUMMY_TASK_MODEL: model used only for the dummy task
   ----------------------------------------------------------------------- */

/*+ the dummy task doesn't add any new field +*/
typedef TASK_MODEL DUMMY_TASK_MODEL;

#define dummy_task_default_model(m) task_default_model(m,DUMMY_PCLASS)
#define dummy_task_def_level(m,l)   task_def_level(m,l)
#define dummy_task_def_system(m)    task_def_system(m)
#define dummy_task_def_nokill(m)    task_def_nokill(m)
#define dummy_task_def_ctrl_jet(m)  task_def_ctrl_jet(m)
#define dummy_task_def_group(m,g)   task_def_group(m,g)
#define dummy_task_def_arg(m,a)     task_def_arg(m,a)


/* -----------------------------------------------------------------------
   INTERRUPT_TASK_MODEL: model used only for the INTDrive task
   ----------------------------------------------------------------------- */

/*+ The only added field is the wcet used to check ISR overrun +*/

typedef struct {
  TASK_MODEL t;
  TIME wcet;
} INTERRUPT_TASK_MODEL;

#define interrupt_task_default_model(m)                             \
                        task_default_model((m).t,INTERRUPT_PCLASS), \
                        (m).wcet        = 0 
#define interrupt_task_def_level(m,l)    task_def_level((m).t,l)
#define interrupt_task_def_arg(m,a)      task_def_arg((m).t,a)
#define interrupt_task_def_system(m)     task_def_system((m).t)
#define interrupt_task_def_nokill(m)     task_def_nokill((m).t)
#define interrupt_task_def_wcet(m,w)     (m).wcet = (w)


/* -----------------------------------------------------------------------
   HARD_TASK_MODEL: hard Tasks
   ----------------------------------------------------------------------- */

/*  A Hard Task model can be used to model periodic and sporadic tasks.
    These tasks are usually guaranteed basing on their minimum interarrival
    time (mit) and wcet, and may have a relative deadline and a release
    offset.

    A hard task can raise these exceptions:
    XDEADLINE_MISS XWCET_VIOLATION XACTIVATION

    The default model sets wcet, mit and relative deadline to 0, and
    the periodicity to PERIODIC.
*/

typedef struct {
  TASK_MODEL t;
  TIME mit;
  TIME drel;
  TIME wcet;
  int periodicity;
  TIME offset;
} HARD_TASK_MODEL;

#define hard_task_default_model(m)                             \
                        task_default_model((m).t,HARD_PCLASS), \
                        (m).mit         = 0,                   \
                        (m).drel        = 0,                   \
                        (m).wcet        = 0,                   \
                        (m).periodicity = PERIODIC,            \
                        (m).offset      = 0 
#define hard_task_def_level(m,l)    task_def_level((m).t,l)
#define hard_task_def_arg(m,a)      task_def_arg((m).t,a)
#define hard_task_def_stack(m,s)    task_def_stack((m).t,s)
#define hard_task_def_stackaddr(m,s) task_def_stackaddr((m).t,s)
#define hard_task_def_group(m,g)    task_def_group((m).t,g)
#define hard_task_def_usemath(m)    task_def_usemath((m).t)
#define hard_task_def_system(m)     task_def_system((m).t)
#define hard_task_def_nokill(m)     task_def_nokill((m).t)
#define hard_task_def_ctrl_jet(m)   task_def_ctrl_jet((m).t)
#define hard_task_def_mit(m,p)      (m).mit = (p)
#define hard_task_def_drel(m,d)     (m).drel = (d)
#define hard_task_def_wcet(m,w)     (m).wcet = (w)
#define hard_task_def_offset(m,o)   (m).offset = (o)
#define hard_task_def_periodic(m)   (m).periodicity = PERIODIC
#define hard_task_def_aperiodic(m)  (m).periodicity = APERIODIC
#define hard_task_def_joinable(m)   task_def_joinable((m).t)
#define hard_task_def_unjoinable(m) task_def_unjoinable((m).t)
#define hard_task_def_trace(m)      task_def_trace((m).t)
#define hard_task_def_notrace(m)    task_def_notrace((m).t)


/* -----------------------------------------------------------------------
   SOFT_TASK_MODEL: Soft Tasks
   ----------------------------------------------------------------------- */

/*  A Soft Task model can be used to model periodic and aperiodic tasks
    usually not guaranteed or guaranteed basing on their period and mean
    execution time (met). A Soft task can also record pending activations if
    the arrivals are set to SAVE.

    A wcet field is also present for those servers that need if (i.e., TBS)

    The default model sets met, period and wcet to 0, the periodicity to
    PERIODIC and the arrivals to SAVE.

    A Soft Task don't raise any exception.
*/

typedef struct {
  TASK_MODEL t;
  TIME period;
  TIME met;
  TIME wcet;
  int periodicity;
  int arrivals;
} SOFT_TASK_MODEL;

#define soft_task_default_model(m)                             \
                        task_default_model((m).t,SOFT_PCLASS), \
                        (m).period      = 0,                   \
                        (m).met         = 0,                   \
                        (m).wcet        = 0,                   \
                        (m).periodicity = PERIODIC,            \
                        (m).arrivals    = SAVE_ARRIVALS
#define soft_task_def_level(m,l)       task_def_level((m).t,l)
#define soft_task_def_arg(m,a)         task_def_arg((m).t,a)
#define soft_task_def_stack(m,s)       task_def_stack((m).t,s)
#define soft_task_def_stackaddr(m,s)   task_def_stackaddr((m).t,s)
#define soft_task_def_group(m,g)       task_def_group((m).t,g)
#define soft_task_def_usemath(m)       task_def_usemath((m).t)
#define soft_task_def_system(m)        task_def_system((m).t)
#define soft_task_def_nokill(m)        task_def_nokill((m).t)
#define soft_task_def_ctrl_jet(m)      task_def_ctrl_jet((m).t)
#define soft_task_def_period(m,p)      (m).period = (p)
#define soft_task_def_met(m,d)         (m).met = (d)
#define soft_task_def_wcet(m,w)        (m).wcet = (w)
#define soft_task_def_periodic(m)      (m).periodicity = PERIODIC
#define soft_task_def_aperiodic(m)     (m).periodicity = APERIODIC
#define soft_task_def_save_arrivals(m) (m).arrivals    = SAVE_ARRIVALS
#define soft_task_def_skip_arrivals(m) (m).arrivals    = SKIP_ARRIVALS
#define soft_task_def_joinable(m)      task_def_joinable((m).t)
#define soft_task_def_unjoinable(m)    task_def_unjoinable((m).t)
#define soft_task_def_trace(m)         task_def_trace((m).t)
#define soft_task_def_notrace(m)       task_def_notrace((m).t)


/* -----------------------------------------------------------------------
   NRT_TASK_MODEL: Non Realtime Tasks
   ----------------------------------------------------------------------- */

/* A NRT task has a weight and a time slice, plus  a policy attribute.
   It can be used to model Round Robin, Proportional Share, POSIX,
   and Priority tasks.

   Policy and inherit is inserted in the model to support posix
   compliant scheduling...

   The default model set weight and slice to 0, policy to RR, and inherit
   to explicit.
*/

#define NRT_RR_POLICY   0
#define NRT_FIFO_POLICY 1

#define NRT_INHERIT_SCHED  0
#define NRT_EXPLICIT_SCHED 1

typedef struct {
  TASK_MODEL t;
  int weight;
  TIME slice;
  int arrivals;
  int policy;
  int inherit;
} NRT_TASK_MODEL;

#define nrt_task_default_model(m) task_default_model((m).t,NRT_PCLASS), \
                                      (m).weight   = 0,                 \
                                      (m).slice    = 0,                 \
                                      (m).arrivals = SAVE_ARRIVALS,     \
                                      (m).policy   = NRT_RR_POLICY,     \
                                      (m).inherit  = NRT_EXPLICIT_SCHED
#define nrt_task_def_level(m,l)       task_def_level((m).t,l)
#define nrt_task_def_arg(m,a)         task_def_arg((m).t,a)
#define nrt_task_def_stack(m,s)       task_def_stack((m).t,s)
#define nrt_task_def_stackaddr(m,s)   task_def_stackaddr((m).t,s)
#define nrt_task_def_group(m,g)       task_def_group((m).t,g)
#define nrt_task_def_usemath(m)       task_def_usemath((m).t)
#define nrt_task_def_system(m)        task_def_system((m).t)
#define nrt_task_def_nokill(m)        task_def_nokill((m).t)
#define nrt_task_def_ctrl_jet(m)      task_def_ctrl_jet((m).t)
#define nrt_task_def_joinable(m)      task_def_joinable((m).t)
#define nrt_task_def_unjoinable(m)    task_def_unjoinable((m).t)
#define nrt_task_def_weight(m,w)      (m).weight = (w)
#define nrt_task_def_slice(m,s)       (m).slice = (s)
#define nrt_task_def_save_arrivals(m) (m).arrivals    = SAVE_ARRIVALS
#define nrt_task_def_skip_arrivals(m) (m).arrivals    = SKIP_ARRIVALS
#define nrt_task_def_policy(m,p)      (m).policy = (p)
#define nrt_task_def_inherit(m,i)     (m).inherit = (i)
#define nrt_task_def_trace(m)         task_def_trace((m).t)
#define nrt_task_def_notrace(m)       task_def_notrace((m).t)


/* -----------------------------------------------------------------------
   JOB_TASK_MODEL: Job Task
   ----------------------------------------------------------------------- */

/*  This model implements a Job with an optional period and a starting
    deadline (for the first activation).

    A Job task can raise a XDEADLINE_MISS exception;
    if the flag noraiseexc is != 0, the exception is not raised.

    It represent a SINGLE job activation. Typically, a task with this
    model NEVER call a task_sleep or task_endcycle. Why? because it is
    a single activation.

    In fact, this model is normally used with aperiodic
    servers: the aperiodic server insert a guest task in another level
    with that model; then, when the current activation is ended (e.g. a
    task_sleep() is called) the level, into the XXX_task_sleep, calls
    the XXX_guest_end to terminate the actual activation.

    Note that there is no capacity control on this model.
    Note that the task that accept this task DOESN'T reactivate the
    task after a period... There is NOT a guest_endcycle defined
    for this model...

    The default model set noraiseexc and period to 0, and accept a deadline
*/


typedef struct {
  TASK_MODEL t;
  TIME period;
  struct timespec deadline;
  int noraiseexc;
} JOB_TASK_MODEL;

#define job_task_default_model(m,dl)                     \
                  task_default_model((m).t,JOB_PCLASS),  \
                  (m).period = 0,                        \
                  TIMESPEC_ASSIGN(&((m).deadline),&(dl)),\
                  (m).noraiseexc = 0
#define job_task_def_level(m,l)     task_def_level((m).t,l)
#define job_task_def_arg(m,a)       task_def_arg((m).t,a)
#define job_task_def_stack(m,s)     task_def_stack((m).t,s)
#define job_task_def_stackaddr(m,s) task_def_stackaddr((m).t,s)
#define job_task_def_group(m,g)     task_def_group((m).t,g)
#define job_task_def_usemath(m)     task_def_usemath((m).t)
#define job_task_def_system(m)      task_def_system((m).t)
#define job_task_def_nokill(m)      task_def_nokill((m).t)
#define job_task_def_ctrl_jet(m)    task_def_ctrl_jet((m).t)
#define job_task_def_period(m,per)  (m).period = (per)
#define job_task_def_deadline(m,dl) TIMESPEC_ASSIGN(&((m).deadline),&(dl))
#define job_task_def_noexc(m)       (m).noraiseexc = 1
#define job_task_def_yesexc(m)      (m).noraiseexc = 0
#define job_task_def_joinable(m)    task_def_joinable((m).t)
#define job_task_def_unjoinable(m)  task_def_unjoinable((m).t)
#define job_task_def_trace(m)       task_def_trace((m).t)
#define job_task_def_notrace(m)     task_def_notrace((m).t)


/* -----------------------------------------------------------------------
   ELASTIC_TASK_MODEL: Elastic Task
   ----------------------------------------------------------------------- */

/*  This model implements an elastic task. An elastic task is described by
    the following attributes:

    Tmin   - The nominal (minimum) period. This is the period the task 
             wants to execute at whenever there are enough resources.

    Tmax   - The maximum tolerable period. The elastic model will never 
             force the task to execute at a longer period than this.

    C      - The declared worst-case execution time. By default,
             an exception will be raised if the wcet is violated.

    E      - The elasticity coefficient. A coefficient of 0 means
             that its utilization cannot be changed by the elastic
             algorithm. A large number means that the task is very
	     elastic. The default value is 0.

    beta  -  This parameter determines how the elastic scaling is done.
             PERIOD_SCALING means that the period will be changed, while
             WCET_SCALING means that the wcet will be changed. The
             default is PERIOD_SCALING.
*/


/* Elastic Task */

#define PERIOD_SCALING 0   
#define WCET_SCALING   1


typedef struct {
  TASK_MODEL t;
  TIME Tmin;
  TIME Tmax;
  TIME C;
  int  E;
  int  beta;
  int  arrivals;
} ELASTIC_TASK_MODEL;
                                                                                                                             
#define elastic_task_default_model(m)                        \
                  task_default_model((m).t,ELASTIC_PCLASS),  \
                  (m).Tmin = 0,                        \
		  (m).Tmax = 0,                        \
		  (m).C = 0,                           \
		  (m).E = 0,     		       \
		  (m).beta = PERIOD_SCALING,           \
		  (m).arrivals = SKIP_ARRIVALS
#define elastic_task_def_level(m,l)     task_def_level((m).t,l)
#define elastic_task_def_arg(m,a)       task_def_arg((m).t,a)
#define elastic_task_def_stack(m,s)     task_def_stack((m).t,s)
#define elastic_task_def_stackaddr(m,s) task_def_stackaddr((m).t,s)
#define elastic_task_def_group(m,g)     task_def_group((m).t,g)
#define elastic_task_def_usemath(m)     task_def_usemath((m).t)
#define elastic_task_def_system(m)      task_def_system((m).t)
#define elastic_task_def_nokill(m)      task_def_nokill((m).t)
#define elastic_task_def_ctrl_jet(m)    task_def_ctrl_jet((m).t)
#define elastic_task_def_period(m,min,max)  (m).Tmin = (min),\
					      (m).Tmax = (max)
#define elastic_task_def_wcet(m,w)      (m).C = (w)
#define elastic_task_def_param(m,e,b)   (m).E = (e), \
					(m).beta = (b)
#define elastic_task_def_save_arrivals(m) (m).arrivals    = SAVE_ARRIVALS
#define elastic_task_def_skip_arrivals(m) (m).arrivals    = SKIP_ARRIVALS
#define elastic_task_def_joinable(m)    task_def_joinable((m).t)
#define elastic_task_def_unjoinable(m)  task_def_unjoinable((m).t)
#define elastic_task_def_trace(m)       task_def_trace((m).t)
#define elastic_task_def_notrace(m)     task_def_notrace((m).t)










/* -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   RESOURCE MODELS
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   -----------------------------------------------------------------------
   ----------------------------------------------------------------------- */







/* -----------------------------------------------------------------------
   RTYPE values
   ----------------------------------------------------------------------- */

/* These are the values for the rtype field of a resource descriptor.
   The value in the rtype field is used to distinguish the interface really
   implemented by the resource object.

   For example, a mutex resource descriptor "inherit" from a resource_des
   and implements also all the mutex functions as "virtual", so a type field
   is added to the resource descriptor to distinguish witch interface is
   really added. +*/

#define DEFAULT_RTYPE     0  /*+ no fields added to resource_des +*/
#define MUTEX_RTYPE       1  /*+ the structure implements a mutex
                                 protocol, so a cast to mutex_resource_des
                                 is legal +*/



/* -----------------------------------------------------------------------
   RES_MODEL - the base struct
   ----------------------------------------------------------------------- */

/*+
 RES_MODEL

 This structure is used like the TASK_MODEL.
 It groups together a set of optional parameters describing
 the resource model used by a task.

 It contains only a field; the others are model-dependent.
+*/

typedef struct {
  int rclass;        /* protocol */
  RLEVEL level;          /* level */
} RES_MODEL;

#define res_default_model(r, p)      (r).rclass = (p), (r).level = 0
#define res_def_level(r,l)           (r).level = (l)



/* -----------------------------------------------------------------------
   RCLASS values
   ----------------------------------------------------------------------- */

/*+ These are the values for the type field in the resource models
    a resource level l that accept a resource model with rclass r
    accept also the alias pclass (p | l)
    => the LSByte MUST be 0 (256 levels maximum) (as for PCLASS!!!) +*/

#define PC_RCLASS    0x0100
#define SRP_RCLASS   0x0200
#define SRP2_RCLASS  0x0300

#define BDEDF_RCLASS   0x0400
#define BDPSCAN_RCLASS 0x0500

/* -----------------------------------------------------------------------
   PC_RES_MODEL: BlockDevice EDF resource model
   ----------------------------------------------------------------------- */

typedef struct {
  RES_MODEL r;
  TIME dl;
} BDEDF_RES_MODEL;

#define BDEDF_res_default_model(res) \
  res_default_model((res).r,BDEDF_RCLASS); \
  (res).dl=0     
#define BDEDF_res_def_level(res,l)  res_def_level((res).r,l)     
#define BDEDF_res_def_dl(res,reldl)  (res).dl=reldl

/* -----------------------------------------------------------------------
   PC_RES_MODEL: BlockDevice PSCAN resource model
   ----------------------------------------------------------------------- */

typedef struct {
  RES_MODEL r;
  int priority;
} BDPSCAN_RES_MODEL;

#define BDPSCAN_res_default_model(res) \
  res_default_model((res).r,BDPSCAN_RCLASS); \
  (res).priority=255     
#define BDPSCAN_res_def_level(res,l)  res_def_level((res).r,l)     
#define BDPSCAN_res_def_priority(res,pri)  (res).priority=pri

/* -----------------------------------------------------------------------
   PC_RES_MODEL: Priority ceiling resource model
   ----------------------------------------------------------------------- */

/* the tasks created without using this resource models are assumed to have
   priority = MAX_DWORD (the lowest). */

typedef struct {
  RES_MODEL r;
  DWORD priority;
} PC_RES_MODEL;

#define PC_res_default_model(res, prio) \
                                 res_default_model((res).r, PC_RCLASS); \
                                 (res).priority = (prio)
#define PC_res_def_level(res,l)  res_def_level(res,l)

/* -----------------------------------------------------------------------
   SRP_RES_MODEL: Stack Resource Policy resource model
   ----------------------------------------------------------------------- */

/* the tasks created without using this resource model are not allowed to
   lock any SRP mutex. if two of this models are passed to the task_create,
   one of them is chosen, in a nondeterministic way, so use only one of
   this resource model per task!!!

   The First SRP version uses another resource model that is embedded into
   the mutex structure. refer to kernel/modules/srp.c. this second resource
   model has the SRP2_RCLASS
*/

typedef struct {
  RES_MODEL r;
  DWORD preempt;  /* the preemption level of a task */
} SRP_RES_MODEL;

#define SRP_res_default_model(res, pre) \
                                 res_default_model((res).r, SRP_RCLASS); \
                                 (res).preempt = (pre)
#define SRP_res_def_level(res,l) res_def_level(res,l)


/* -----------------------------------------------------------------------
   MUTEX Attributes
   ----------------------------------------------------------------------- */

/*+
  MUTEX ATTRIBUTES

  A mutexattr object act as the task model for the tasks in the system:
  It specifies the particular options used by a protocol.

  From this basic attribute object many other objects can be derived
  as done for the TASK_MODEL. These objects are used to initialize a mutex
  with a specified protocol.
+*/
typedef struct {
  int mclass;      /* the protocol type... */
} mutexattr_t;

#define mutexattr_default(a, c)  (a).mclass = (c)


/* -----------------------------------------------------------------------
   MCLASS values
   ----------------------------------------------------------------------- */

/*+ These are the value for the mclass field;
    a mutex level l that accept a task model with mclass m
    accept also the alias mclass (m | l)
    => the LSByte MUST be 0 (256 levels maximum) +*/

#define NPP_MCLASS      0x0100
#define PI_MCLASS       0x0200
#define PC_MCLASS       0x0300
#define SRP_MCLASS      0x0400
#define NOP_MCLASS      0x0500
#define NOPM_MCLASS     0x0600
#define PISTAR_MCLASS   0x0700

/* -----------------------------------------------------------------------
   PI_mutexattr_t: Priority Inheritance Mutex Attribute
   ----------------------------------------------------------------------- */

typedef mutexattr_t PI_mutexattr_t;

#define PI_MUTEXATTR_INITIALIZER {PI_MCLASS}
#define PI_mutexattr_default(a)  mutexattr_default(a, PI_MCLASS)

/* -----------------------------------------------------------------------
   PISTAR_mutexattr_t: Priority Inheritance Mutex Attribute for server 
   ----------------------------------------------------------------------- */

typedef mutexattr_t PISTAR_mutexattr_t;

#define PISTAR_MUTEXATTR_INITIALIZER {PISTAR_MCLASS}
#define PISTAR_mutexattr_default(a)  mutexattr_default(a, PISTAR_MCLASS); 

/* -----------------------------------------------------------------------
   NPP_mutexattr_t: Non Preemptive Protocol Mutex Attribute
   ----------------------------------------------------------------------- */

typedef mutexattr_t NPP_mutexattr_t;

#define NPP_MUUEXATTR_INITIALIZER {NPP_MCLASS}
#define NPP_mutexattr_default(a)  mutexattr_default(a, NPP_MCLASS)

/* -----------------------------------------------------------------------
   PC_mutexattr_t: Priority Ceiling Mutex Attribute
   ----------------------------------------------------------------------- */

typedef struct {
  mutexattr_t a;
  DWORD ceiling;
} PC_mutexattr_t;

#define PC_MUTEXATTR_INITIALIZER {{PC_MCLASS},MAX_DWORD}
#define PC_mutexattr_default(at,c)  mutexattr_default((at).a, PC_MCLASS); \
                                    (at).ceiling = (c)

/* -----------------------------------------------------------------------
   SRP_mutexattr_t: Stack Resource Policy Mutex Attribute
   ----------------------------------------------------------------------- */

typedef mutexattr_t SRP_mutexattr_t;

#define SRP_MUTEXATTR_INITIALIZER {SRP_MCLASS}
#define SRP_mutexattr_default(a)  mutexattr_default(a, SRP_MCLASS)

/* -----------------------------------------------------------------------
   NOP_mutexattr_t: No Protocol Mutex Attribute
   ----------------------------------------------------------------------- */

typedef mutexattr_t NOP_mutexattr_t;

#define NOP_MUTEXATTR_INITIALIZER {NOP_MCLASS}
#define NOP_mutexattr_default(a)  mutexattr_default(a, NOP_MCLASS)

/* -----------------------------------------------------------------------
   NOPM_mutexattr_t: No Protocol Multiple lock Mutex Attribute
   ----------------------------------------------------------------------- */

typedef mutexattr_t NOPM_mutexattr_t;

#define NOPM_MUTEXATTR_INITIALIZER {NOPM_MCLASS}
#define NOPM_mutexattr_default(a)  mutexattr_default(a, NOPM_MCLASS)

__END_DECLS
#endif /* __MODEL_H__ */

