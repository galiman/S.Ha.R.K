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

/*
 * Copyright (C) 2002 Paolo Gai
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

#include "cbsstar.h"
#include "ll/i386/64bit.h"
#include <tracer.h>

/*
 * DEBUG stuffs begin
 */
//#define CBSSTAR_DEBUG
#ifdef CBSSTAR_DEBUG

static __inline__ void fake_printf(char *fmt, ...) {}

#define cbsstar_printf kern_printf
#define cbsstar_printf2 kern_printf
#define cbsstar_printf3 kern_printf

//#define cbsstar_printf fake_printf
//#define cbsstar_printf2 fake_printf
//#define cbsstar_printf3 fake_printf

#endif
/*
 * DEBUG stuffs end
 */

/* this structure contains the status for a single budget */
struct budget_struct {
  TIME Q;                 /* budget */
  TIME T;                 /* period */
  TIME D;                 /* deadline */

  struct timespec dline;  /* deadline */
  struct timespec replenish; /* server period */

  int replenish_timer;        /* oslib event for budget reactivation*/
  int avail;              /* current budget */
  
  LEVEL l;                /* Current CBSSTAR level */
  int loc_sched_id;       /* Local scheduler id */
  LEVEL loc_sched_level;  /* Local scheduler level */
  
  PID current;            /* the task currently put in execution */
  int flags;

  IQUEUE tasks;           /* a FIFO queue for the tasks handled
                             using the budget */

};

#define CBSSTAR_NOACTIVE   0
#define CBSSTAR_ACTIVE     1

typedef struct {
  level_des l;               /* the standard level descriptor */

  struct budget_struct *b;   /* the budgets! */
  int n;                     /* the maximum index for the budgets */
  int freebudgets;           /* number of free budgets; starts from n */

  int tb[MAX_PROC];          /* link task->budget (used in guest_end) */

  bandwidth_t U;             /*+ the used bandwidth by the server       +*/

  int cap_lev;
  struct timespec cap_lasttime;

  LEVEL scheduling_level;

} CBSSTAR_level_des;


static void CBSSTAR_replenish_timer_hardreservation(void *a)
{
  struct budget_struct *b = a;
  PID p;
 
  #ifdef CBSSTAR_DEBUG
    cbsstar_printf("(CS:HrdRes:");  
  #endif

  b->replenish_timer = NIL;

  /* we modify the deadline according to rule 4 ... */
  /* there is a while because if the wcet is << than the system tick
     we need to postpone the deadline many times */ 
  
  b->avail += b->Q;
  if (b->avail > b->Q) b->avail = b->Q;
  
  if (b->avail > 0) b->flags = CBSSTAR_ACTIVE;

 /* Tracer */
  TRACER_LOGEVENT(FTrace_EVT_server_replenish,0,(unsigned int)(b));

  /* avail may be <0 because a task executed via a shadow fo many time
     b->current == NIL only if the prec task was finished and there
     was not any other task to be put in the ready queue
     ... we are now activating the next task */
  if (b->current == NIL && b->flags) {
      if (iq_query_first(&(b->tasks)) != NIL) {
        CBSSTAR_level_des *lev;
	JOB_TASK_MODEL job;
 
        p = iq_getfirst(&b->tasks);
 
        #ifdef CBSSTAR_DEBUG
          cbsstar_printf("%d",p);
        #endif

        kern_gettime(&b->replenish);
        TIMESPEC_ASSIGN(&b->dline, &b->replenish);
        ADDUSEC2TIMESPEC(b->D, &b->dline);
        ADDUSEC2TIMESPEC(b->T, &b->replenish);

        b->current = p;

        lev = (CBSSTAR_level_des *)(level_table[b->l]);
	
        job_task_default_model(job, b->dline);
	job_task_def_noexc(job);
	level_table[ lev->scheduling_level ]->
	  private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);
       
        event_need_reschedule();

    }
  }
  
  if (b->flags == CBSSTAR_NOACTIVE) {
    
    kern_gettime(&b->replenish);
    ADDUSEC2TIMESPEC(b->T, &b->replenish);;
  
    b->replenish_timer=kern_event_post(&b->replenish, CBSSTAR_replenish_timer_hardreservation, b);
  }

  #ifdef CBSSTAR_DEBUG
    cbsstar_printf(")");
  #endif

}

static void CBSSTAR_activation(CBSSTAR_level_des *lev,
                           PID p,
                           struct timespec *acttime)
{
  JOB_TASK_MODEL job;
  struct budget_struct *b = &lev->b[lev->tb[p]];
  /* we have to check if the deadline and the wcet are correct before
     activating a new task or an old task... */

  /* we have to check if the deadline and the wcet are correct before
   *      activating a new task or an old task... */

   /* check 1: if the deadline is before than the actual scheduling time */

   /* check 2: if ( avail_time >= (cbs_dline - acttime)* (wcet/period) )
    *      (rule 7 in the CBS article!) */
   TIME t;
   struct timespec t2,t3;

   mul32div32to32(b->D,b->avail,b->Q,t);
   
   t3.tv_sec = t / 1000000;
   t3.tv_nsec = (t % 1000000) * 1000;
   SUBTIMESPEC(&b->dline, acttime, &t2);

   TRACER_LOGEVENT(FTrace_EVT_server_active,0,(unsigned int)b);

   if (/* 1 */ TIMESPEC_A_LT_B(&b->dline, acttime) ||
       /* 2 */ TIMESPEC_A_GT_B(&t3, &t2) ) {
       TIMESPEC_ASSIGN(&b->replenish, acttime);
       ADDUSEC2TIMESPEC(b->T, &b->replenish);
       TIMESPEC_ASSIGN(&b->dline, acttime);
       ADDUSEC2TIMESPEC(b->D, &b->dline);
       b->avail=b->Q;
       b->flags=CBSSTAR_ACTIVE;
  }

  /* record the current task inserted in the master module */
  b->current = p;

  job_task_default_model(job, b->dline);
  job_task_def_noexc(job);
  level_table[ lev->scheduling_level ]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);

}

static void CBSSTAR_account_capacity(CBSSTAR_level_des *lev, PID p)
{
  struct timespec ty;
  TIME tx;
  struct budget_struct *b = &lev->b[lev->tb[p]];

  if (lev->cap_lev != NIL) { 
    kern_event_delete(lev->cap_lev); 
    lev->cap_lev = NIL;
  }

  SUBTIMESPEC(&schedule_time, &lev->cap_lasttime, &ty);
  tx = TIMESPEC2USEC(&ty);
  b->avail -= tx;

  #ifdef CBSSTAR_DEBUG
    kern_printf("(CS:Cap p%d av=%d tx=%d)", p, b->avail,tx);
  #endif

  if (b->avail <= 0) { 
    b->flags = CBSSTAR_NOACTIVE;
    TRACER_LOGEVENT(FTrace_EVT_server_exhaust,0,(unsigned int)(b));
  }

  if (TIMESPEC_A_LT_B(&b->dline, &schedule_time)) {
    /* we modify the deadline ... */
    TIMESPEC_ASSIGN(&b->dline, &schedule_time);
    ADDUSEC2TIMESPEC(b->D, &b->dline);
    TIMESPEC_ASSIGN(&b->replenish, &schedule_time);
    ADDUSEC2TIMESPEC(b->T, &b->replenish);
  }

  if (b->flags == CBSSTAR_NOACTIVE && b->replenish_timer == NIL)  {
    b->replenish_timer=kern_event_post(&b->replenish, CBSSTAR_replenish_timer_hardreservation, b);
  }
 
}


/* The on-line guarantee is enabled only if the appropriate flag is set... */
static int CBSSTAR_public_guarantee(LEVEL l, bandwidth_t *freebandwidth)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  #ifdef CBSSTAR_DEBUG
    cbsstar_printf("(CS:Gua)");
  #endif

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

static void capacity_handler(void *l)
{
  
  CBSSTAR_level_des *lev = l;
  lev->cap_lev = NIL;
  event_need_reschedule();
 
}

static int CBSSTAR_private_eligible(LEVEL l, PID p)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
  struct budget_struct *b = &lev->b[lev->tb[p]];
  JOB_TASK_MODEL job;

  /* we have to check if the deadline and the wcet are correct...
     if the CBSSTAR level schedules in background with respect to others
     levels, there can be the case in witch a task is scheduled by
     schedule_time > CBSSTAR_deadline; in this case (not covered in the
     article because if there is only the standard scheduling policy
     this never apply) we reassign the deadline */
  
    if ( TIMESPEC_A_LT_B(&b->dline, &schedule_time)) {
      if (lev->cap_lev!=NIL) {
        kern_event_delete(lev->cap_lev);
        lev->cap_lev=NIL; 
      } 
	    
      /* we kill the current activation */
      level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level, p);
      /* we modify the deadline ... */
      kern_gettime(&b->replenish);
      TIMESPEC_ASSIGN(&b->dline, &b->replenish);
      ADDUSEC2TIMESPEC(b->D, &b->dline);
      ADDUSEC2TIMESPEC(b->T, &b->replenish);      

      /* and the capacity */
      b->avail = b->Q;

      b->flags = CBSSTAR_ACTIVE;

      /* Tracer */
      TRACER_LOGEVENT(FTrace_EVT_server_replenish,0,(unsigned int)(b));

      if (b->replenish_timer!=NIL)  {
        kern_event_delete(b->replenish_timer);
	b->replenish_timer=NIL;
      }
    
      /* and, finally, we reinsert the task in the master level */
      job_task_default_model(job, b->dline);
      job_task_def_noexc(job);
      level_table[ lev->scheduling_level ]->
        private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);
    
      return -1;
    
    }  
 

  return 0;

}

static void CBSSTAR_private_insert(LEVEL l, PID p, TASK_MODEL *m)
{
  /* A task has been activated for some reason. Basically, the task is
  inserted in the queue if the queue is empty, otherwise the task is
  inserted into the master module, and an oslib event is posted. */

  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
  BUDGET_TASK_MODEL *budget;

  if (m->pclass != BUDGET_PCLASS ||
      (m->level != 0 && m->level != l)) {
    kern_raise(XINVALID_TASK, p);
    return;
  }
  budget = (BUDGET_TASK_MODEL *)m;

  #ifdef CBSSTAR_DEBUG
    cbsstar_printf("(CS:PriIns:%d:%d", p, budget->b);
  #endif
  
  if (budget->b == -1)
    return;

  lev->tb[p] = budget->b;

  if (lev->b[budget->b].current == NIL && lev->b[budget->b].flags ) {
    /* This is the first task in the budget,
       the task have to be inserted into the master module */
    struct timespec t;
    kern_gettime(&t);
    CBSSTAR_activation(lev,p,&t);
  } else {
    /* The budget is not empty, another task is already into the
       master module, so the task is inserted at the end of the budget
       queue */
    iq_insertlast(p,&lev->b[budget->b].tasks);
  }

  #ifdef CBSSTAR_DEBUG
    cbsstar_printf(")");
  #endif

}

static void CBSSTAR_private_extract(LEVEL l, PID p)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  #ifdef CBSSTAR_DEBUG
    kern_printf("(CS:Ext:%d)", p);
  #endif

  /* a task is removed from execution for some reasons. It must be
     that it is the first in its budget queue (only the first task in
     a budget queue is put into execution!) */

  /* remove the task from execution (or from the ready queue) */
  if (lev->b[lev->tb[p]].current == p) {

   CBSSTAR_account_capacity(lev,p);
    /* remove the task from the master module */
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level, p);

    /* check if the buffer has someone else to schedule */
    if (iq_query_first(&lev->b[lev->tb[p]].tasks) == NIL) {
      /* the buffer has no tasks! */
      lev->b[lev->tb[p]].current = NIL;
    }
    else if (lev->b[lev->tb[p]].flags) {
      /* if so, insert the new task into the master module */
      PID n;
      struct timespec t;
      
      kern_gettime(&t);
      n = iq_getfirst(&lev->b[lev->tb[p]].tasks);
      CBSSTAR_activation(lev,n,&t);  // it modifies b[lev->tb[p]].current
    }
    else
      lev->b[lev->tb[p]].current=NIL;

  }
  else  {
    iq_extract(p, &lev->b[lev->tb[p]].tasks);
  }
}

static void CBSSTAR_private_dispatch(LEVEL l, PID p, int nostop)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
  struct timespec ty;

  #ifdef CBSSTAR_DEBUG
    kern_printf("(CS:Dsp:%d)", p);
  #endif

  /* the current task (that is the only one inserted in the master module
     for the corresponding budget) is dispatched. Note that the current
     task is not inserted in any FIFO queue, so the task does not have to
     be extracted! */

  /* ... then, we dispatch it to the master level */
  if (!nostop)
    level_table[ lev->scheduling_level ]->
            private_dispatch(lev->scheduling_level,p,nostop);

  /* ...and finally, we have to post a capacity event */
  TIMESPEC_ASSIGN(&ty, &schedule_time);
  TIMESPEC_ASSIGN(&lev->cap_lasttime, &schedule_time);
  ADDUSEC2TIMESPEC(lev->b[lev->tb[exec]].avail,&ty);
  lev->cap_lev = kern_event_post(&ty,capacity_handler, lev);
    

  
}

static void CBSSTAR_private_epilogue(LEVEL l, PID p)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
  struct budget_struct *b;
  int skip_epilog;
  skip_epilog = 0;

  #ifdef CBSSTAR_DEBUG
    kern_printf("(CS:Epi:%d)",p);
  #endif

  if (p==exec) b = &lev->b[lev->tb[p]];
    else if (lev->tb[exec]!=NIL)  {
	  b = &lev->b[lev->tb[exec]];
	  p=exec;
	  skip_epilog=1;
       }
       else return; 	
   
  CBSSTAR_account_capacity(lev,p);

  // L'evento di capacità va cancellato perchè sarà ripristinato nella successiva dispatch
  /* we have to check if the capacity is still available */
  if (b->flags)  {
    /* there is capacity available, maybe it is simply a preemption;
       the task have to return to the ready queue */
    if (!skip_epilog)
      level_table[ lev->scheduling_level ]->
	private_epilogue(lev->scheduling_level,p);
    
  } else {
    /* we kill the current activation */
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level, p);    
    
    iq_insertfirst(p, &b->tasks);
    b->current = NIL;
    
  }
   
}

/* Registration functions }*/

/*+ Registration function:
    int flags                 the init flags ... see CBSSTAR.h +*/
LEVEL CBSSTAR_register_level(int n, LEVEL master)
{
  LEVEL l;            /* the level that we register */
  CBSSTAR_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

  printk("CBSSTAR_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(CBSSTAR_level_des));

  lev = (CBSSTAR_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.private_insert   = CBSSTAR_private_insert;
  lev->l.private_extract  = CBSSTAR_private_extract;
  lev->l.private_eligible = CBSSTAR_private_eligible;
  lev->l.private_dispatch = CBSSTAR_private_dispatch;
  lev->l.private_epilogue = CBSSTAR_private_epilogue;

  lev->l.public_guarantee = CBSSTAR_public_guarantee;

  /* fill the CBSSTAR descriptor part */
  lev->b = (struct budget_struct *)kern_alloc(sizeof(struct budget_struct)*n);

  for (i=0; i<n; i++) {
    lev->b[i].Q = 0;
    lev->b[i].T = 0;
    lev->b[i].D = 0;
    NULL_TIMESPEC(&lev->b[i].dline);
    NULL_TIMESPEC(&lev->b[i].replenish);
    lev->b[i].replenish_timer = NIL;
    lev->b[i].avail = 0;
    lev->b[i].current = -1;
    lev->b[i].flags = CBSSTAR_ACTIVE;
    lev->b[i].l=l;
    iq_init(&lev->b[i].tasks, /* &freedesc */NULL, 0);
  }

  lev->n = n;
  lev->freebudgets = 0;

  for (i=0; i<MAX_PROC; i++) 
    lev->tb[i] = NIL;

  lev->U = 0;
  lev->cap_lev = NIL;
  NULL_TIMESPEC(&lev->cap_lasttime);
  lev->scheduling_level = master;

  return l;

}

int CBSSTAR_setbudget(LEVEL l, TIME Q, TIME T, TIME D, LEVEL local_scheduler_level, int scheduler_id)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
  int r;

  #ifdef CBSSTAR_DEBUG
    cbsstar_printf("(CS:SetBud)");
  #endif

  for (r = 0; r < lev->n; r++)
    if (lev->b[r].Q == 0) break;

  if (r != lev->n) {
    bandwidth_t b;
    b = (MAX_BANDWIDTH / T) * Q;
    
    /* really update lev->U, checking an overflow... */
    if (Q< T /* && MAX_BANDWIDTH - lev->U > b */) {
      
      lev->U += b;
      lev->freebudgets++;
      
      lev->b[r].Q = Q;
      lev->b[r].T = T;
      lev->b[r].D = D;
      lev->b[r].avail = Q;
      lev->b[r].flags = CBSSTAR_ACTIVE;
      lev->b[r].loc_sched_id = scheduler_id;
      lev->b[r].loc_sched_level = local_scheduler_level;
      
      return r;
    }
    else
      return -2;
  }
  else
    return -1;
}

int CBSSTAR_get_remain_capacity(LEVEL l, int budget)
{
  struct timespec actual,ty;
  int    tx;
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
  kern_gettime(&actual);
  SUBTIMESPEC(&actual, &schedule_time, &ty);
  tx = TIMESPEC2USEC(&ty);
  // the remain capacity is from the first dispatch so is less then
  // actual capacity
  return (lev->b[budget].avail-tx);

}

int CBSSTAR_removebudget(LEVEL l, int budget)
{

  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  bandwidth_t b;
  
  b = (MAX_BANDWIDTH / lev->b[budget].T) * lev->b[budget].Q;

  lev->U -= b;

  lev->b[budget].Q = 0;
  lev->b[budget].T = 0;
  lev->b[budget].D = 0;
  NULL_TIMESPEC(&lev->b[budget].dline);
  NULL_TIMESPEC(&lev->b[budget].replenish);
  lev->b[budget].replenish_timer = NIL;
  lev->b[budget].avail = 0;
  lev->b[budget].current = -1;
  lev->b[budget].flags = CBSSTAR_ACTIVE;

  return 0;

}

int CBSSTAR_adjust_budget(LEVEL l, TIME Q, TIME T, TIME D, int budget)
{

  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  lev->b[budget].Q = Q;
  lev->b[budget].T = T;
  lev->b[budget].D = D;

  return 0;

}

int CBSSTAR_getbudgetinfo(LEVEL l, TIME *Q, TIME *T, TIME *D, int budget)
{

  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
 
  *Q = lev->b[budget].Q;
  *T = lev->b[budget].T;
  *D = lev->b[budget].D;

  return 0;

}

int CBSSTAR_get_last_reclaiming(LEVEL l, PID p)
{ 

  return 0;

}

int CBSSTAR_is_active(LEVEL l, int budget)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  return lev->b[budget].flags;

}

int CBSSTAR_get_local_scheduler_level_from_budget(LEVEL l, int budget)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  return lev->b[budget].loc_sched_level;

}

int CBSSTAR_get_local_scheduler_level_from_pid(LEVEL l, PID p)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  return lev->b[lev->tb[p]].loc_sched_level;

}

int CBSSTAR_get_local_scheduler_id_from_budget(LEVEL l, int budget)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  return lev->b[budget].loc_sched_id;

}

int CBSSTAR_get_local_scheduler_id_from_pid(LEVEL l, PID p)
{
  CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);

  return lev->b[lev->tb[p]].loc_sched_id;

}

void CBSSTAR_disable_server(LEVEL l, int budget)
{
 CBSSTAR_level_des *lev = (CBSSTAR_level_des *)(level_table[l]);
 int avail_budget;
 int spare;

 /* force a hard reservation event */
 avail_budget=lev->b[lev->tb[exec]].avail;
 lev->b[lev->tb[exec]].flags = CBSSTAR_NOACTIVE;
 //kern_printf("(CBS DS %d)", exec);
 /* save the unused capacity */
 spare=avail_budget+lev->b[lev->tb[exec]].avail;
 if (spare<=0) spare=0;
 
}
