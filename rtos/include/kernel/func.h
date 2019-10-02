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
 CVS :        $Id: func.h,v 1.19 2005/05/10 17:15:52 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.19 $
 Last update: $Date: 2005/05/10 17:15:52 $
 ------------

Kernel functions:

- Debug stuff

- Primitives called at initialization time

- Kernel global functions (scheduler, queues)

- Kernel VM hooks

- IRQ, errors & exception handling

- System management primitives

- Jet management primitives

- Task management primitives

- Mutex primitives

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

#ifndef __KERNEL_FUNC_H__
#define __KERNEL_FUNC_H__

#include <ll/ll.h>
#include <ll/stdio.h>
#include <kernel/types.h>
#include <kernel/model.h>
#include <kernel/mem.h>
#include <signal.h>
#include <kernel/var.h>
#include <errno.h>


/*---------------------------------------------------------------------*/
/* Debug stuff                                                         */
/*---------------------------------------------------------------------*/

/* if a source use printk() it should include log.h not func.h */
#include <kernel/log.h>

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*---------------------------------------------------------------------*/
/* Kernel global functions: initialization & termination...            */
/*---------------------------------------------------------------------*/

/*+ this function is called by __kernel_init__.
    It register the modules in the system at init time +*/
TIME __kernel_register_levels__(void *arg);

/*+ This function returns a level_des **. the value returned shall be
    used to register a level module. 

    The function is usually called at module registration time.  The
    function can also be called when the system is already started, to
    allow the implementation of dynamic module registration.  

    The argument must be the size of the data block that have to be allocated

    The function returns the number of the descriptor allocated for the module
    or -1 in case there are no free descriptors.

    The function also reserves a descriptor with size s, initialized
    with default function pointers.

+*/
LEVEL level_alloc_descriptor(size_t s);

/*+ This function release a level descriptor previously allocated using 
  level_alloc_descriptor(). 

  The function returns 0 if the level has been freed, or -1 if someone is
  using it, -2 if the level has never been registered.

+*/
int level_free_descriptor(LEVEL l);

/* Call this if you want to say that your module is using module l 
   (e.g., for calling its private functions) */
int level_use_descriptor(LEVEL l);

/* Call this when you no more need the module l */
int level_unuse_descriptor(LEVEL l);

/*+ This function returns a resource_des **. the value returned shall be
    used to register a resource module. The function shall be called only at
    module registration time. +*/
RLEVEL resource_alloc_descriptor();

/*+ This function compute the command line parameters from the multiboot_info
    NOTE: this function modify the multiboot struct, so this function and
    __call_main__ are mutually exclusives!!! +*/
void __compute_args__(struct multiboot_info *mbi, int *_argc, char **_argv);

/*+ This function calls the standard C main() function, with a
    parameter list up to 100 parameters                        +*/
int __call_main__(struct multiboot_info *mb);

/*+ This task initialize the system and calls the main() with
    __call_mail__ .
    It should be created by a level registered in the system by
    __kernel_register_levels__ +*/
TASK __init__(void *arg);

/*+ Use this function to post your own exit operations
    (when uses some defines contained in const.h) +*/
int sys_atrunlevel(void (*func_code)(void *),void *parm, BYTE when);

/*---------------------------------------------------------------------*/
/* Kernel global functions: scheduler,                                 */
/*---------------------------------------------------------------------*/

/*+ This is the generic scheduler.
    The scheduler calls the appropriates level schedulers and then
    dispatch the task chosen. +*/
void scheduler(void);

/*+ called in the events to force the system to execute the scheduler at
    the end of an event list +*/
void event_need_reschedule();

void task_makefree(void *ret);
void check_killed_async(void);

int guarantee();

void levels_init(void); /* see init.c */

void runlevel_init();
void call_runlevel_func(int runlevel, int aborting);

// in kill.c
void kill_user_tasks();

void event_resetepilogue();


void call_task_specific_data_destructors();
void task_specific_data_init();

// in kill.c
void register_cancellation_point(int (*func)(PID p, void *arg), void *arg);

// in signal.c
void register_interruptable_point(int (*func)(PID p, void *arg), void *arg);


/*---------------------------------------------------------------------*/
/* Kernel VM hooks                                                     */
/*---------------------------------------------------------------------*/

/* this hooks redefines the VM functions to use them into the kernel...

   the only VM functions called directly from the kernel are VM_init and
   VM_end
*/

/* Advanced Timer adjust */
#define kern_scale_timer       ll_scale_advtimer

/* Exit mode selection */
#define sys_set_reboot         ll_set_reboot

/* Context management routines */
#define kern_context_create    ll_context_create
#define kern_context_delete    ll_context_delete

extern __inline__ CONTEXT kern_context_save()
{
  cli();
  return ll_context_from();
}

/*+ this functions are called every time a context is changed +*/
void kern_after_dispatch(void);

/* Warning: if modified, modify also:
   - task_join
   - cond_wait
   - cond_timedwait
   - internal_sem_wait
*/
extern __inline__ void kern_context_load(CONTEXT c)
{
  ll_context_to(c);
  kern_after_dispatch();
  sti();
}


/* Interrupt enabling/disabling */
#define kern_cli          cli
#define kern_sti          sti

/* Interrupt enabling/disabling with flag save */
#define kern_fsave        ll_fsave
#define kern_frestore     ll_frestore

/* interrupt handling */
#define kern_irq_unmask   VM_irq_unmask
#define kern_irq_mask     VM_irq_mask

extern __inline__ int kern_event_post(const struct timespec *time,
                                      void (*handler)(void *p),
                                      void *par)
{
  int e;
  e = event_post(*time,handler,par);

  if (e == -1)
    kern_raise(XNOMORE_EVENTS,exec_shadow);

  return e;
}

#define kern_event_delete event_delete

/*+ the default capacity timer used by the kernel... +*/
void capacity_timer(void *arg);


#define kern_printf message

extern __inline__ TIME kern_gettime(struct timespec *t)
{
  return ll_gettime(TIME_NEW, t);
}



/*---------------------------------------------------------------------*/
/* Kernel global functions: IRQ handling                               */
/*---------------------------------------------------------------------*/

/*+ Interrupt handler installation +*/
int handler_set(int no, void (*fast)(int n), BYTE lock, PID pi, void (*intdrv)(int n));

/*+ Interrupt handler removal      +*/
int handler_remove(int no);

/*+ Return the Interrupt handler for intdrive module +*/
void *handler_get_intdrive(int no);

/*---------------------------------------------------------------------*/
/* System management primitives                                        */
/*---------------------------------------------------------------------*/

void exit(int status);
void _exit(int status);

/*+ Shuts down the system when in RUNLEVEL SHUTDOWN +*/
void sys_abort_shutdown(int err);

/*+ Print a message then call exit(333).
    Can be called from all the tasks...  +*/
void sys_panic(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));

/*+ prints an error message (see perror.c) +*/
void perror (const char *s);

/*+ this primitive returns the time read from the system timer +*/
TIME sys_gettime(struct timespec *t);

/*+ this primitive can be used to set a message that will be printed 
    at shutdown +*/
int sys_shutdown_message(char *fmt,...);

/*---------------------------------------------------------------------*/
/* Jet management primitives                                           */
/*---------------------------------------------------------------------*/

/*+ This primitive returns the maximum execution time and the total
    execution time from the task_create or the last jet_delstat
    It returns also the number of instances to use to calculate the mean
    time and the current job execution time.
    The value returned is 0 if all ok, -1 if the PID is not correct or
    the task doesn't have the JET_ENABLE bit set.
+*/
int jet_getstat(PID p, TIME *sum, TIME *max, int *n, TIME *curr);

/*+ This primitive reset to 0 the maximum execution time and the mean
    execution time of the task p, and reset to 0 all the entries in
    jet_table.
    The value returned is 0 if all ok, -1 if the PID is not correct or
    the task doesn't have the JET_ENABLE bit set.                     +*/
int jet_delstat(PID p);

/*+ This primitive returns the last n values of the task execution time
    recorded after the last call to jet_gettable or jet_delstat.
    If n is
    <0 it will be set only the last values inserted in the table
       since the last call of jet_gettable.
    >0 it will be set up to JET_TABLE_DIM datas.

    The value returned is -1 if the PID is not correct or
    the task doesn't have the JET_ENABLE bit set, otherwise it returns the
    number of values set in the parameter table.
    (can be from 0 to JET_TABLE_DIM-1)
+*/
int jet_gettable(PID p, TIME *table, int n);

/*+ This function updates the jet information. +*/
void jet_update_slice(TIME t);

/*+ This function updates the jet information at the task end period
    it is called in task_endcycle and task_sleep +*/
void jet_update_endcycle();

/*---------------------------------------------------------------------*/
/* Internal Macros                                                     */
/*---------------------------------------------------------------------*/

extern __inline__ void kern_epilogue_macro(void)
{
  TIME tx;    /* a dummy used for time computation             */
  struct timespec ty; /* a dummy used for time computation     */

  kern_gettime(&schedule_time);
  
  /* manage the capacity event */
  SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
  tx = TIMESPEC2USEC(&ty);
  if (proc_table[exec_shadow].control & CONTROL_CAP) proc_table[exec_shadow].avail_time -= tx;
  jet_update_slice(tx);

  /* if the event didn't fire before, we delete it. */
  if (cap_timer != NIL) {
    kern_event_delete(cap_timer);
    cap_timer = NIL;
  }
}

/* This function is called by the kernel into kern.c to register a default 
   exception handler */
int set_default_exception_handler(void);
int remove_default_exception_handler(void);

/*---------------------------------------------------------------------*/
/* Task management primitives                                          */
/*---------------------------------------------------------------------*/


/*+ This primitive:
    - Reserve a task descriptor
    - Create the task based on the task model passed
    - Initialize the resources used by the task
    - Guarantees the task set
+*/
PID task_createn(char *name,      /*+ the symbolic name of the task +*/
                 TASK (*body)(),  /*+ a pointer to the task body    +*/
		 TASK_MODEL *m,   /*+ the task model                +*/
                                  /*+ the resources models, a list  +*/
                 ...);            /*+ of models terminated by NULL  +*/


/*+ a redefinition of task_createn +*/
extern __inline PID task_create(char *name, TASK (*body)(),
                                void *m, void *r)
{
   return task_createn(name, body, (TASK_MODEL *)m, (RES_MODEL *)r, NULL);
}


/* This function allow to create a set of tasks without guarantee.
   It must be called with interrupts disabled and it must be used with
   group_create_accept and group_create_reject.

   This function allocates a task descriptor and fills it.
   After that, the guarantee() function should be called to check for task(s)
   admission.
   Next, each task created with group_create must be accepted with a call to
   group_create_accept() or rejected with a call to group_create_reject.

   The function returns the PID of the allocated descriptor, or NIL(-1)
   if the descriptor cannot be allocated or some problems arises the creation.
   If -1 is returned, errno is set to a value that represent the error:
      ENO_AVAIL_TASK       -> no free descriptors available
      ENO_AVAIL_SCHEDLEVEL -> there were no scheduling modules that can handle
                              the TASK_MODEL *m passed as parameter
      ETASK_CREATE         -> there was an error during the creation of the
                              task into the scheduling module
      ENO_AVAIL_RESLEVEL   -> there were no resource modules that can handle
                              one of the RES_MODEL * passed as parameter
*/
PID group_create(char *name,
                 TASK (*body)(),
                 TASK_MODEL *m,
                 ...);


/*
  This function should be called when a task created with group_create
  is successfully guaranteed and accepted in the system.
  This function finish the creation process allocating the last resources
  for the task (i.e., the stack and the context).
  it returns:
   0 in case of success (all the resources can be allocated)
  -1 if something goes wrong. In this case, THE TASK IS AUTOMATICALLY REJECTED
     AND THE GROUP_CREATE_REJECT MUST NOT BE CALLED. 
     errno is set to a value that explains the problem occurred:

     ENO_AVAIL_STACK_MEM -> No stack memory available for the task 
     ENO_AVAIL_TSS       -> No context available for the task (This is a 
                            CRITICAL error, and usually never happens...)
*/
int group_create_accept(PID i, TASK_MODEL *m);

/*
  This function should be called when a task created with group_create
  can not be successfully guaranteed.
  This function reject the task from the system.
  You cannot use the PID of a rejected task after this call.
*/
void group_create_reject(PID i);

/*+
  It blocks all explicit activation of a task made with task_activate and
  group_activate. These activations are registered in an internal counter,
  returned by task_unblock_activation.
  it returns 0 if all ok, or -1 otherwise. errno is set accordingly.
+*/
int task_block_activation(PID p);

/*+
  It unblocks all explicit activations of a task, and returns the number of
  "frozen" activations. It not call the task_activate!!!!
  it returns -1 if an error occurs. errno is set accordingly.
+*/
int task_unblock_activation(PID p);


/*+ Activate a task specified via pid returned from task_create +*/
int task_activate(PID pid);
					
/*+ Activate a task specified via pid from task_create at time t +*/
int task_activate_at(PID pid, struct timespec *t);

/*+ Kill a task specified via pid returned from task_create +*/
int task_kill(PID pid);

/*+
  This primitive autokills the excuting task; it was used to avoid
  that returning from a task cause a jmp to an unpredictable location.

  Now it is obsolete, the task_create_stub do all the works.

  It is used by the Posix layer to implement pthread_exit
+*/
void task_abort(void *returnvalue);

/*+ Creates a cancellation point in the calling task +*/
void task_testcancel(void);

/*+ Set the cancellation state of the task +*/
int task_setcancelstate(int state, int *oldstate);

/*+ Set the cancellation type of the task +*/
int task_setcanceltype(int type, int *oldtype);

/*+ this function suspends execution of the calling task until the target
    task terminates, unless the target task has already terminated.
    It works like the pthread_join +*/
int task_join(PID p, void **value);

/*+ this function set the detach state of a task to joinable. This function
    is not present in Posix standard...
    returns ESRCH if p is non correct +*/
int task_joinable(PID p);

/*+ this function set the detach state of a task to detached. This function
    works as the posix's pthread_detach
    returns EINVAL if p can't be joined (or currently a task has done a
    join on it (condition not provided in posix)
    ESRCH if p is not correct +*/
int task_unjoinable(PID p);

/*+ Disable the preemption mechanism on the task.
    This primitive is very dangerous!!!!         +*/
void task_nopreempt(void);

/*+ Enable the preemption mechanism on the task. +*/
void task_preempt(void);

/*+ sends a message to the scheduling module that is handling the task +*/
int task_message(void *m, PID p, int reschedule);

/*+ This function signals to the kernel that the current istance of
    the task (periodic or aperiodic) is ended; so the task can be
    suspended until it is activated again. Pending activations may be saved
    depending on the task model +*/
extern __inline__ void task_endcycle(void)
{
  task_testcancel();
  task_message(NULL, NIL, 1);
}

/*+ This function signals to the kernel that the current istance of
    the task (periodic or aperiodic) is ended; so the task can be
    suspended until it is activated again. Pending activations may be saved
    depending on the task model +*/
extern __inline__ void task_disable(PID p)
{
  task_message((void *)(1), p, 0);
}

/*+ This primitives refers the group id which is supplied
    by the application, not by the kernel                 +*/
int group_activate(WORD g);
int group_activate_at(WORD g, struct timespec *t);
int group_kill(WORD g);


/*---------------------------------------------------------------------*/
/* Mutex primitives                                                    */
/*---------------------------------------------------------------------*/

/* This primitives manages a mutex in the system.
   The behavior of the functions is similar to the POSIX ones. */


/*+ Alloc a mutex descriptor +*/
int mutex_init(mutex_t *mutex, const mutexattr_t *attr);

/*+ Free a mutex descriptor  +*/
int mutex_destroy(mutex_t *mutex);

/*+ Block wait 	             +*/
int mutex_lock(mutex_t *mutex);

/*+ Non-block wait           +*/
int mutex_trylock(mutex_t *mutex);

/*+ unlock primitive +*/
int mutex_unlock(mutex_t *mutex);

/*---------------------------------------------------------------------*/
/* Condition variables                                                 */
/*---------------------------------------------------------------------*/

/*+ Initialization of the condition variable +*/
int cond_init(cond_t *cond);

/*+ free a condition variable descriptor +*/
int cond_destroy(cond_t *cond);

/*+ signal on a condition variable, it unlocks only one task +*/
int cond_signal(cond_t *cond);

/*+ broadcast on a condition variable, it unlocks all the blocked tasks +*/
int cond_broadcast(cond_t *cond);

/*+ wait on a condition variable +*/
int cond_wait(cond_t *cond, mutex_t *mutex);

/*+ wait on a condition variable (with timer) +*/
int cond_timedwait(cond_t *cond, mutex_t *mutex,
                   const struct timespec *abstime);

/*---------------------------------------------------------------------*/
/* Task specific data primitives                                       */
/*---------------------------------------------------------------------*/

/* they are similar to the POSIX standard */

int task_key_create(task_key_t *key, void (*destructor)(void *));
void *task_getspecific(task_key_t key);
int task_setspecific(task_key_t key, const void *value);
int task_key_delete(task_key_t key);

/*---------------------------------------------------------------------*/
/* Task cancellation handlers                                          */
/*---------------------------------------------------------------------*/

/*+ push the specified cancellation cleanup handler routine onto
    the cancellation cleanup stack
void task_cleanup_push(void (*routine)(void *), void *arg); +*/
#define task_cleanup_push(rtn,arg) { \
        struct _task_handler_rec __cleanup_handler, **__head; \
        __cleanup_handler.f = rtn; \
        __cleanup_handler.a = arg; \
        __head = task_getspecific(0); \
        __cleanup_handler.next = *__head; \
        *__head = &__cleanup_handler;

/*+
    removes the routine at the top of the cancellation cleanup stack
    of the calling thread
void task_cleanup_pop(int execute); +*/
#define task_cleanup_pop(ex) \
        *__head = __cleanup_handler.next; \
        if (ex) (*__cleanup_handler.f) (__cleanup_handler.a); \
}

__END_DECLS
#endif /* __FUNC_H__ */
