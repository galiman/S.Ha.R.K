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
 CVS :        $Id: grpcreat.c,v 1.10 2005/01/08 14:48:03 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.10 $
 Last update: $Date: 2005/01/08 14:48:03 $
 ------------

 This file contains:

 - the function that creates a task

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
//#include <kernel/trace.h>
#include <tracer.h>

static DWORD unique_task_ID = 0;

/*+ Task create stub:
    when a task is created, the context is initialized to call this stub.
    Why??? for 2 reasons:
    - The signal delivery is done in the kern_context_load. When a task is
      created and activated but before the first dispatch, some signals
      could arrive... so the first thing a task have to do is to deliver
      pending signals...
    - When a task ends, it can return a value, so the value can be easily
      managed in the stub to implement task_join & co...
+*/
static void task_create_stub(void *arg)
{
  void *ret;
  kern_after_dispatch();

  ret = proc_table[exec_shadow].body(arg);

  kern_cli();
  //  kern_printf("EXIT task %d, value %d\n",exec_shadow,ret); 
  task_makefree(ret);
  scheduler();
  // kern_printf("MAKEFREE return exec_shadow=%d task_counter=%d\n",exec_shadow,task_counter); 

  ll_context_to(proc_table[exec_shadow].context);
}


/*+
  Allocate & fill a task descriptor; 
  After that call the task is ready to be guaranteed. 
  look at group_create for some other comments :-)
  +*/
static
PID internal_create_init(char *name, /*+ the symbolic name of the task +*/
			 TASK (*body)(), /*+ a pointer to the task body    +*/
			 TASK_MODEL *m,  /*+ the task model                +*/
			 va_list rlist)  /*+ used to manage the resources 
					   models +*/
{
  PID i = NIL;     /* the task descriptor to fill... */
  LEVEL l;         /* used for searching the correct level of the task    */
  RLEVEL l_res;    /* used for searching the correct resource level ...   */
  RES_MODEL *r;    /* used for managing the resources models */
  int j;           /* a counter */

  /* Get a free descriptor */
  for (;;) {
    i = iq_getfirst(&freedesc);

    /* If no one is available abort the system */
    if (i == NIL) {
      errno = ENO_AVAIL_TASK;
      return -1;
    }

    if (!(proc_table[i].control & WAIT_FOR_JOIN))
      break;

    proc_table[i].control |= DESCRIPTOR_DISCARDED;
  }

  /* Fill in the descriptor */
  proc_table[i].task_ID         = unique_task_ID++;
  proc_table[i].body            = body;
  strncpy(proc_table[i].name,name,19);
  proc_table[i].name[19]        = 0;
  proc_table[i].status          = SLEEP;    /* Task is not active when created */
  proc_table[i].pclass          = m->pclass & 0xFF00;
  proc_table[i].group           = m->group;
  proc_table[i].stacksize       = (m->stacksize == 0) ? STACK_SIZE : m->stacksize;
  proc_table[i].control         = m->control | KILL_DEFERRED | KILL_ENABLED;
  proc_table[i].frozen_activations = 0;
  proc_table[i].sigmask         = proc_table[exec_shadow].sigmask; /* mask inherit.*/
  proc_table[i].sigpending      = 0; /* No pending signal for new tasks*/
  proc_table[i].shadow          = i;
  proc_table[i].cleanup_stack   = NULL;
  //  proc_table[i].next            = proc_table[i].prev = NIL;
  proc_table[i].errnumber       = 0;        /* meaningless value */

  /* Fill jet info */
  proc_table[i].jet_tvalid      = 0;
  proc_table[i].jet_curr        = 0;
  proc_table[i].jet_max         = 0;
  proc_table[i].jet_sum         = 0;
  proc_table[i].jet_n           = 0;
  for (j=0; j<JET_TABLE_DIM; j++)
     proc_table[i].jet_table[j] = 0;

  proc_table[i].waiting_for_me  = NIL;
  proc_table[i].return_value    = NULL;

  proc_table[i].keys[0] = &proc_table[i].cleanup_stack;
  for (j=1; j<PTHREAD_KEYS_MAX; j++)
    proc_table[i].keys[j] = NULL;

  /* now, the task descriptor has some fields not initializated:
     - master_level   (initialized later, modified by l[]->task_create() )
     - task_level     (initialized later in this function)
     - context, stack (initialized at the end of this function)
     - additional stuff like priority & co. have to be init. only if used...)
     - delay_timer    (initialized in __kernel_init__ and mantained coherent
                       by the scheduling modules...)
     - guest_pclass   (set only in guest_create and used with guest task)
  */

  /* search for a level that can manage the task model */
  for (l=level_first; l != -1; l=level_next[l])
    if (level_table[l]->public_create(l,i,m) >= 0)
      break;

  if (l == -1) {
    /* no level can accept the task_model, exit!!! */
    proc_table[i].status = FREE;
    iq_insertfirst(i,&freedesc);
    errno = ENO_AVAIL_SCHEDLEVEL;
    return -1;
  }

  /* initialize task level */
  proc_table[i].task_level = l;

  /* register all the resource models passed */
  for (;;) {
    r = va_arg(rlist,RES_MODEL *);

    if (!r) break;   /* all the resource models are managed */

    /* search for a level that can manage the resource model */
    for (l_res=0; l_res<res_levels; l_res++)
      if (resource_table[l_res]->res_register(l_res, i, r) >= 0)
        break;
    if (l_res == res_levels) {
      /* no level can accept the resource_model, exit!!! */
      /* detach the resources and the task */
      group_create_reject(i);
      errno = ENO_AVAIL_RESLEVEL;
      return -1;
    }
  }
  return i;
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
                 ...)
{
  PID p;
  va_list rlist;

  va_start(rlist, m);
  p = internal_create_init(name, body, m, rlist);
  va_end(rlist);

  return p;
}

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
int group_create_accept(PID i, TASK_MODEL *m)
{
  CONTEXT c;       /* the context of the new task */
  BYTE *tos;       /* top of stack of the new task */

  /* Allocate a stack for the task, only if stackaddr != NULL */
  if (m->stackaddr) {
    tos = proc_table[i].stack = m->stackaddr;
    proc_table[i].control |= STACKADDR_SPECIFIED;
  }
  else {
    tos = proc_table[i].stack = (BYTE *) kern_alloc(proc_table[i].stacksize);
    if (proc_table[i].stack == NULL) {
      group_create_reject(i);
      errno = ENO_AVAIL_STACK_MEM;
      return -1;
    }
  }
  

  /* Set up the initial context */
  tos += proc_table[i].stacksize;
  c = kern_context_create(task_create_stub,tos,m->arg,NULL,m->control);
  // { extern CONTEXT global_context; 
  if (!c) { // || c == global_context) {
    /*    grx_close();
    { int i;
    for (i = 0; i<10000; i++) 
    kern_printf("!!!\n"); ll_abort(666);*/
    kern_free(tos, proc_table[i].stacksize);
    group_create_reject(i);
    errno = ENO_AVAIL_TSS;
    return -1;
  }
/*
  printf_xy(0,0,WHITE,"context = %d global=%d stack=%p",c,global_context,proc_table[i].stack);
  }*/
  proc_table[i].context = c;

  /* Tracer stuff */
  TRACER_LOGEVENT(FTrace_EVT_task_create,(unsigned short int)c,(unsigned int)i);

  //kern_printf("[c%i %i]",i,proc_table[i].context);
	      
  /* Count the task if it is an Application or System Task... */
  if (!(m->control & SYSTEM_TASK))
    task_counter++;
  else if (!(m->control & NO_KILL))
    system_counter++;

  return 0;
}

/*
  This function should be called when a task created with group_create
  can not be successfully guaranteed.
  This function reject the task from the system.
  You cannot use the PID of a rejected task after this call.
*/
void group_create_reject(PID i)
{
  LEVEL  l;     /* stores the level of a task */
  RLEVEL lr;    /* used for searching the correct resource level ...   */

  for (lr=0; lr<res_levels; lr++)
    resource_table[lr]->res_detach(lr,i);

  l = proc_table[i].task_level;  
  level_table[l]->public_detach(l,i);
  
  proc_table[i].status = FREE;
  
  iq_insertfirst(i,&freedesc);
}



/*+
  Allocate & fill a task descriptor; the task is not explicitely
  activated; you have to use the task_activate to do this
  Just set up the minimum necessary to make thing works well           

  This function creates and guarantees a task using the group_create
  functions.
  If in your code you never need to create group of tasks, consider replacing
  this file with the file kernel/create.c, that handle all the task
  creation process in a single function call.
+*/
PID task_createn(char *name,      /*+ the symbolic name of the task +*/
                 TASK (*body)(),  /*+ a pointer to the task body    +*/
		 TASK_MODEL *m,   /*+ the task model                +*/
                 ...)             /*+ the resources models, a list
                                     of RES_MODEL * terminated by NULL +*/
{
  PID p;           /* the task descriptor to fill... */
  va_list rlist;   /* used for managing the resources models */
  SYS_FLAGS f;     /* the flags to be restored at the end
                      (we must save them because the task_create can be
                       called at system initialization) */

  f = kern_fsave();

  va_start(rlist, m);
  p = internal_create_init(name, body, m, rlist);
  va_end(rlist);

  if (p != NIL) {
    if (level_table[proc_table[p].task_level]->public_guarantee)
      if (guarantee() < 0) {
	group_create_reject(p);
	errno = ENO_GUARANTEE;
	kern_frestore(f);
	return -1;
      }
    
    if (group_create_accept(p,m)) p = -1;
  }
  kern_frestore(f);
  
  return p;
}







