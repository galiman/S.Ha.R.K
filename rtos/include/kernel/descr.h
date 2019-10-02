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
 CVS :        $Id: descr.h,v 1.5 2004/05/17 15:03:50 anton Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2004/05/17 15:03:50 $
 ------------

Kernel main data structures

This file declare:

- the descriptors
  - cleanup handlers
  - levels
  - mutexes
  - mutex attributes
  - resource levels


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



#ifndef __KERNEL_DESCR_H__
#define __KERNEL_DESCR_H__


#include <ll/ll.h>
#include <kernel/model.h>
#include <kernel/types.h>
#include <kernel/iqueue.h>
#include <limits.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  CLEANUP HANDLER STRUCTURES

  Derived directly from posix standard, B.18.2.3
  This structure implements the task cleanup functions queue...
  look at kern.c!

  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
struct _task_handler_rec {
  void (*f)(void *);
  void *a;
  struct _task_handler_rec *next;
};



struct condition_struct;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GENERAL TASK DESCRIPTOR

  In this type definition there is all the basic information for
  handling a task in the system.

  All the informations scheduler-dependent (like deadline, priority,
  and so on) are put in the level module files.
  In any case, a priority field is inserted to simplify the implementation
  of most of the scheduling algorithms
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef struct {
        DWORD   task_ID;      /*+ progressive task counter ID +*/
        LEVEL   task_level;   /*+ the "real" level that owns the task     +*/

	CONTEXT context;      /*+ Context area pointer (see vm.h)         +*/
	BYTE    *stack;       /*+ Pointer to stack area base              +*/
	TASK    (*body)();    /*+ Pointer to the code of the task
                                  (starting address)                      +*/
	char    name[MAX_TASKNAME]; /*+ Text identifing the process name  +*/

        WORD    status;       /*+ actual task status
                                  (it could be EXE, SLEEP, IDLE, ...)     +*/
	WORD    pclass;       /*+ The code number of the task model used  +*/
	WORD    group;        /*+ 0 if task is single, else group id      +*/
	WORD    stacksize;    /*+ Task stack size                         +*/
	DWORD   control;      /*+ Control task operating mode
	                         Refer to the TASK_MODEL type for its use +*/

        int     frozen_activations; /*+ number of frozen activation;
                                        see kern.c, task_block_activations
                                        see model.h,flag in control field +*/

        /* sigset_t!!! */
        int sigmask;          /*+ The task signal mask                    +*/
        int sigpending;       /*+ The signal pending mask                 +*/
        int sigwaiting;       /*+ The signal waiting mask                 +*/

        int     avail_time;   /*+ the time the task can execute before a
                                  timer fire. see also the control field
                                  and bits related in model.h             +*/

        PID     shadow;       /*+ Shadow task                             +*/

        struct _task_handler_rec *cleanup_stack;
                              /*+ The cleanup stack                       +*/



	int     errnumber;

        /* Job Execution Time fields */
        TIME    jet_table[JET_TABLE_DIM];
                              /*+ Execution time of the last
                                  activations of the task.                +*/
        int     jet_tvalid;   /*+ number of valid entry in the jet_table  +*/
        int     jet_curr;     /*+ Current entry in the jet_table          +*/
        TIME    jet_max;      /*+ Maximum Execution time since task_create
                                  or last jet_delstat                     +*/
        TIME    jet_sum;      /*+ Mean Execution time since task_create
                                  or last jet_delstat                     +*/
        TIME    jet_n;        /*+ Number of instances on witch the mean
                                  time have to be computed                +*/

        /* task_join fields */
        PID waiting_for_me;   /*+ the task that waits my dead,
                                  NIL if there aren't                     +*/
        void *return_value;   /*+ task return value                       +*/

        /* task specific data (it uses directly POSIX constant) */
        void *keys[PTHREAD_KEYS_MAX];

        /* condition variable field */
        struct condition_struct *cond_waiting;
                              /*+ the condition on that the task is
                                  waiting +*/

        /* stuff used in most algorithms; they are not used directly in
         * the generic kernel, with exclusion of delay_timer that is used
         * also in cond_timedwait
         */

        int     delay_timer;  /*+ A field useful to store the delay timer +*/

        int     wcet;         /*+ a worst case time execution             +*/


} proc_des;


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  LEVEL DESCRIPTOR

  In this type definition there is all the basic information for
  handling a scheduling level in the system.

  All the informations that depends on the particular module are put
  in the level module files.

  Here a small description of the various functions:

  -------------------------------------------------------------------
  - PUBLIC Functions:
    on one side, a module should export an interface to the Generic
    Kernel, giving a set of functions that the Generic Kernel can use
    to ask a service to the module. That is, the Public Functions are
    called ONLY by the Generic Kernel.

  - PRIVATE Functions: on the other side, a module can export an
    interface to the public part of the same or of another
    module. That is, Private Functions are called ONLY by Public and
    Private Functions. 
  -------------------------------------------------------------------

  int  (*private_insert  )(LEVEL l, PID p, TASK_MODEL *m);
  Inserts a task into the internal module data structure.
    
  void (*private_extract )(LEVEL l, PID p);
  Removes a task from the internal module data structure.

  int  (*private_eligible)(LEVEL l, PID p);
  A task inserted into the internal module data structure needs to be 
  scheduled. returns 0 if it can be scheduled, -1 if not.

  void (*private_dispatch)(LEVEL l, PID p, int nostop);
  A task inserted into the internal module data structure has been dispatched.

  void (*private_epilogue)(LEVEL l, PID p);
  A task inserted into the internal module data structure has been preempted.
  



  PID  (*public_scheduler)(LEVEL l);
  returns a task to schedule, or -1 if no tasks are ready

  int  (*public_guarantee)(LEVEL l, bandwidth_t *freebandwidth);
  returns 0 if the level is guaranteed, -1 if not
  no guarantee if (*f)()=null
  the function updates the parameter freebandwidth (see guarantee() )                  
  int  (*public_create   )(LEVEL l, PID p, TASK_MODEL *m);
  the task p is created into the module
  returns 0->ok, -1->error

  void (*public_detach )(LEVEL l, PID p); 
  there is an error in the public_create. The function removes all the
  informations about the task in the module.

  void (*public_end      )(LEVEL l, PID p);
  the task has been killed, or it ended regularly

  int  (*public_eligible )(LEVEL l, PID p);
  A task needs to be scheduled. returns 0 if it can be scheduled, -1 if not.
  
  void (*public_dispatch )(LEVEL l, PID p, int nostop);
  A task has been dispatched.

  void (*public_epilogue )(LEVEL l, PID p);
  A task has been preempted (or its capacity is exausted).

  void (*public_activate )(LEVEL l, PID p, struct timespec *t);
  A task has been activated.

  void (*public_unblock  )(LEVEL l, PID p);
  void (*public_block    )(LEVEL l, PID p);
  A task has been unblocked/blocked on a synchronization point
  (e.g. a semaphore, a mailbox, a nanosleep).

  int (*public_message  )(LEVEL l, PID p, void *m);
  A task sent a message m to the module.

  If the message has value NULL the
  behavior should be the task_endcycle primitive behavior. 

  The function returns an integer to the user.

  If you want to avoid the call to public_epilogue, after public_message,
  just write exec = exec_shadow = -1; in your public_message code.

  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef struct {
  void (*private_insert  )(LEVEL l, PID p, TASK_MODEL *m);
  void (*private_extract )(LEVEL l, PID p);
  int  (*private_eligible)(LEVEL l, PID p);
  void (*private_dispatch)(LEVEL l, PID p, int nostop);
  void (*private_epilogue)(LEVEL l, PID p);

  PID  (*public_scheduler)(LEVEL l);
  int  (*public_guarantee)(LEVEL l, bandwidth_t *freebandwidth);
  int  (*public_create   )(LEVEL l, PID p, TASK_MODEL *m);
  void (*public_detach   )(LEVEL l, PID p);
  void (*public_end      )(LEVEL l, PID p);
  int  (*public_eligible )(LEVEL l, PID p);
  void (*public_dispatch )(LEVEL l, PID p, int nostop);
  void (*public_epilogue )(LEVEL l, PID p);
  void (*public_activate )(LEVEL l, PID p, struct timespec *t);
  void (*public_unblock  )(LEVEL l, PID p);
  void (*public_block    )(LEVEL l, PID p);
  int  (*public_message  )(LEVEL l, PID p, void *m);
} level_des;


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  RESOURCE DESCRIPTOR

  In this type definition there is all the basic information for
  handling a resource module in the system.

  All the informations protocol-dependent (like ceiling, task that use
  a particular resource, and so on) are put in the resource module files.

  In general, the initialization of a resource module is splitted in two
  parts:
  - the registration   -> tipically done with a finction called
                          XXX_register_module. It is called before the
                          system initialization, in
                          the function __kernel_register_levels__().
  - the initialization -> called during the system initialization,
                          This is done posting some init functions with
                          the sys_at_init()
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef struct {
  int rtype;                    /*+ resource module extented interface
                                    code (see model.h)                   +*/

  int (*res_register)(RLEVEL l, PID p, RES_MODEL *r);
                                /*+ When the system knows that a
                                    resource model can be registered
                                    by a level, it calls this
                                    function. It registers all the
                                    information about the task and the
                                    model. returns 0 if the model
                                    can be handled, -1 otherwise+*/

  void (*res_detach)(RLEVEL l, PID p);
                                /*+ this function is called when the task
                                    is killed or some error is occurred
                                    in the task_create. It have to unlink
                                    the task from the module... If the task
                                    is already unlinked from the protocol
                                    no action is done                    +*/
} resource_des;







/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MUTEX DESCRIPTOR

  In this type definition there is all the basic fields for
  handling a mutex in the system

  Many of the informations protocol-dependent (like ceiling, and so on)
  are put in the resource module or are pointef by the field opt.

  The opt field is used because in this way a mutex can be allocated in
  a dynamic way (in this case opt points to a dynamically allocated
  structure) or in a static way (in this case opt can be an index or a
  pointer to a static structure)
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
typedef struct {
  RLEVEL mutexlevel;  /*+ protocol used by the mutex. +*/
  int use;            /*+ the mutex is used in a condition wait... +*/
  void *opt;
} mutex_t;


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MUTEX RESOURCE DESCRIPTOR

  This object is a resource_des object with a set of functions used to
  implement the mutex behaviour.

  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
typedef struct {
  resource_des r;

  int (*init)   (RLEVEL l, mutex_t *m, const mutexattr_t *a);
    /*+ this function is called when a mutex is created. it returns
        >=0 if the mutexattr_t can be managed by the level (=0 Ok, an
        error otherwise), -1 otherwise +*/
  int (*destroy)(RLEVEL l, mutex_t *m);
  int (*lock)   (RLEVEL l, mutex_t *m);
  int (*trylock)(RLEVEL l, mutex_t *m);
  int (*unlock) (RLEVEL l, mutex_t *m);

} mutex_resource_des;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  CONDITION VARIABLE DESCRIPTOR

  This is the condition variable descriptor.
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef struct condition_struct {
  IQUEUE waiters; /*+ queue for tasks waiting on the condition +*/
  mutex_t *used_for_waiting;
} cond_t;

__END_DECLS
#endif /* __TYPE_H__ */
