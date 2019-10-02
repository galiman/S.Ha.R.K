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
 CVS :        $Id: posix.c,v 1.2 2005/02/25 11:02:25 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2005/02/25 11:02:25 $
 ------------

 This file contains the scheduling module compatible with POSIX
 specifications

 Read posix.h for further details.

 RR tasks have the CONTROL_CAP bit set

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
 * but WITHOUT ANY WARR2ANTY; without even the implied waRR2anty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <posix/posix/posix.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>
#include <posix/posix/comm_message.h>

/*+ Status used in the level +*/
#define POSIX_READY   MODULE_STATUS_BASE

/*+ Use for change level in POSIX +*/
#define POSIX_CHANGE_LEVEL 1


/*+ the level redefinition for the Round Robin level +*/
typedef struct {
  level_des l;          /*+ the standard level descriptor          +*/

  int nact[MAX_PROC];   /*+ number of pending activations          +*/
  
  int priority[MAX_PROC]; /*+ priority of each task                +*/

  IQUEUE *ready;        /*+ the ready queue array                  +*/

  int slice;            /*+ the level's time slice                 +*/

  struct multiboot_info *multiboot; /*+ used if the level have to insert
                                        the main task +*/
  int maxpriority;      /*+ the priority are from 0 to maxpriority
                            (i.e 0 to 31)                          +*/

  int yielding;         /*+ equal to 1 when a sched_yield is called +*/

  /* introduce for changing level in POSIX */
  int flag[MAX_PROC];

  int new_level[MAX_PROC];
  int new_slice[MAX_PROC];
  int new_control[MAX_PROC];

} POSIX_level_des;

/* This is not efficient but very fair :-)
   The need of all this stuff is because if a task execute a long time
   due to (shadow!) priority inheritance, then the task shall go to the
   tail of the queue many times... */
static PID POSIX_public_scheduler(LEVEL l)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  PID p;

  int prio;

  prio = lev->maxpriority;

  for (;;) {
    p = iq_query_first(&lev->ready[prio]);
    if (p == NIL) {
      if (prio) {
        prio--;
        continue;
      }
      else
        return NIL;
    }

    if ((proc_table[p].control & CONTROL_CAP) &&
        (proc_table[p].avail_time <= 0)) {
      proc_table[p].avail_time += proc_table[p].wcet;
      iq_extract(p,&lev->ready[prio]);
      iq_insertlast(p,&lev->ready[prio]);
    }
    else
      return p;
  }
}

static int POSIX_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);
  NRT_TASK_MODEL *nrt;

  if (m->pclass != NRT_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;

  nrt = (NRT_TASK_MODEL *)m;

  /* the task state is set at SLEEP by the general task_create */

  /* I used the wcet field because using wcet can account if a task
     consume more than the timeslice... */

  if (nrt->inherit == NRT_INHERIT_SCHED &&
      proc_table[exec_shadow].task_level == l) {
    /* We inherit the scheduling properties if the scheduling level
       *is* the same */
    lev->priority[p] = lev->priority[exec_shadow];
  
    proc_table[p].avail_time = proc_table[exec_shadow].avail_time;
    proc_table[p].wcet       = proc_table[exec_shadow].wcet;

    proc_table[p].control = (proc_table[p].control & ~CONTROL_CAP) |
                            (proc_table[exec_shadow].control & CONTROL_CAP);
  
    lev->nact[p] = (lev->nact[exec_shadow] == -1) ? -1 : 0;
  }
  else {
    lev->priority[p] = nrt->weight;
  
    if (nrt->slice) {
      proc_table[p].avail_time = nrt->slice;
      proc_table[p].wcet       = nrt->slice;
    }
    else {
      proc_table[p].avail_time = lev->slice;
      proc_table[p].wcet       = lev->slice;
    }
  
    if (nrt->policy == NRT_RR_POLICY)
      proc_table[p].control   |= CONTROL_CAP;
  
    if (nrt->arrivals == SAVE_ARRIVALS)
      lev->nact[p] = 0;
    else
      lev->nact[p] = -1;
  }

  lev->flag[p] = 0;

  return 0; /* OK */
}

static void POSIX_public_dispatch(LEVEL l, PID p, int nostop)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  /* the task state is set EXE by the scheduler()
     we extract the task from the ready queue
     NB: we can't assume that p is the first task in the queue!!! */
  iq_extract(p, &lev->ready[lev->priority[p]]);
}

static void POSIX_public_epilogue(LEVEL l, PID p)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  /* Change task level */
  if (lev->flag[p] & POSIX_CHANGE_LEVEL) {

    STD_command_message msg;

    proc_table[p].status = SLEEP;
    proc_table[p].task_level = lev->new_level[p];
    msg.command = STD_ACTIVATE_TASK;
    level_table[lev->new_level[p]] -> public_message(lev->new_level[p],p,&msg);
    return;

  }

  if (lev->yielding) {
    lev->yielding = 0;
    iq_insertlast(p,&lev->ready[lev->priority[p]]);
  }
  /* check if the slice is finished and insert the task in the coPOSIXect
     qqueue position */
  else if (proc_table[p].control & CONTROL_CAP &&
           proc_table[p].avail_time <= 0) {
    proc_table[p].avail_time += proc_table[p].wcet;
    iq_insertlast(p,&lev->ready[lev->priority[p]]);
  }
  else
    iq_insertfirst(p,&lev->ready[lev->priority[p]]);

  proc_table[p].status = POSIX_READY;
}

static void POSIX_public_activate(LEVEL l, PID p, struct timespec *t)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  /* Test if we are trying to activate a non sleeping task    */
  /* save activation (only if needed...) */
  if (proc_table[p].status != SLEEP) {
    if (lev->nact[p] != -1)
      lev->nact[p]++;
    return;
  }

  /* Insert task in the correct position */
  proc_table[p].status = POSIX_READY;
  iq_insertlast(p,&lev->ready[lev->priority[p]]);
}

static void POSIX_public_unblock(LEVEL l, PID p)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  /* Similar to POSIX_task_activate, but we don't check in what state
     the task is */

  /* Insert task in the coPOSIXect position */
  proc_table[p].status = POSIX_READY;
  iq_insertlast(p,&lev->ready[lev->priority[p]]);
}

static void POSIX_public_block(LEVEL l, PID p)
{
  /* Extract the running task from the level
     . we have already extract it from the ready queue at the dispatch time.
     . the capacity event have to be removed by the generic kernel
     . the wcet don't need modification...
     . the state of the task is set by the calling function

     So, we do nothing!!!
  */
}

#ifdef OLDVERSION
static int POSIX_public_message(LEVEL l, PID p, void *m)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  if (lev->nact[p] > 0) {
    /* continue!!!! */
    lev->nact[p]--;
    iq_insertfirst(p,&lev->ready[lev->priority[p]]);
    proc_table[p].status = POSIX_READY;
  }
  else
    proc_table[p].status = SLEEP;

  jet_update_endcycle(); /* Update the Jet data... */
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l); 

  return 0;
}
#else
static int POSIX_public_message(LEVEL l, PID p, void *m)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);
  STD_command_message *msg;
  
  NRT_TASK_MODEL *nrt;

  /* Task Endcycle */
  switch ((long)(m)) {
  
    case (long)(NULL):
      if (lev->nact[p] > 0) {
        /* continue!!!! */
        lev->nact[p]--;
        iq_insertfirst(p,&lev->ready[lev->priority[p]]);
        proc_table[p].status = POSIX_READY;
      } else
        proc_table[p].status = SLEEP;

      jet_update_endcycle(); /* Update the Jet data... */
      TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l); 
      
      break;

   /* Task Disable */
   case (long)(1):

     break;

   default:

     msg = (STD_command_message *)m;

     switch(msg->command) {
       case STD_SET_NEW_LEVEL:       
         lev->flag[p] |= POSIX_CHANGE_LEVEL;
         lev->new_level[p] = (int)(msg->param);

         break;
       case STD_SET_NEW_MODEL:
	 
         nrt = (NRT_TASK_MODEL *)(msg->param);

         lev->priority[p] = nrt->weight;
                                                                                                                             
         if (nrt->slice) {
           lev->new_slice[p] = nrt->slice;
         } else {
           lev->new_slice[p] = 0;
         }
                                                                                                                             
         if (nrt->policy == NRT_RR_POLICY)
           lev->new_control[p] |= CONTROL_CAP;
                                                                                                                             
         if (nrt->arrivals == SAVE_ARRIVALS)
           lev->nact[p] = 0;
         else
           lev->nact[p] = -1;
 
         lev->flag[p] = 0;

         break;

       case STD_ACTIVATE_TASK:

         if (lev->new_slice[p]) {
           proc_table[p].avail_time = lev->new_slice[p];
           proc_table[p].wcet = lev->new_slice[p];
         } else {
           proc_table[p].avail_time = lev->slice;
           proc_table[p].wcet = lev->slice;
         }

         proc_table[p].control = lev->new_control[p];

         POSIX_public_activate(l,p, NULL);

         break;
         
     }

     break;

  }
 
  return 0;
}

#endif

static void POSIX_public_end(LEVEL l, PID p)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  lev->nact[p] = -1;

  /* then, we insert the task in the free queue */
  proc_table[p].status = FREE;
  iq_priority_insert(p,&freedesc);
}

/* Registration functions */

/*+ This init function install the "main" task +*/
static void POSIX_call_main(void *l)
{
  LEVEL lev;
  PID p;
  NRT_TASK_MODEL m;
  void *mb;

  lev = (LEVEL)l;

  nrt_task_default_model(m);
  nrt_task_def_level(m,lev); /* with this we are sure that the task aPOSIXives
                                to the coPOSIXect level */

  mb = ((POSIX_level_des *)level_table[lev])->multiboot;
  nrt_task_def_arg(m,mb);
  nrt_task_def_usemath(m);
  nrt_task_def_nokill(m);
  nrt_task_def_ctrl_jet(m);
  nrt_task_def_weight(m,0);
  nrt_task_def_policy(m,NRT_RR_POLICY);
  nrt_task_def_inherit(m,NRT_EXPLICIT_SCHED);

  p = task_create("Main", __init__, (TASK_MODEL *)&m, NULL);

  if (p == NIL)
    printk("\nPanic!!! can't create main task...\n");

  POSIX_public_activate(lev,p,NULL);
}


/*+ Registration function:
    TIME slice                the slice for the Round Robin queue
    int createmain            1 if the level creates the main task 0 otherwise
    struct multiboot_info *mb used if createmain specified   +*/
LEVEL POSIX_register_level(TIME slice,
                       int createmain,
                       struct multiboot_info *mb,
                       int prioritylevels)
{
  LEVEL l;            /* the level that we register */
  POSIX_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */
  int x;              /* a counter */

  printk("POSIX_register_level\n");

  l = level_alloc_descriptor(sizeof(POSIX_level_des));

  lev = (POSIX_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.public_scheduler = POSIX_public_scheduler;
  lev->l.public_create    = POSIX_public_create;
  lev->l.public_end       = POSIX_public_end;
  lev->l.public_dispatch  = POSIX_public_dispatch;
  lev->l.public_epilogue  = POSIX_public_epilogue;
  lev->l.public_activate  = POSIX_public_activate;
  lev->l.public_unblock   = POSIX_public_unblock;
  lev->l.public_block     = POSIX_public_block;
  lev->l.public_message   = POSIX_public_message;
  lev->l.public_guarantee = NULL;

  /* fill the POSIX descriptor part */
  for (i = 0; i < MAX_PROC; i++) {
    lev->nact[i] = -1;
    lev->flag[i] = 0 ;
    lev->new_level[i] = -1;
    lev->new_slice[i] = -1;
    lev->new_control[i] = 0;
    
  }

  lev->maxpriority = prioritylevels -1;

  lev->ready = (IQUEUE *)kern_alloc(sizeof(IQUEUE) * prioritylevels);

  for (x = 0; x < prioritylevels; x++)
    iq_init(&lev->ready[x], &freedesc, 0);

  if (slice < POSIX_MINIMUM_SLICE) slice = POSIX_MINIMUM_SLICE;
  if (slice > POSIX_MAXIMUM_SLICE) slice = POSIX_MAXIMUM_SLICE;
  lev->slice      = slice;

  lev->multiboot  = mb;

  if (createmain)
    sys_atrunlevel(POSIX_call_main,(void *) l, RUNLEVEL_INIT);

  return l;
}

/*+ this function forces the running task to go to his queue tail;
    (it works only on the POSIX level) +*/
int POSIX_sched_yield(LEVEL l)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  if (proc_table[exec_shadow].task_level != l)
    return -1;

  proc_table[exec_shadow].context = kern_context_save();
  lev->yielding = 1;
  scheduler();
  kern_context_load(proc_table[exec_shadow].context);
  return 0;
}

/*+ this function returns the maximum level allowed for the POSIX level +*/
int POSIX_get_priority_max(LEVEL l)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);
  return lev->maxpriority;
}

/*+ this function returns the default timeslice for the POSIX level +*/
int POSIX_rr_get_interval(LEVEL l)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);
  return lev->slice;
}

/*+ this functions returns some paramaters of a task;
    policy must be NRT_RR_POLICY or NRT_FIFO_POLICY;
    priority must be in the range [0..prioritylevels]
    returns ENOSYS or ESRCH if there are problems +*/
int POSIX_getschedparam(LEVEL l, PID p, int *policy, int *priority)
{
  if (p<0 || p>= MAX_PROC || proc_table[p].status == FREE)
    return ESRCH;

  if (proc_table[p].task_level != l)
    return ENOSYS;

  if (proc_table[p].control & CONTROL_CAP)
    *policy = NRT_RR_POLICY;
  else
    *policy = NRT_FIFO_POLICY;

  *priority = ((POSIX_level_des *)(level_table[l]))->priority[p];

  return 0;
}

/*+ this functions sets paramaters of a task +*/
int POSIX_setschedparam(LEVEL l, PID p, int policy, int priority)
{
  POSIX_level_des *lev = (POSIX_level_des *)(level_table[l]);

  if (p<0 || p>= MAX_PROC || proc_table[p].status == FREE)
    return ESRCH;

  if (proc_table[p].task_level != l)
    return ENOSYS;

  if (policy == SCHED_RR)
    proc_table[p].control |= CONTROL_CAP;
  else if (policy == SCHED_FIFO)
    proc_table[p].control &= ~CONTROL_CAP;
  else
    return EINVAL;

  if (lev->priority[p] != priority) {
    if (proc_table[p].status == POSIX_READY) {
      iq_extract(p,&lev->ready[lev->priority[p]]);
      lev->priority[p] = priority;
      iq_insertlast(p,&lev->ready[priority]);
    }
    else
      lev->priority[p] = priority;
  }

  return 0;
}



