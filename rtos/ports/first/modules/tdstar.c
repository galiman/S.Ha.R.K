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

/*
 * Copyright (C) 2001 Paolo Gai
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
#include "fsf.h"
#include "tdstar.h"
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/* for iqueues */
/* #include "iqueue.h" Now iqueues are the only queue type available
   into the kernel */
#include <kernel/iqueue.h>

/* for BUDGET_TASK_MODEL */
#include "fsf_configuration_parameters.h"
#include "fsf_core.h"
#include "fsf_server.h"
#include <posix/posix/comm_message.h>

/*
 * DEBUG stuffs begin
 */

//#define TDSTAR_DEBUG

#ifdef TDSTAR_DEBUG

static __inline__ fake_printf(char *fmt, ...) {}

//#define tdstar_printf fake_printf
//#define tdstar_printf2 fake_printf
//#define tdstar_printf3 fake_printf

#define tdstar_printf kern_printf
#define tdstar_printf2 kern_printf
#define tdstar_printf3 kern_printf
#endif

/*
 * DEBUG stuffs end
 */

/* Status used in the level */
#define TDSTAR_READY         MODULE_STATUS_BASE    /* - Ready status        */
#define TDSTAR_IDLE          MODULE_STATUS_BASE+4  /* to wait the deadline  */

/* flags */
#define TDSTAR_CHANGE_LEVEL     8 
#define TDSTAR_FLAG_NOPREEMPT   4 
#define TDSTAR_FLAG_NORAISEEXC  2
#define TDSTAR_FLAG_SPORADIC    1


typedef struct offline_element {
  PID                    pid;
  struct timespec        start;
  struct timespec        end;
  struct timespec        comp_time;
  struct offline_element *next;
} offline_element;


/* the level redefinition for the Earliest Deadline First level */
typedef struct {
  level_des l;     /* the standard level descriptor          */

  TIME period[MAX_PROC]; /* The task periods; the deadlines are
                       stored in the priority field           */
  int deadline_timer[MAX_PROC];
                   /* The task deadline timers               */

  struct timespec deadline_timespec[MAX_PROC];

  int dline_miss[MAX_PROC]; /* Deadline miss counter */
  int wcet_miss[MAX_PROC];  /* Wcet miss counter */

  int nact[MAX_PROC];       /* Wcet miss counter */

  int flag[MAX_PROC];
                   /* used to manage the JOB_TASK_MODEL and the
                       periodicity                            */

  IQUEUE ready;     /* the ready queue                        */

  PID activated;   /* the task that has been inserted into the
         	       master module */

  int budget[MAX_PROC];

  int scheduling_level;

  int cap_lev;
  struct timespec cap_lasttime;
  struct offline_element *td_table_act_current;
  struct offline_element *td_table_act_head;
  struct offline_element *td_table_act_free;
  struct offline_element tdtable[FSF_MAX_N_TARGET_WINDOWS];

  int new_level[MAX_PROC];
  int wcet[MAX_PROC]; /* save the wcet fields */
  
  struct timespec zero_time;

} TDSTAR_level_des;

static void capacity_handler(void *l)
{
  TDSTAR_level_des *lev = l;
  lev->cap_lev = NIL;
  event_need_reschedule();
}

static void TDSTAR_check_preemption(TDSTAR_level_des *lev)
{
  PID first;

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:chk)");
  #endif
  /* check if the task is preempteble or not */
  if (lev->activated != NIL && lev->flag[lev->activated] & TDSTAR_FLAG_NOPREEMPT) return;

  if ((first = iq_query_first(&lev->ready)) != lev->activated) {
    if (lev->activated != NIL)
      level_table[ lev->scheduling_level ]->
	private_extract(lev->scheduling_level, lev->activated);

    lev->activated = first;

    if (first != NIL) {
      BUDGET_TASK_MODEL b;
      budget_task_default_model(b, lev->budget[first]);

      level_table[ lev->scheduling_level ]->
	private_insert(lev->scheduling_level, first, (TASK_MODEL *)&b);
    }
  }
}

static void TDSTAR_timer_deadline(void *par);

static void TDSTAR_internal_activate(TDSTAR_level_des *lev, PID p, 
				      struct timespec *t)
{
  TIME tx;
 
  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:iact:%ds %dus )", t->tv_sec, t->tv_nsec/1000);
  #endif

  tx = TIMESPEC2USEC(&lev->td_table_act_current->end);

  ADDUSEC2TIMESPEC(tx, t);

  *iq_query_timespec(p, &lev->ready) = *t;
  lev->deadline_timespec[p] = *t;

  /* Insert task in the correct position */
  proc_table[p].status = TDSTAR_READY;
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].control &= ~CONTROL_CAP;

  /* check for preemption */
  TDSTAR_check_preemption(lev);
}

static void TDSTAR_timer_deadline(void *par)
{
  PID p = (PID) par;
  TDSTAR_level_des *lev;

  lev = (TDSTAR_level_des *)level_table[proc_table[p].task_level];
  lev->deadline_timer[p] = NIL;   

  switch (proc_table[p].status) {
    case TDSTAR_IDLE:
      /* set the request time */
      if (!(lev->flag[p] & TDSTAR_FLAG_SPORADIC))
        TDSTAR_internal_activate(lev,p,iq_query_timespec(p, &lev->ready));

      event_need_reschedule();
      break;

    default:
      #ifdef TDSTAR_DEBUG
        kern_printf("(TD:Dl:%d)",p);
      #endif
      /* else, a deadline miss occurred!!! */
      lev->dline_miss[p]++;
      TRACER_LOGEVENT(FTrace_EVT_task_deadline_miss,proc_table[p].context,proc_table[p].task_level);

      /* the task is into another state */
      if (!(lev->flag[p] & TDSTAR_FLAG_SPORADIC)) {
        lev->nact[p]++;
        ADDUSEC2TIMESPEC(lev->period[p], &lev->deadline_timespec[p]);
      }
  }

  /* Set the deadline timer */
  if (!(lev->flag[p] & TDSTAR_FLAG_SPORADIC))
    lev->deadline_timer[p] = kern_event_post(&lev->deadline_timespec[p],
                                             TDSTAR_timer_deadline,
                                             (void *)p);

}

static int TDSTAR_private_change_level(LEVEL l, PID p)
{

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  /* Change task level */
  if (lev->flag[p] & TDSTAR_CHANGE_LEVEL) {
    
#ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:clev)");
#endif
  
    STD_command_message msg;
    
    proc_table[p].status = SLEEP;
    lev->flag[p] &= ~ TDSTAR_CHANGE_LEVEL;
    
    level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);
    iq_extract(p,&lev->ready);
    
    if (lev->deadline_timer[p] != -1)
      kern_event_delete(lev->deadline_timer[p]);
    
    TDSTAR_check_preemption(lev);
    
    lev->nact[p] = 0;
    lev->budget[p] = -1;
    proc_table[p].task_level = lev->new_level[p];
    
    /* Send change level command to local scheduler */

    msg.command = STD_ACTIVATE_TASK;
    msg.param = NULL;

    level_table[ lev->new_level[p] ]->public_message(lev->new_level[p],p,&msg);
   
    return 1;

  }

  return 0;

}


static void TDSTAR_timer_guest_deadline(void *par)
{
  PID p = (PID) par;

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:gdl)");
  #endif

  kern_raise(XDEADLINE_MISS,p);
}

static int TDSTAR_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  /* if the TDSTAR_task_create is called, then the pclass must be a
     valid pclass. */
  HARD_TASK_MODEL *h;

  if (m->pclass != HARD_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  h = (HARD_TASK_MODEL *)m;
  if (h->wcet || h->mit) return -1;
  /* now we know that m is a valid model */

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:Crt)");
  #endif

  lev->period[p] = 0;

  lev->flag[p] = 0;

  if (h->periodicity == APERIODIC)
       lev->flag[p] |= TDSTAR_FLAG_SPORADIC;
  else return 0;

  lev->deadline_timer[p] = -1;
  lev->dline_miss[p]     = 0;
  lev->wcet_miss[p]      = 0;
  lev->nact[p]           = 0;

  /* Enable wcet check */
  proc_table[p].avail_time = h->wcet;
  proc_table[p].wcet       = h->wcet;
  proc_table[p].status     = SLEEP;

  return 0; /* OK, also if the task cannot be guaranteed... */
}


static void TDSTAR_account_capacity(TDSTAR_level_des *lev, PID p)
{
  struct timespec ty;
  TIME tx;

  SUBTIMESPEC(&schedule_time, &lev->cap_lasttime, &ty);
  tx = TIMESPEC2USEC(&ty);

  proc_table[p].avail_time -= tx;
}

static int TDSTAR_public_eligible(LEVEL l, PID p)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  #ifdef TDSTAR_DEBUG
    tdstar_printf2("(TD:eli:%d)",p);
  #endif

  return level_table[ lev->scheduling_level ]->
    private_eligible(lev->scheduling_level,p);

}

static void TDSTAR_public_dispatch(LEVEL l, PID p, int nostop)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  struct timespec ty;

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:dis)");
  #endif
  
  if (!nostop || proc_table[exec].task_level==l) {
      TIMESPEC_ASSIGN(&ty, &schedule_time);
      TIMESPEC_ASSIGN(&lev->cap_lasttime, &schedule_time);

      /* ...and finally, we have to post a capacity event on exec task because the shadow_task consume
       *      *        capacity on exe task always */
      if (proc_table[exec].avail_time > 0) {
        ADDUSEC2TIMESPEC(proc_table[exec].avail_time ,&ty);
        lev->cap_lev = kern_event_post(&ty,capacity_handler, lev);
      }
      level_table[lev->scheduling_level]->private_dispatch(lev->scheduling_level, p, nostop);
  }
  else
      level_table[proc_table[exec].task_level]->public_dispatch(proc_table[exec].task_level, p, nostop);

}

static void TDSTAR_public_epilogue(LEVEL l, PID p)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:epi ");
  #endif

  if (lev->cap_lev!=NIL) {
       kern_event_delete(lev->cap_lev);
       lev->cap_lev=NIL;
  }

 
  if ( proc_table[exec].task_level==l ) {
    
     if (proc_table[exec].avail_time > 0) TDSTAR_account_capacity(lev,exec);

     if (TDSTAR_private_change_level(l, p)) return;

     /* check if the wcet is finished... */
     if (proc_table[exec].avail_time < 0) {
        /* wcet finished: disable wcet event and count wcet miss */

       #ifdef TDSTAR_DEBUG
         tdstar_printf2("W%d",p);
       #endif
        //proc_table[p].control &= ~CONTROL_CAP;
        lev->wcet_miss[exec]++;
	proc_table[exec].avail_time = 0;
	TRACER_LOGEVENT(FTrace_EVT_task_wcet_violation,proc_table[exec].context,proc_table[exec].task_level);
     }

     #ifdef TDSTAR_DEBUG
       tdstar_printf(")");
     #endif

     level_table[ lev->scheduling_level ]->
       private_epilogue(lev->scheduling_level,p);

     proc_table[exec].status = TDSTAR_READY;
    } else
        level_table[proc_table[exec].task_level]->public_epilogue(proc_table[exec].task_level,p);

}

static void TDSTAR_public_activate(LEVEL l, PID p, struct timespec *o)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  struct timespec t;
  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:act:%d)",p);
  #endif

  /* Test if we are trying to activate a non sleeping task    */
  /* save activation (only if needed... */
  if (proc_table[p].status != SLEEP) {
    /* a periodic task cannot be activated when it is already active */
    /* but aperiodic task can be reactivate before */
    if (lev->flag[p] & TDSTAR_FLAG_SPORADIC) {
	if (proc_table[p].status != TDSTAR_IDLE) {
	  lev->nact[p]++;
	  kern_printf("err***");
	  return;
	}
    } else {
	kern_raise(XACTIVATION,p);
    }
  }

  /* Set the new wcet and avail time check */
  proc_table[p].avail_time = TIMESPEC2USEC(&(lev->td_table_act_current->comp_time));
  proc_table[p].wcet       = TIMESPEC2USEC(&(lev->td_table_act_current->comp_time));

  kern_gettime(&t);

  TDSTAR_internal_activate(lev,p, &t);

  /* Set the deadline timer */
  lev->deadline_timer[p] = kern_event_post(&lev->deadline_timespec[p],
                                           TDSTAR_timer_deadline,
                                           (void *)p);

}

static void TDSTAR_public_unblock(LEVEL l, PID p)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:ins)");
  #endif

  /* Insert task in the correct position */
  proc_table[p].status = TDSTAR_READY;
  iq_timespec_insert(p,&lev->ready);

  /* and check for preemption! */
  TDSTAR_check_preemption(lev);

}

static void TDSTAR_public_block(LEVEL l, PID p)
{

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  #ifdef TDSTAR_DEBUG
   tdstar_printf("(TD:ext)");
  #endif

  /* the task is blocked on a synchronization primitive. we have to
     remove it from the master module -and- from the local queue! */
  iq_extract(p,&lev->ready);

  /* and finally, a preemption check! (it will also call guest_end) */
  TDSTAR_check_preemption(lev);
}

static int TDSTAR_public_message(LEVEL l, PID p, void *m)
{

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  struct timespec temp;
  STD_command_message *msg;
  HARD_TASK_MODEL *h;

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:ecy ");
  #endif

  switch ((long)(m)) {

    /* Task EndCycle */
    case (long)(NULL):

      if (TDSTAR_private_change_level(l,p)) return 0;

       /* we call guest_end directly here because the same task may
         be reinserted in the queue before calling the preemption check! */
       level_table[ lev->scheduling_level ]->
         private_extract(lev->scheduling_level,p);
       lev->activated = NIL;

       iq_extract(p,&lev->ready);

       /* we reset the capacity counters... */
       proc_table[p].avail_time = proc_table[p].wcet;

       if (lev->nact[p] > 0) {
    
         #ifdef TDSTAR_DEBUG
           tdstar_printf2("E%d",p);
         #endif
      
         /* Pending activation: reactivate the thread!!! */
         lev->nact[p]--;
      
         /* see also TDSTAR_timer_deadline */
         kern_gettime(&temp);
      
         TDSTAR_internal_activate(lev,p, &temp);
      
         /* check if the deadline has already expired */
         temp = *iq_query_timespec(p, &lev->ready);
         if (TIMESPEC_A_LT_B(&temp, &schedule_time)) {
	   /* count the deadline miss */
           lev->dline_miss[p]++;
	   kern_event_delete(lev->deadline_timer[p]);
	   lev->deadline_timer[p] = NIL;
         }

       } else {

         #ifdef TDSTAR_DEBUG
           tdstar_printf("e%d",p);
         #endif
      
         /* the task has terminated his job before it consume the wcet. All OK! */
         if (lev->flag[p] & TDSTAR_FLAG_SPORADIC)
		proc_table[p].status = SLEEP;
	 else
		proc_table[p].status = TDSTAR_IDLE;
          
         if (lev->flag[p] & TDSTAR_FLAG_SPORADIC && lev->deadline_timer[p] != NIL) {
           kern_event_delete(lev->deadline_timer[p]); 
	   lev->deadline_timer[p] = NIL;
	 }
      
         /* and finally, a preemption check! */
         TDSTAR_check_preemption(lev);
      
         /* when the deadline timer fire, it recognize the situation and set
	   correctly all the stuffs (like reactivation, etc... ) */
       }

    #ifdef TDSTAR_DEBUG
      tdstar_printf(")");
    #endif

    TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,proc_table[p].context,proc_table[p].task_level);
    jet_update_endcycle(); /* Update the Jet data... */
    break;

  default:
    msg = (STD_command_message *)m;
  
#ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:MSG %d)",msg->command);
#endif   
 
    switch(msg->command) {
    case STD_SET_NEW_MODEL:
      /* if the TDSTAR_task_create is called, then the pclass must be a
	 valid pclass. */
      h=(HARD_TASK_MODEL *)(msg->param);
   
      /* now we know that m is a valid model */
      lev->wcet[p] = h->wcet;
      lev->period[p] = h->mit;

#ifdef TDSTAR_DEBUG      
      kern_printf("(TD:NM p%d w%d m%d)", p, h->wcet, h->mit);
#endif       
      lev->flag[p] = 0;
      lev->deadline_timer[p] = -1;
      lev->dline_miss[p]     = 0;
      lev->wcet_miss[p]      = 0;
      lev->nact[p]           = 0;

      break;

    case STD_SET_NEW_LEVEL:
      
      lev->flag[p] |= TDSTAR_CHANGE_LEVEL;
      lev->new_level[p] = (int)(msg->param);

      break;

    case STD_ACTIVATE_TASK:
#ifdef TDSTAR_DEBUG
      kern_printf("(TD:SA)");
#endif       
      /* Enable wcet check */
      proc_table[p].avail_time = lev->wcet[p];
      proc_table[p].wcet       = lev->wcet[p];
      proc_table[p].control &= ~CONTROL_CAP;
      
      TDSTAR_public_activate(l, p,NULL);
      
      break;

      
    }

    break;
      
  }
  return 0;
}

static void TDSTAR_public_end(LEVEL l, PID p)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  #ifdef TDSTAR_DEBUG
    tdstar_printf("(TD:end)");
  #endif

  iq_extract(p,&lev->ready);

  /* we finally put the task in the ready queue */
  proc_table[p].status = FREE;
  
  iq_insertfirst(p,&freedesc);
  lev->activated=NIL;
  
  if (lev->deadline_timer[p] != -1) {
    kern_event_delete(lev->deadline_timer[p]);
    lev->deadline_timer[p] = NIL;
  }

  /* and finally, a preemption check! (it will also call guest_end) */
  TDSTAR_check_preemption(lev);
}

/* Guest Functions
   These functions manages a JOB_TASK_MODEL, that is used to put
   a guest task in the TDSTAR ready queue. */

static void TDSTAR_private_insert(LEVEL l, PID p, TASK_MODEL *m)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  JOB_TASK_MODEL *job;

  if (m->pclass != JOB_PCLASS || (m->level != 0 && m->level != l) ) {
    kern_raise(XINVALID_TASK, p);
    return;
  }

  job = (JOB_TASK_MODEL *)m;

  /* if the TDSTAR_guest_create is called, then the pclass must be a
     valid pclass. */

  *iq_query_timespec(p, &lev->ready) = job->deadline;
  
  lev->deadline_timer[p] = -1;
  lev->dline_miss[p]     = 0;
  lev->wcet_miss[p]      = 0;
  lev->nact[p]           = 0;

  if (job->noraiseexc)
    lev->flag[p] |= TDSTAR_FLAG_NORAISEEXC;
  else {
    lev->flag[p] &= ~TDSTAR_FLAG_NORAISEEXC;
    lev->deadline_timer[p] = kern_event_post(iq_query_timespec(p, &lev->ready),
                                             TDSTAR_timer_guest_deadline,
                                             (void *)p);
  }

  lev->period[p] = job->period;

  /* Insert task in the correct position */
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].status = TDSTAR_READY;

  /* check for preemption */
  TDSTAR_check_preemption(lev);

  /* there is no bandwidth guarantee at this level, it is performed
     by the level that inserts guest tasks... */
}

static void TDSTAR_private_dispatch(LEVEL l, PID p, int nostop)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  level_table[ lev->scheduling_level ]->
    private_dispatch(lev->scheduling_level,p,nostop);
}

static void TDSTAR_private_epilogue(LEVEL l, PID p)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  /* the task has been preempted. it returns into the ready queue... */
  level_table[ lev->scheduling_level ]->
    private_epilogue(lev->scheduling_level,p);

  proc_table[p].status = TDSTAR_READY;
}

static void TDSTAR_private_extract(LEVEL l, PID p)
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

#ifdef TDSTAR_DEBUG
  kern_printf("TDSTAR_guest_end: dline timer %d\n",lev->deadline_timer[p]);
#endif

  iq_extract(p, &lev->ready);

  /* we remove the deadline timer, because the slice is finished */
  if (lev->deadline_timer[p] != NIL) {
#ifdef TDSTAR_DEBUG
    kern_printf("TDSTAR_guest_end: dline timer %d\n",lev->deadline_timer[p]);
#endif
    kern_event_delete(lev->deadline_timer[p]);
    lev->deadline_timer[p] = NIL;
  }

  /* and finally, a preemption check! (it will also call guest_end() */
  TDSTAR_check_preemption(lev);
}

/* Registration functions */

/* Registration function:
    int flags                 the init flags ... see TDSTAR.h */

LEVEL TDSTAR_register_level(int master)
{
  LEVEL l;            /* the level that we register */
  TDSTAR_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

#ifdef TDSTAR_DEBUG
  printk("TDSTAR_register_level\n");
#endif

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(TDSTAR_level_des));

  lev = (TDSTAR_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.private_insert   = TDSTAR_private_insert;
  lev->l.private_extract  = TDSTAR_private_extract;
  lev->l.private_dispatch = TDSTAR_private_dispatch;
  lev->l.private_epilogue = TDSTAR_private_epilogue;

  lev->l.public_guarantee = NULL;
  lev->l.public_eligible  = TDSTAR_public_eligible;
  lev->l.public_create    = TDSTAR_public_create;
  lev->l.public_end       = TDSTAR_public_end;
  lev->l.public_dispatch  = TDSTAR_public_dispatch;
  lev->l.public_epilogue  = TDSTAR_public_epilogue;
  lev->l.public_activate  = TDSTAR_public_activate;
  lev->l.public_unblock   = TDSTAR_public_unblock;
  lev->l.public_block     = TDSTAR_public_block;
  lev->l.public_message   = TDSTAR_public_message;

  /* fill the TDSTAR descriptor part */
  for(i=0; i<MAX_PROC; i++) {
    lev->period[i]         = 0;
    lev->deadline_timer[i] = -1;
    lev->flag[i]           = 0;
    lev->dline_miss[i]     = 0;
    lev->wcet_miss[i]      = 0;
    lev->nact[i]           = 0;
    lev->budget[i]         = NIL;
    lev->new_level[i]      = -1;
  }
  /// struct offline_table tdtable[FSF_MAX_N_TARGET_WINDOWS];
  for (i=0; i<FSF_MAX_N_TARGET_WINDOWS; i++) {
	 lev->tdtable[i].pid=NIL;
	 lev->tdtable[i].next=&lev->tdtable[(i+1)%FSF_MAX_N_TARGET_WINDOWS];
	 NULL_TIMESPEC(&lev->tdtable[i].comp_time);
	 NULL_TIMESPEC(&lev->tdtable[i].end);
	 NULL_TIMESPEC(&lev->tdtable[i].start);
  }
  lev->tdtable[FSF_MAX_N_TARGET_WINDOWS-1].next=NULL;
  lev->td_table_act_current=NULL;
  lev->td_table_act_head=NULL;
  lev->td_table_act_free=&lev->tdtable[0];

  iq_init(&lev->ready, NULL, IQUEUE_NO_PRIORITY);
  lev->activated = NIL;
  

  lev->scheduling_level = master;
  lev->cap_lev = NIL;
  NULL_TIMESPEC(&lev->cap_lasttime);
  NULL_TIMESPEC(&lev->zero_time);

  return l;
}

int TDSTAR_get_dline_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  
  return lev->dline_miss[p];
}

int TDSTAR_get_wcet_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  return lev->wcet_miss[p];
}

int TDSTAR_get_nact(PID p)
{
  LEVEL l = proc_table[p].task_level;
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  return lev->nact[p];
}

int TDSTAR_reset_dline_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  lev->dline_miss[p] = 0;
  return 0;
}

int TDSTAR_reset_wcet_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  lev->wcet_miss[p] = 0;
  return 0;
}

int TDSTAR_setbudget(LEVEL l, PID p, int budget)
{

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  
  lev->budget[p] = budget;

  return 0;

}

int TDSTAR_getbudget(LEVEL l, PID p)
{

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  return lev->budget[p];

}

void TDSTAR_set_nopreemtive_current(LEVEL l) {
  
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
 
  lev->flag[lev->activated]|=TDSTAR_FLAG_NOPREEMPT;
}

void TDSTAR_unset_nopreemtive_current(LEVEL l) {

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);

  lev->flag[lev->activated]&=~TDSTAR_FLAG_NOPREEMPT;
}
 
int TDSTAR_budget_has_thread(LEVEL l, int budget)
{

  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  int i;

  for(i = 0; i< MAX_PROC; i++)
    if (lev->budget[i] == budget) return 1;

  return 0;

}

/* Activate task and post the new activation event */
void tdstar_task_activate(TDSTAR_level_des *lev) { 
  
  struct timespec end_time;

#ifdef TDSTAR_DEBUG
  kern_printf("Pid %d",lev->td_table_act_current->pid);
#endif

  task_activate(lev->td_table_act_current->pid);
  lev->td_table_act_current=lev->td_table_act_current->next;

  if (lev->td_table_act_current!=NULL) {

    ADDTIMESPEC(&lev->zero_time, &lev->td_table_act_current->start, &end_time);
 #ifdef TDSTAR_DEBUG
    tdstar_printf("(NEXT:iact:%ds %dus )", end_time.tv_sec, end_time.tv_nsec/1000);
  #endif
    kern_event_post(&end_time,(void *)((void *)(tdstar_task_activate)),lev);

  } else exit(0);

}

void TDSTAR_start_simulation(LEVEL l) 
{
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  struct timespec end_time;
  //kern_printf("(Start simulation)");
  kern_gettime(&lev->zero_time);
#ifdef TDSTAR_DEBUG
    tdstar_printf("(START:iact:%ds %dus )", lev->zero_time.tv_sec, lev->zero_time.tv_nsec/1000);
#endif
  if (lev->td_table_act_head != NULL) {
    lev->td_table_act_current=lev->td_table_act_head;
    ADDTIMESPEC(&lev->zero_time, &lev->td_table_act_head->start, &end_time);
  #ifdef TDSTAR_DEBUG
    tdstar_printf("(NEXT:iact:%ds %dus )", end_time.tv_sec, end_time.tv_nsec/1000);
  #endif

    kern_event_post(&end_time,(void *)((void *)(tdstar_task_activate)),lev);
  }
  

}
///typedef struct {
///  int size;
///  struct fsf_target_window table[FSF_MAX_N_TARGET_WINDOWS];
///} fsf_table_driven_params_t;

void TDSTAR_settable(LEVEL l, fsf_table_driven_params_t *param, PID pd) {
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  int i;
  struct offline_element *elem,*p,*newelem;
  //kern_printf("Set table dim=%d", param->size);

  for (i=0; i<param->size; i++) {
    	elem = NULL;
	//kern_printf("(E=%d)", i);
	newelem=lev->td_table_act_free;
	newelem->pid=pd;
	TIMESPEC_ASSIGN(&newelem->start,&param->table[i].start);
	TIMESPEC_ASSIGN(&newelem->end, &param->table[i].end);
	TIMESPEC_ASSIGN(&newelem->comp_time, &param->table[i].comp_time);

	lev->td_table_act_free=lev->td_table_act_free->next;
		
	for (p = lev->td_table_act_head; p; p = elem->next) {	        
		if (TIMESPEC_A_GT_B(&(param->table[i].start), &p->start)) {
		  elem=p;
		}		
		else
		  break;
	}
	if (elem) {
	  //kern_printf("(New elem)");
	  elem->next = newelem;
	} else {
	  //kern_printf("(Head)");
	  lev->td_table_act_head = newelem;
	}
	newelem->next = p;
    
  }
}

void TDSTAR_debugtable(LEVEL l) {
  TDSTAR_level_des *lev = (TDSTAR_level_des *)(level_table[l]);
  struct offline_element *p;
  int i;

  for (p = lev->td_table_act_head, i=0; p; p = p->next, i++) {

    kern_printf("(ev %d, start=%ds %dus)", i, p->start.tv_sec, (p->start.tv_nsec)/1000);
  }
    
}
