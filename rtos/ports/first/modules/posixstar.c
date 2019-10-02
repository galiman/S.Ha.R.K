/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Trimarchi Michael   <trimarchi@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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


#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include "posixstar.h"
#include "fsf_configuration_parameters.h"
#include "fsf_core.h"
#include "fsf_server.h"
#include <posix/posix/comm_message.h>

//#define POSIXSTAR_DEBUG

/*+ Status used in the level +*/
#define POSIXSTAR_READY   MODULE_STATUS_BASE

#define POSIXSTAR_CHANGE_LEVEL 1

/* flags */
#define POSIXSTAR_FLAG_NOPREEMPT   4 

/*+ the level redefinition for the Round Robin level +*/
typedef struct {
  level_des l;          /*+ the standard level descriptor          +*/

  int nact[MAX_PROC];   /*+ number of pending activations          +*/
  
  int priority[MAX_PROC]; /*+ priority of each task                +*/

  IQUEUE *ready;        /*+ the ready queue array                  +*/

  int slice;            /*+ the level's time slice                 +*/

  int maxpriority;      /*+ the priority are from 0 to maxpriority
                            (i.e 0 to 31)                          +*/

  int yielding;         /*+ equal to 1 when a sched_yield is called +*/

  int budget[MAX_PROC];

  int flag[MAX_PROC];

  int new_level[MAX_PROC];
  int new_slice[MAX_PROC];
  int new_control[MAX_PROC];

  PID activated;
  int scheduling_level;
  int cap_lev;
  struct timespec cap_lasttime;
 
} POSIXSTAR_level_des;

static void capacity_handler(void *l)
{
  POSIXSTAR_level_des *lev = l;
  lev->cap_lev = NIL;
  event_need_reschedule();
}
/* the private scheduler choice a task and insert in cbsstar module */
/* This is not efficient but very fair :-)
   The need of all this stuff is because if a task execute a long time
   due to (shadow!) priority inheritance, then the task shall go to the
   tail of the queue many times... */

static void POSIXSTAR_private_scheduler(POSIXSTAR_level_des * lev)
{
  /* the old posix scheduler select the private job for CBS */
  PID p = NIL;

  int prio;


  prio = lev->maxpriority;
  
  for (;;) {
    p = iq_query_first(&lev->ready[prio]);
    if (p == NIL) {
      if (prio) {
        prio--;
        continue;
      }
      else {
        p=NIL;
	break;
      }
    }

    if (proc_table[p].avail_time <= 0) {
      while (proc_table[p].avail_time<=0)
  	proc_table[p].avail_time += proc_table[p].wcet;
      iq_extract(p,&lev->ready[prio]);
      iq_insertlast(p,&lev->ready[prio]);
    }
    else {
      break;
    }
  }

  /* check if the task is preempteble or not */
  if (lev->activated != NIL && (lev->flag[lev->activated] & POSIXSTAR_FLAG_NOPREEMPT)) return;
 
  if (p!=lev->activated) {
    if (lev->activated != NIL )  {
     level_table[ lev->scheduling_level ]->
       private_extract(lev->scheduling_level, lev->activated);
   }
   lev->activated = p;
   
   if (p != NIL) {
     BUDGET_TASK_MODEL b;
     budget_task_default_model(b, lev->budget[p]);
     #ifdef POSIXSTAR_DEBUG
       kern_printf("(PS:SchAct:%d:%d)",p,lev->budget[p]);
     #endif
     level_table[ lev->scheduling_level ]->
       private_insert(lev->scheduling_level, p, (TASK_MODEL *)&b);
   }
 }
}

static int POSIXSTAR_public_eligible(LEVEL l, PID p)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  return level_table[ lev->scheduling_level ]->
      private_eligible(lev->scheduling_level,p);
  
  return 0;
}


static void POSIXSTAR_account_capacity(POSIXSTAR_level_des *lev, PID p)
{
  struct timespec ty;
  TIME tx;

  SUBTIMESPEC(&schedule_time, &lev->cap_lasttime, &ty);
  tx = TIMESPEC2USEC(&ty);

  proc_table[p].avail_time -= tx;

}

static int POSIXSTAR_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  NRT_TASK_MODEL *nrt;

  /* DEBUG */
  #ifdef POSIXSTAR_DEBUG
    kern_printf("(PS:Crt:%d)",p);
  #endif

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

    lev->nact[p] = (lev->nact[exec_shadow] == -1) ? -1 : 0;
  }
  else {
    if (nrt->weight<=lev->maxpriority)
      lev->priority[p] = nrt->weight;
    else lev->priority[p]=lev->maxpriority;
  
    if (nrt->slice) {
      proc_table[p].avail_time = nrt->slice;
      proc_table[p].wcet       = nrt->slice;
    }
    else {
      proc_table[p].avail_time = lev->slice;
      proc_table[p].wcet       = lev->slice;
    }
 
    #if defined POSIXSTAR_DEBUG
        kern_printf("(slice %d)", proc_table[p].wcet);
    #endif 
    if (nrt->arrivals == SAVE_ARRIVALS)
      lev->nact[p] = 0;
    else
      lev->nact[p] = -1;
  
  }

  lev->flag[p] = 0;
  proc_table[p].control = (proc_table[p].control & ~CONTROL_CAP);

  return 0;

}

static void POSIXSTAR_public_dispatch(LEVEL l, PID p, int nostop)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  struct timespec ty;
  /* the task state is set EXE by the scheduler()
     we extract the task from the ready queue
     NB: we can't assume that p is the first task in the queue!!! */

  #ifdef POSIXSTAR_DEBUG
     if( !nostop) kern_printf("(PS:Dsp:%d)",p); else
	 kern_printf("(PS:Dsp_shad:%d)",p);
  #endif

  if (!nostop || proc_table[exec].task_level==l) {
    TIMESPEC_ASSIGN(&ty, &schedule_time);
    TIMESPEC_ASSIGN(&lev->cap_lasttime, &schedule_time);
     
  /* ...and finally, we have to post a capacity event on exec task because the shadow_task consume
   *        capacity on exe task always */
    ADDUSEC2TIMESPEC(proc_table[exec].avail_time, &ty);
    
    lev->cap_lev = kern_event_post(&ty,capacity_handler, lev);
    level_table[lev->scheduling_level]->private_dispatch(lev->scheduling_level, p, nostop);
  }
  else 
    level_table[proc_table[exec].task_level]->public_dispatch(proc_table[exec].task_level, p, nostop);
  
}

static int POSIXSTAR_private_change_level(LEVEL l, PID p)
{

  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  /* Change task level */
  if (lev->flag[p] & POSIXSTAR_CHANGE_LEVEL) {
 
    STD_command_message msg;
 
    proc_table[p].status = SLEEP;
    lev->flag[p] &= ~POSIXSTAR_CHANGE_LEVEL;

    level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);
    iq_extract(p,&lev->ready[lev->priority[p]]);

    POSIXSTAR_private_scheduler(lev);

    lev->nact[p] = 0;
    lev->budget[p] = -1;
    
    proc_table[p].task_level = lev->new_level[p];
    msg.command = STD_ACTIVATE_TASK;
    level_table[lev->new_level[p]] -> public_message(lev->new_level[p],p,&msg);

    return 1;

  }

  return 0;

}

static void POSIXSTAR_public_epilogue(LEVEL l, PID p)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
 
  #ifdef POSIXSTAR_DEBUG
    kern_printf("(PS:Epi:%d)",p);
  #endif 

 if (lev->cap_lev!=NIL) {
   kern_event_delete(lev->cap_lev);
   lev->cap_lev=NIL;
 }

 if (proc_table[exec].task_level==l ) {
    
   POSIXSTAR_account_capacity(lev,exec);

   if (POSIXSTAR_private_change_level(l,p)) return;
   
   if (lev->yielding) {
     lev->yielding = 0;
     iq_extract(p,&lev->ready[lev->priority[exec]]);
     iq_insertlast(p,&lev->ready[lev->priority[exec]]);
   } else {

     if (proc_table[exec].avail_time <= 0) {
 
       POSIXSTAR_private_scheduler(lev);
    
       if (exec==lev->activated) {
         level_table[lev->scheduling_level]->private_epilogue(lev->scheduling_level,p);  
       } 

     } else  {
         
       level_table[lev->scheduling_level]->private_epilogue(lev->scheduling_level,p);
      
     }
      
   }
     
   proc_table[exec].status = POSIXSTAR_READY;
  
 } else { 
   
   level_table[proc_table[exec].task_level]->public_epilogue(proc_table[exec].task_level,p);
 
 } 
 
}

static void POSIXSTAR_internal_activate(POSIXSTAR_level_des *lev, PID p) 
{

  /* Insert task in the correct position */
  proc_table[p].status = POSIXSTAR_READY;
  iq_insertlast(p,&lev->ready[lev->priority[p]]);

}

	
static void POSIXSTAR_public_activate(LEVEL l, PID p, struct timespec *t)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  /* Test if we are trying to activate a non sleeping task    */
  /* save activation (only if needed...) */
  if (proc_table[p].status != SLEEP) {
    if (lev->nact[p] != -1)
      lev->nact[p]++;
    return;
  }

  #ifdef POSIXSTAR_DEBUG 
    kern_printf("(PS:Act:%d)",p);
  #endif
  
  POSIXSTAR_internal_activate(lev, p);
  POSIXSTAR_private_scheduler(lev);

}


static void POSIXSTAR_public_unblock(LEVEL l, PID p)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  /* Similar to POSIX_task_activate, but we don't check in what state
     the task is */

  #ifdef POSIXSTAR_DEBUG
    kern_printf("(PS:UnBlk:%d)",p);
  #endif
  /* Insert task in the coPOSIXect position */
  proc_table[p].status = POSIXSTAR_READY;
  iq_insertlast(p,&lev->ready[lev->priority[p]]);
  POSIXSTAR_private_scheduler(lev);

}

static void POSIXSTAR_public_block(LEVEL l, PID p)
{  
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  /* Extract the running task from the level
     . we have already extract it from the ready queue at the dispatch time.
     . the capacity event have to be removed by the generic kernel
     . the wcet don't need modification...
     . the state of the task is set by the calling function

     So, we do nothing!!!
  */

  #ifdef POSIXSTAR_DEBUG
    kern_printf("(PS:Blk:%d)", p);	
  #endif
 
  iq_extract(p,&lev->ready[lev->priority[p]]);
  POSIXSTAR_private_scheduler(lev);
  
}

static int POSIXSTAR_public_message(LEVEL l, PID p, void *m)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  STD_command_message *msg;
  NRT_TASK_MODEL *nrt;

  #ifdef POSIXSTAR_DEBUG
    kern_printf("(PS:Msg:%d)",p);
  #endif

  switch ((long)(m)) {

    /* Task EndCycle */
    case (long)(NULL):

      if (POSIXSTAR_private_change_level(l,p)) return 0;

      if (lev->nact[p] > 0) {
        /* continue!!!! */
        lev->nact[p]--;
        iq_extract(p,&lev->ready[lev->priority[p]]);
        iq_insertfirst(p,&lev->ready[lev->priority[p]]);
        proc_table[p].status = POSIXSTAR_READY;
      }
      else {
        proc_table[p].status = SLEEP;
        iq_extract(p,&lev->ready[lev->priority[p]]);
      }

      jet_update_endcycle(); /* Update the Jet data... */
      POSIXSTAR_private_scheduler(lev);

      break;

    /* Task Disable */
    case (long)(1):

      break; 

    default:

      msg = (STD_command_message *)m;

      switch(msg->command) {
        case STD_SET_NEW_LEVEL:
         
          lev->flag[p] |= POSIXSTAR_CHANGE_LEVEL;
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
            lev->new_control[p] &= ~CONTROL_CAP;
                                                                                                                             
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

         proc_table[p].control |= lev->new_control[p];

         POSIXSTAR_public_activate(l,p,NULL);

         break;
         
      }

      break;
         
  }

  return 0;

}

static void POSIXSTAR_public_end(LEVEL l, PID p)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  #ifdef POSIXSTAR_DEBUG
    kern_printf("(PS:End:%d)", p);
  #endif

  lev->nact[p] = -1;
  /* extract task from the queue */
  iq_extract(p,&lev->ready[lev->priority[p]]);

  /* then, we insert the task in the free queue */
  proc_table[p].status = FREE;
  iq_priority_insert(p,NULL);
  POSIXSTAR_private_scheduler(lev);

}

/* Registration functions */

/*+ Registration function:
    TIME slice                the slice for the Round Robin queue +*/
LEVEL POSIXSTAR_register_level(int master, TIME slice,
                       int prioritylevels)
{
  LEVEL l;            /* the level that we register */
  POSIXSTAR_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */
  int x;              /* a counter */

  #ifdef POSIXSTRA_DEBUG
    kern_printf("POSIXSTAR_register_level\n");
  #endif
 
  l = level_alloc_descriptor(sizeof(POSIXSTAR_level_des));

  lev = (POSIXSTAR_level_des *)level_table[l];

  lev->l.public_guarantee = NULL;
  lev->l.public_create    = POSIXSTAR_public_create;
  lev->l.public_end       = POSIXSTAR_public_end;
  lev->l.public_dispatch  = POSIXSTAR_public_dispatch;
  lev->l.public_epilogue  = POSIXSTAR_public_epilogue;
  lev->l.public_activate  = POSIXSTAR_public_activate;
  lev->l.public_unblock   = POSIXSTAR_public_unblock;
  lev->l.public_block     = POSIXSTAR_public_block;
  lev->l.public_message   = POSIXSTAR_public_message;
  lev->l.public_eligible  = POSIXSTAR_public_eligible;

  /* fill the POSIX descriptor part */
  for (i = 0; i < MAX_PROC; i++) {
    lev->nact[i] = -1;
    lev->budget[i] = -1;

    lev->flag[i] = 0;
 
    lev->new_level[i] = 0;
    lev->new_slice[i] = 0;
    lev->new_control[i] = 0;
  }

  lev->maxpriority = prioritylevels - 1;

  lev->ready = (IQUEUE *)kern_alloc(sizeof(IQUEUE) * prioritylevels);

  for (x = 0; x < prioritylevels; x++)
    iq_init(&lev->ready[x], NULL, 0);

  if (slice < POSIXSTAR_MINIMUM_SLICE) slice = POSIXSTAR_MINIMUM_SLICE;
  if (slice > POSIXSTAR_MAXIMUM_SLICE) slice = POSIXSTAR_MAXIMUM_SLICE;
  lev->slice = slice;
  lev->activated = NIL;
  lev->scheduling_level = master;
  lev->cap_lev = NIL;
  lev->yielding = 0;
  NULL_TIMESPEC(&lev->cap_lasttime);

  return l;

}

int POSIXSTAR_setbudget(LEVEL l, PID p, int budget)
{

  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  
  lev->budget[p] = budget;

  return 0;

}

int POSIXSTAR_getbudget(LEVEL l, PID p)
{

  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  return lev->budget[p];

}

int POSIXSTAR_budget_has_thread(LEVEL l, int budget)
{

  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  int i;

  for(i = 0; i< MAX_PROC; i++)
    if (lev->budget[i] == budget) return 1;

  return 0;

}

/*+ this function forces the running task to go to his queue tail;
    (it works only on the POSIX level) +*/
int POSIXSTAR_sched_yield(LEVEL l)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  if (proc_table[exec_shadow].task_level != l)
    return -1;

  proc_table[exec_shadow].context = kern_context_save();
  lev->yielding = 1;
  scheduler();
  kern_context_load(proc_table[exec_shadow].context);
  return 0;
}

/*+ this function returns the maximum level allowed for the POSIX level +*/
int POSIXSTAR_get_priority_max(LEVEL l)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  return lev->maxpriority;
}

/*+ this function returns the default timeslice for the POSIX level +*/
int POSIXSTAR_rr_get_interval(LEVEL l)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
  return lev->slice;
}

/*+ this functions returns some paramaters of a task;
    policy must be NRT_RR_POLICY or NRT_FIFO_POLICY;
    priority must be in the range [0..prioritylevels]
    returns ENOSYS or ESRCH if there are problems +*/
int POSIXSTAR_getschedparam(LEVEL l, PID p, int *policy, int *priority)
{
  if (p<0 || p>= MAX_PROC || proc_table[p].status == FREE)
    return ESRCH;

  if (proc_table[p].task_level != l)
    return ENOSYS;

  *policy = NRT_RR_POLICY;
  *priority = ((POSIXSTAR_level_des *)(level_table[l]))->priority[p];

  return 0;
}

/*+ this functions sets paramaters of a task +*/
int POSIXSTAR_setschedparam(LEVEL l, PID p, int policy, int priority)
{
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  if (p<0 || p>= MAX_PROC || proc_table[p].status == FREE)
    return ESRCH;

  if (proc_table[p].task_level != l)
    return ENOSYS;
  if (lev->priority[p] != priority) {
    if (proc_table[p].status == POSIXSTAR_READY) {
      iq_extract(p,&lev->ready[lev->priority[p]]);
      lev->priority[p] = priority;
      iq_insertlast(p,&lev->ready[priority]);
    }
    else
      lev->priority[p] = priority;
  }

  return 0;
}


void POSIXSTAR_set_nopreemtive_current(LEVEL l) {
  
  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);
 
  lev->flag[lev->activated]|=POSIXSTAR_FLAG_NOPREEMPT;
}

void POSIXSTAR_unset_nopreemtive_current(LEVEL l) {

  POSIXSTAR_level_des *lev = (POSIXSTAR_level_des *)(level_table[l]);

  lev->flag[lev->activated]&=~POSIXSTAR_FLAG_NOPREEMPT;
}
