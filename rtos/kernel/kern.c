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
 CVS :        $Id: kern.c,v 1.15 2006/03/07 19:11:58 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.15 $
 Last update: $Date: 2006/03/07 19:11:58 $
 ------------

 This file contains:

 - the kernel system variables

 - the errno functions

 - the scheduler, capacity timer, and grarantee

 - the sys_gettime


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

#include <stdarg.h>
#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/*----------------------------------------------------------------------*/
/* Kernel System variables                                              */
/*----------------------------------------------------------------------*/

/* error numbers used for system shutdown */
static int errnumber_global;
static int errnumber_shutdown;



CONTEXT global_context; /*+ Context used during initialization;
                            It references also a safe stack      +*/

int task_counter;       /*+ Application task counter. It represent
                            the number of Application tasks in the
                            system. When all Application Tasks end,
                            also the system ends.                +*/

int system_counter;     /*+ System task counter. It represent
                            the number of System tasks in the
                            system with the NO_KILL flag reset.
                            When all Application Tasks end,
                            the system waits for the end of the
                            system tasks and then it ends.       +*/

PID exec;               /*+ Task advised by the scheduler        +*/
PID exec_shadow;        /*+ Currently executing task             +*/

IQUEUE freedesc;        /*+ Free descriptor handled as a queue   +*/

DWORD sys_tick;         /*+ System tick (in usec)                +*/
struct timespec schedule_time;
                        /*+ Timer read at each call to schedule()+*/

int   cap_timer;        /*+ the capacity event posted when the
                            task starts                          +*/
struct timespec cap_lasttime;
                        /*+ the time at whitch the capacity
                            event is posted. Normally, it is
                            equal to schedule_time               +*/



DWORD sched_levels;     /*+ Schedule levels active in the system +*/
DWORD res_levels;       /*+ Resource levels active in the system +*/

/*+ Process descriptor table +*/
proc_des proc_table[MAX_PROC];

/* Scheduling modules descriptor table */
/* ------------------------------------------------------------------------ */

/* the descriptor table */
level_des *level_table[MAX_SCHED_LEVEL];
/* ... and the size of each descriptor */
size_t level_size[MAX_SCHED_LEVEL];

/* an utilization counter incremented if a level is used by another module */
int level_used[MAX_SCHED_LEVEL];
/* these data structures (first, last, free, next & prev) 
   are used to implement a double linked list of scheduling modules. 
   That list is used by the scheduler to call the module's schedulers. */
int level_first; /* first module in the list */
int level_last;  /* last module in the list */
int level_free;  /* free single linked list of free module descriptors. */
int level_next[MAX_SCHED_LEVEL];
int level_prev[MAX_SCHED_LEVEL];
/* ------------------------------------------------------------------------ */

/*+ Resource descriptor table +*/
resource_des *resource_table[MAX_RES_LEVEL];

/*+ this is the system runlevel... it may be from 0 to 4:
    0 - init
    1 - running
    2 - shutdown
    3 - before halting
    4 - halting
+*/
int runlevel;

/*+ this variable is set to 1 into call_runlevel_func (look at init.c)
    ad it is used because the task_activate (look at activate.c) must
    work in a different way when the system is in the global_context +*/
int calling_runlevel_func;

/* this variable is set when _exit is called. in this case, the
  atexit functions will not be called. 
  Values: 
  - 0 neither exit or _exit have been called
  - 1 exit has been called
  - 2 _exit has been called
*/

int _exit_has_been_called;


/*----------------------------------------------------------------------*/
/* Kernel internal functions                                            */
/*----------------------------------------------------------------------*/

/*+ errno Handling: this functions returns the correct address for errno.
    The address returned can be either the global errno or the errno local
    to the execution task */
static int *__errnumber()
{
  if (exec_shadow == -1)
    return &errnumber_global;
  else
    return &(proc_table[exec_shadow].errnumber);
}

/*+ this is the capacity timer. it fires when the running task has expired
    his time contained in the avail_time field. The event is tipically
    posted in the scheduler() after the task_dispatch. The task_dispatch
    can modify the avail_time field to reach his scheduling purposes.
    The wcet field is NOT used in the Generic kernel. it is initialized at
    init time to 0. +*/
void capacity_timer(void *arg)
{
  /* the capacity event is served, so at the epilogue we
     don't have to erase it */
  cap_timer = NIL;

//  kern_printf("cap%d ",exec_shadow);

  /* When we reschedule, the call to task_epilogue check the slice and
     put the task in the queue's tail */
  event_need_reschedule();
}

/*+
  Generic Scheduler:
  This function select the next task that should be executed.
  The selection is made calling the level schedulers.
  It assume that THERE IS a task that can be scheduled in one
  level.

  The general scheduler:
  - first, it checks for interrupts.
  - then, it calls the epilogue of the task pointed in exec_shadow
  - after that, it calls the level schedulers
  - then it sets exec and it follows the shadow chain
  - finally it calls task_dispatch for the new task (the shadow!!!),
    saying if exec != exec_shadow

+*/
void scheduler(void)
{
  LEVEL l;    /* a counter                                     */
  struct timespec ty; /* a dummy used for time computation     */

  PID p;      /* p is the task chosen by the level scheduler   */
  int ok;     /* 1 only if the task chosen by the level scheduler
                 is eligible (normally, it is; but in some server
                 it is not always true (i.e., CBS))            */

  PID old_exec_shadow;
  
  if ( (exec_shadow != -1 &&
       (proc_table[exec_shadow].control & NO_PREEMPT) ) )
    return;

  //  kern_printf("(!");

  /*
  exec_shadow = exec = -1 only if the scheduler is called from:
   . task_endcycle
   . task_kill
   . task_extract
   . task_sleep
   . task_delay
  and from the system startup routines.

  Normally, the scheduler is called with exec & co != -1...

  if exec & co. is set to -1 before calling scheduler(), the following
  stuffs have to be executed before the call
  - get the schedule_time
  - account the capacity if necessary
  - call an epilogue
  */

  /* then, we call the epilogue. the epilogue tipically checks the
     avail_time field... */
  if (exec_shadow != -1) {
    kern_epilogue_macro();

    l = proc_table[exec_shadow].task_level;
    level_table[l]->public_epilogue(l,exec_shadow);
  }

  //  kern_printf("[");

  l = level_first;
  for(;;) {
    do {
      p = level_table[l]->public_scheduler(l);
      //      kern_printf("p=%d",p);
      if (p != NIL)
        ok = level_table[ proc_table[p].task_level ]->
          public_eligible(proc_table[p].task_level,p);
      else
        ok = 0;
      //      kern_printf(" ok=%d",ok);      
    } while (ok < 0); /* repeat the level scheduler if the task isn't
                         eligible... (ex. in the aperiodic servers...) */
    if (p != NIL) break;

    l = level_next[l];  /* THERE MUST BE a level with a task to schedule */
    //    kern_printf(" l=%d",l);      
  };

  //  kern_printf("]");

  /* we follow the shadow chain */
  old_exec_shadow=exec_shadow;
  exec_shadow = exec = p;
  while (exec_shadow != proc_table[exec_shadow].shadow)
    exec_shadow = proc_table[exec_shadow].shadow;

  /* tracer stuff */
  TRACER_LOGEVENT(FTrace_EVT_task_schedule,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)proc_table[exec].context);
  //    kern_printf("[%i->%i]",old_exec_shadow,exec_shadow);

  /* we control the correctness of the shadows when we kill */
  proc_table[exec_shadow].status = EXE;
  
  //  kern_printf("(d%d)",exec_shadow);
  l = proc_table[exec_shadow].task_level;
  level_table[l]->public_dispatch(l, exec_shadow, exec!=exec_shadow);

  //  kern_printf("*");

  /* Finally,we post the capacity event, BUT
     . only if the task require that
     . only if exec==exec_shadow (if a task is blocked we don't want
       to check the capacity!!!) */
  if ((proc_table[exec_shadow].control & CONTROL_CAP)
      && exec==exec_shadow) {
    TIMESPEC_ASSIGN(&ty, &schedule_time);
    ADDUSEC2TIMESPEC(proc_table[exec_shadow].avail_time,&ty);
    //    kern_printf("s%d ns%d sched s%d ns%d",ty.tv_sec,ty.tv_nsec, schedule_time.tv_sec, schedule_time.tv_nsec);
    cap_timer = kern_event_post(&ty, capacity_timer, NULL);
  }
  /* set the time at witch the task is scheduled */
  TIMESPEC_ASSIGN(&cap_lasttime, &schedule_time);

  //  kern_printf("(s%d)",exec_shadow);
}


/*+
  Guarantee:
  This function guarantees the system: it calls the
  level_guarantee of each level that have that function != NULL

  The guarantee is based on a utilization factor basis.
  We mantain only a DWORD. num has to be interpreted as num/MAX_DWORD
  free bandwidth.
+*/
int guarantee()
{
  bandwidth_t num=MAX_BANDWIDTH;
  int l;

  for (l =0; l<MAX_SCHED_LEVEL && level_table[l]->public_guarantee; l++)
    if (!level_table[l]->public_guarantee(l,&num))
      return -1;

  return 0; /* OK */
}

/*----------------------------------------------------------------------*/
/* Context switch handling functions                                    */
/*----------------------------------------------------------------------*/
/* this function is called every time a context change occurs,
   when a task is preempted by an event called into an IRQ */
void kern_after_dispatch()
{
  /* every time a task wakes up from an IRQ, it has to check for async
     cancellation */
  check_killed_async();

  /* Then, look for pending signal delivery */
  kern_deliver_pending_signals();
}

/*----------------------------------------------------------------------*/
/* Kernel main system functions                                         */
/*----------------------------------------------------------------------*/

/*+
  This function initialize
  - the virtual machine (timer, interrupt, mem)
  the system's structures (queues, tables) , & the two task main &
  dummy, that are always present
+*/
void __kernel_init__(/* struct multiboot_info *multiboot */ void)
{
  int i,j;                                              /* counters */

  struct ll_initparms parms;                          /* for the VM */

  struct multiboot_info *multiboot=mbi_address();



  /*
   * Runlevel 0: kernel startup
   *
   *
   */

  runlevel = RUNLEVEL_STARTUP;

  /* The kernel startup MUST proceed with int disabled!    */
  kern_cli();

  /* First we initialize the memory allocator, because it is needed by
     __kernel_register_levels__     */
  kern_mem_init(multiboot);

  /* Clear the task descriptors */
  for (i = 0; i < MAX_PROC; i++) {
     proc_table[i].task_level   = -1;
     proc_table[i].stack        = NULL;
     proc_table[i].name[0]      = 0;
     proc_table[i].status       = FREE;
     proc_table[i].pclass       = 0;
     proc_table[i].group        = 0;
     proc_table[i].stacksize    = 0;
     proc_table[i].control      = 0;
     proc_table[i].frozen_activations = 0;
     proc_table[i].sigmask      = 0;
     proc_table[i].sigpending   = 0;
     proc_table[i].avail_time   = 0;
     proc_table[i].shadow       = i;
     proc_table[i].cleanup_stack= NULL;
     proc_table[i].errnumber    = 0;
     //proc_table[i].priority     = 0;
     //NULL_TIMESPEC(&proc_table[i].timespec_priority);
     proc_table[i].delay_timer  = -1;
     proc_table[i].wcet         = -1;

     proc_table[i].jet_tvalid   = 0;
     proc_table[i].jet_curr     = 0;
     proc_table[i].jet_max      = 0;
     proc_table[i].jet_sum      = 0;
     proc_table[i].jet_n        = 0;
     for (j=0; j<JET_TABLE_DIM; j++)
        proc_table[i].jet_table[j] = 0;

     proc_table[i].waiting_for_me = NIL;
     proc_table[i].return_value   = NULL;

     for (j=0; j<PTHREAD_KEYS_MAX; j++)
       proc_table[i].keys[j] = NULL;
  }

  /* set up the free descriptor queue */
  //  for (i = 0; i < MAX_PROC-1; i++) proc_table[i].next = i+1;
  //  proc_table[MAX_PROC-1].next = NIL;
  //  for (i = MAX_PROC-1; i > 0; i--) proc_table[i].prev = i-1;
  //  proc_table[0].prev = NIL;
  //  freedesc = 0;
  iq_init(&freedesc, NULL, 0);
  for (i = 0; i < MAX_PROC; i++)
    iq_insertlast(i,&freedesc);

  /* Set up the varius stuff */
  errnumber_global = 0;
  errnumber_shutdown = 0;
  task_counter     = 0;
  system_counter   = 0;
  exec             = -1;
  exec_shadow      = -1;
  cap_timer        = -1;
  NULL_TIMESPEC(&cap_lasttime);
  sched_levels     = 0;  /* They are not registered yet... */
  res_levels       = 0;
  calling_runlevel_func = 0;
  _exit_has_been_called = 0;

  /* Clear the key-specific data */
  task_specific_data_init();

  /* Clear exit and init functions */
  runlevel_init();

  /* Init VM layer (Interrupts, levels & memory management)           */
  /* for old exception handling, use excirq_init() */
  signals_init();
  set_default_exception_handler();

  /* Clear scheduling modules registration data */
  levels_init();

  sys_tick = __kernel_register_levels__(multiboot);

  /* test on system tick */
  if (sys_tick>=55000)  {
     printk("The system tick must be less than 55 mSec!");
     l1_exit(0);
  }

  /* OSLib initialization */
  if (sys_tick)
    parms.mode = LL_PERIODIC;
  else
    parms.mode = LL_ONESHOT; // one shot!!!

  parms.tick = sys_tick;

  /*
   * Runlevel INIT: Let's go!!!!
   *
   *
   */

  runlevel = RUNLEVEL_INIT;

  ll_init();
  event_init(&parms);
  seterrnumber(__errnumber);
  event_setprologue(event_resetepilogue);
  event_setlasthandler(kern_after_dispatch);

  /* call the init functions */
  call_runlevel_func(RUNLEVEL_INIT, 0);




  /*
   * Runlevel RUNNING: Hoping that all works fine ;-)
   *
   *
   */

  runlevel = RUNLEVEL_RUNNING;

  /* tracer stuff */
  #ifdef __OLD_TRACER__
    trc_resume();
  #endif
  
  /* exec and exec_shadow are already = -1 */
  kern_gettime(&schedule_time);
  scheduler();
  global_context = ll_context_from(); /* It will be used by exit & co. */
  ll_context_to(proc_table[exec_shadow].context);

  /*
   *
   * Now the system starts!!!
   * (hoping that someone has created some task(s) )
   * The function returns only at system end...
   *
   */


  /*
   * Runlevel SHUTDOWN: Shutting down the system... :-(
   *
   * We return here only when exit or _exit is called
   */

  event_setlasthandler(NULL);
  event_noreschedule = 0;

  // ll_abort(666); 
  /* tracer stuff */
  #ifdef __OLD_TRACER__
    trc_suspend();
  #endif

  remove_default_exception_handler(); 

  runlevel = RUNLEVEL_SHUTDOWN;
  
  //kern_printf("after  - system_counter=%d, task_counter = %d\n", system_counter,task_counter); 
  
  call_runlevel_func(RUNLEVEL_SHUTDOWN, _exit_has_been_called);
  
  //kern_printf("before - system_counter=%d, task_counter = %d\n", system_counter,task_counter);

  if (system_counter) {
    /* To shutdown the kernel correctly, we have to wait that all the SYSTEM
       tasks that are killable will die...
  
       We don't mess about the user task... we only kill them and reschedule
       The only thing important is that the system tasks shut down correctly.
       We do nothing for user tasks that remain active (because, for example,
       they have the cancelability set to deferred) when the system goes to
       runlevel 3 */

    //kern_printf("�lu",kern_gettime(NULL));
    kill_user_tasks();
    //kern_printf("�lu",kern_gettime(NULL)); 

    /* exec and exec_shadow are already = -1 */
    kern_gettime(&schedule_time);
    global_context = ll_context_from(); /* It will be used by sys_abort_shutdown */
    scheduler();

    event_setlasthandler(kern_after_dispatch);
    ll_context_to(proc_table[exec_shadow].context);
    event_setlasthandler(NULL);
  }

  /*
   * Runlevel BEFORE_EXIT: Before Halting the system
   *
   *
   */

  runlevel = RUNLEVEL_BEFORE_EXIT;

  /* the field global_errnumber is
     =0  if the system normally ends
     !=0 if an abort is issued
  */

  //kern_printf("Chiamo exit Functions\n"); 
  
  call_runlevel_func(RUNLEVEL_BEFORE_EXIT, _exit_has_been_called==_EXIT_CALLED);

  //kern_printf("Dopo exit Functions\n"); 

  /* Shut down the VM layer */
  ll_end();

  /*
   * Runlevel AFTER_EXIT: After halting...
   *
   *
   */

  runlevel = RUNLEVEL_AFTER_EXIT;

  //kern_printf("prima before Functions\n"); 

  call_runlevel_func(RUNLEVEL_AFTER_EXIT, 0);

  //kern_printf("dopo before Functions\n"); 
  kern_cli();
  if (errnumber_global) {
    kern_printf("exit() or _exit() called with code : %u\n", errnumber_global);
  }

  if (errnumber_shutdown) {
    kern_printf("sys_abort_shutdown() called with code : %u\n", errnumber_shutdown);
  }

  l1_exit(errnumber_shutdown);
}

/* Close the system when we are in runlevel shutdown */
void sys_abort_shutdown(int err)
{
  SYS_FLAGS f;
              
  /* Check if the system is in SHUTDOWN mode */
  if (runlevel != RUNLEVEL_SHUTDOWN)
    exit(0);
                                                                                                                             
  f = kern_fsave();

  errnumber_shutdown = err;

  if (ll_ActiveInt()) {
    event_noreschedule=1;
    ll_context_to(global_context);
    kern_frestore(f);
  }
  else {
    task_makefree(TASK_CANCELED);
    // the scheduler is not called here because we are switching to the
    // global context!
    ll_context_to(global_context);
  }
}


void internal_exit(int status, int reason)
{
  SYS_FLAGS f;

  /* if something goes wron during the real mode */
  if (runlevel==RUNLEVEL_STARTUP || runlevel==RUNLEVEL_AFTER_EXIT)
    l1_exit(status);

  /* Check if the system is in RUNNING mode */
  if (runlevel != RUNLEVEL_RUNNING) return;

  f = kern_fsave();

  errnumber_global = status;

  if (!_exit_has_been_called)
    _exit_has_been_called = reason;

  if (ll_ActiveInt()) {
    event_noreschedule=1;
    ll_context_to(global_context);
    kern_frestore(f);
  }
  else {
    task_makefree(TASK_CANCELED);
    // the scheduler is not called here because we are switching to the
    // global context!
    ll_context_to(global_context);
  }
}

void exit(int status)
{
  internal_exit(status, EXIT_CALLED);
}

void _exit(int status)
{
  internal_exit(status, _EXIT_CALLED);
}



/* this function is never called... used for the OSLib */
void sys_abort_tail(int code)
{
 //DUMMY!!!!
}



/*+ this primitive returns the time read from the system timer +*/
TIME sys_gettime(struct timespec *t)
{
  SYS_FLAGS f;
  TIME x;

  f = kern_fsave();
  x = kern_gettime(t);
  kern_frestore(f);

  return x;
}


