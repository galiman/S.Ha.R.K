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
 CVS :        $Id: kill.c,v 1.10 2005/01/08 14:42:52 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.10 $
 Last update: $Date: 2005/01/08 14:42:52 $
 ------------

 This file contains:

 - the function that kill a task (task_kill, group_kill)
 - the function that frees a task descriptor (makefree)
 - the task_abort

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

/*+ this structure contains the functions to be called to test if a
    task is blocked on a cancellation point +*/
static struct {
  int (*test)(PID p, void *arg);
  void *arg;
} cancellation_table[MAX_CANCPOINTS];

static int cancellation_points = 0;

void check_killed_async(void)
{
    if (proc_table[exec_shadow].control & KILL_ENABLED     &&
        !(proc_table[exec_shadow].control & KILL_DEFERRED) &&
	proc_table[exec_shadow].control & KILL_REQUEST &&       
        !(proc_table[exec_shadow].control & TASK_MAKEFREE)) {
      task_makefree(TASK_CANCELED);

      scheduler();

      ll_context_to(proc_table[exec_shadow].context);
      // never returns!!!
    }
}

/*+ This function register a cancellation point into the system.
    Be careful!!! no check are performed... +*/
void register_cancellation_point(int (*func)(PID p, void *arg), void *arg)
{
  cancellation_table[cancellation_points].test = func;
  cancellation_table[cancellation_points].arg = arg;
  cancellation_points++;
}


/*+
   This function frees the current task descriptor.
   It deletes the context and call the task_end and
   res_detach functions.
+*/
void task_makefree(void *ret)
{
    RLEVEL l;        /* used for managing the res_detach... */
    LEVEL lev;       /* for readableness */
    PID p;           /* used for controlling shadows */
    PID i;           /* used for readableness instead of exec_shadow */

    i = exec_shadow;
    
    /* Avoid a double call of task_makefree */
    proc_table[i].control |= TASK_MAKEFREE;

    /* first, contol if the task is joinable and someone is waiting for him
       This checks has to be done before  the shadow check (see below)
       because the task join uses the shadow field!!! */
    if (proc_table[i].control & TASK_JOINABLE) {
      proc_table[i].return_value = ret;
      proc_table[i].control |= WAIT_FOR_JOIN;
      if (proc_table[i].waiting_for_me != NIL) {
        /* someone is waiting for me... wake up him!!!
           Note that if the task woken up is killed by someone, the current
           task remain undetached; it is correct, as specified in 16.2.3 */
        register PID x;
        LEVEL l;

        x = proc_table[i].waiting_for_me;
        l = proc_table[x].task_level;
        level_table[l]->public_unblock(l,x);

        proc_table[x].shadow = x;
      }
    }

/*  {
   int xxx;
   kern_printf("(makefree ");
   for (xxx = 0; xxx<7; xxx++) kern_printf("p%d s%d|",xxx, proc_table[xxx].shadow);
   kern_printf(")\n");
  }*/

    /* we control if the task has someone with the shadow equal to
       the task being canceled... */
    for (p = 0; p<MAX_PROC; p++)
      if (p != i && proc_table[p].shadow == i) {
        kern_raise(XINVALID_KILL_SHADOW,i);
        return;
      }

    /* we call the cleanups functions */
    while (proc_table[i].cleanup_stack) {
      kern_sti();
      proc_table[i].cleanup_stack->f(proc_table[i].cleanup_stack->a);
      kern_cli();
      proc_table[i].cleanup_stack = proc_table[i].cleanup_stack->next;
    }

    /* call the thread-specific destructors */
    call_task_specific_data_destructors();

    /* Free the used context space */
    kern_context_delete(proc_table[i].context);
    
    /* tracer stuff */
    TRACER_LOGEVENT(FTrace_EVT_task_end,(unsigned short int)proc_table[i].context,(unsigned int)i);
    //kern_printf("[k%i]",i);
    
    /* Free the used stack area */
    /* Warning! we could be executing on the stack we are */
    /* going to destroy; this should cause no problem as  */
    /* the task_kill() cannot be interrupted & the memory */
    /* released can only be reassigned when we exit the   */
    /* primitive!                                         */
    if (!(proc_table[i].control & STACKADDR_SPECIFIED))
      kern_free(proc_table[i].stack,proc_table[i].stacksize);


    for (l=0; l<res_levels; l++)
      resource_table[l]->res_detach(l,i);

    lev = proc_table[i].task_level;
    level_table[lev]->public_end(lev,i);

    /* THIS ASSIGNMENT MUST STAY HERE!!!
       if we move it near the scheduler (after the counter checks)
       the kernel doesn't work, because:
       - if the task is the last one, a exit is called, but exec_shadow
         is != -1, so theexit calls the task_epilogue that reinsert
         the KILLED task into the ready queue!!!
    */
    exec = exec_shadow = -1;

    //DON'T REMOVE !!! BUG IF EXEC_SHADOW = -1
    //kern_epilogue_macro();
    /* there is no epilogue... */
    kern_gettime(&schedule_time); 	 
    /* we don't have to manage the capacity... because we are killing 	 
       ourselves */ 	 
    if (cap_timer != NIL) { 	 
      event_delete(cap_timer); 	 
      cap_timer = NIL; 	 
    }

    /* Decrement the Application task counter and end the system
       if necessary*/
    if (!(proc_table[i].control & SYSTEM_TASK)) {
      //kern_printf("€task%d€",task_counter);
      task_counter--;
      if (!task_counter && runlevel == RUNLEVEL_RUNNING) {
	_exit_has_been_called = EXIT_CALLED;
        ll_context_to(global_context);;
      }
    }
    else if (!(proc_table[i].control & NO_KILL)) {
      //kern_printf("€syst%d€",system_counter);
      system_counter--;
      if (!system_counter && runlevel == RUNLEVEL_SHUTDOWN)
        ll_context_to(global_context);
    }
}

/*
  This primitive autokills the excuting task; it was used to avoid
  that returning from a task cause a jmp to an unpredictable location.

  Now it is obsolete, the task_create_stub do all the works.

  It is used by the Posix layer to implement pthread_exit
*/
void task_abort(void *returnvalue)
{
    SYS_FLAGS f;

    f = kern_fsave();

    task_makefree(returnvalue);

    ll_context_to(proc_table[exec_shadow].context);
}

/*+
  This primitive kills the i-th task, free the descriptor, and the
  memory for the stack

  look also at kern_init in kern.c!!!
+*/
int task_kill(PID i)
{
    int j;        /* a counter */
    SYS_FLAGS f;

    f = kern_fsave();
    if (proc_table[i].control & NO_KILL ||
        proc_table[i].status == FREE) {
      errno = EINVALID_KILL;
      kern_frestore(f);
      return -1;
    }

    if (proc_table[i].control & KILL_REQUEST) {
      /* the task was already killed before... do nothing!!! */
      kern_frestore(f);
      return 0;
    }

    /* if cancellation is enabled and asyncronous (not deferred!),
       and it is a suicide, kill now, otherwise set the cancellation bit */
    if (i == exec_shadow &&
        proc_table[i].control & KILL_ENABLED &&
        !(proc_table[i].control & KILL_DEFERRED)) {
      task_makefree(TASK_CANCELED);

      ll_context_to(proc_table[exec_shadow].context);
      // never returns!!!
    }

    /* check if the task is blocked on a cancellation point */
    for (j=0; j<cancellation_points; j++)
       if (cancellation_table[j].test(i,cancellation_table[j].arg))
         break;

//    kern_printf("€%d", proc_table[i].control);

    proc_table[i].control |= KILL_REQUEST;

    kern_frestore(f);
    return 0;
}



/* similar to task_kill */
int group_kill(WORD g)
{
    BYTE autokill;
    register PID i;
    SYS_FLAGS f;

    int j;        /* a counter */
  
    if (g == 0) {
      errno = EINVALID_GROUP;
      return -1;
    }

    f = kern_fsave();

    /* Detect suicide */
    autokill = (g == proc_table[exec].group);

    for (i=0 ; i <  MAX_PROC; i++) {
        if (proc_table[i].control & NO_KILL      ||
            proc_table[i].status == FREE         ||
            proc_table[i].control & KILL_REQUEST ||
            proc_table[i].group != g             )
          continue;
    
        /* if cancellation is enabled and asyncronous (not deferred!),
           and it is a suicide, kill now, otherwise set the cancellation bit */
        if (i == exec_shadow &&
            proc_table[i].control & KILL_ENABLED &&
            !(proc_table[i].control & KILL_DEFERRED)) {
          task_makefree(TASK_CANCELED);
          continue;
        }

        /* check if the task is blocked on a cancellation point */
        for (j=0; j<cancellation_points; j++)
           if (cancellation_table[j].test(i,cancellation_table[j].arg))
             break;
    
        proc_table[i].control |= KILL_REQUEST;

    }

    /* If we were killing ourselves we must switch context */
    /* without saving the actual one                       */
    if (autokill) {
      ll_context_to(proc_table[exec].context);
    }
    else
      kern_frestore(f);

    return 0;
}


/* this function is used into kernel_init to kill all the user tasks when
   the system goes to runlevel 2... */
void kill_user_tasks()
{
  PID k;
  int j;

  for (k=0; k<MAX_PROC; k++) {
    /* kill the task k!!! (see also task_kill in kill.c!!!) */
    if (proc_table[k].status == FREE ||
        proc_table[k].control & (KILL_REQUEST|NO_KILL|SYSTEM_TASK) )
      /* the task was already killed before... do nothing!!! */
      continue;

//    kern_printf("€KILLING %d€",k);

    /* check if the task is blocked on a cancellation point */
    for (j=0; j<cancellation_points; j++)
       if (cancellation_table[j].test(k,cancellation_table[j].arg))
         break;

    proc_table[k].control |= KILL_REQUEST;
  }
}
