/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   Michael Trimarchi   <trimarchi@gandalf.sssup.it>
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

#include "ll/i386/64bit.h"
#include "grubstar.h"
#include <tracer.h>

//#define GRUBSTAR_DEBUG

/* this structure contains the status for a single budget */
struct budget_struct {
  TIME Q;                 /* Budget */
  TIME T;                 /* Period */
  TIME D;		  /* Deadline */

  /* Negotiate parameter */
  TIME N_Q;               /* Negotiate budget */
  TIME N_T;               /* Negotiate Period */
  TIME N_D;               /* Negotiate Deadline */

  bandwidth_t Ub;         /* Bandwidth */

  struct timespec dline;     /* deadline */
  struct timespec replenish; /* server period */
  struct timespec vtimer_istant; /* timer of virtual time arrive */
  int replenish_timer;
  int vtimer;
  int avail;              /* current budget */
  
  LEVEL l;                /* Current GRUBSTAR level */
  int loc_sched_id;       /* Local scheduler id */
  LEVEL loc_sched_level;  /* Local scheduler level */

  int last_reclaiming;
  
  PID current;            /* the task currently put in execution */
  int flags;
  int negotiation;

  IQUEUE tasks;           /* a FIFO queue for the tasks handled
                             using the budget */

};

#define GRUBSTAR_NOACTIVE   0
#define GRUBSTAR_ACTIVE     1
#define GRUBSTAR_RECLAIMING 2

typedef struct {
  level_des l;               /* the standard level descriptor */

  struct budget_struct *b;   /* the budgets! */
  int n;                     /* the maximum index for the budgets */
  int freebudgets;           /* number of free budgets; starts from n */

  int tb[MAX_PROC];          /* link task->budget (used in guest_end) */

  bandwidth_t U;             /*+ the used bandwidth by the server       +*/
  bandwidth_t Uf;            /*+ the actual used bandwidth by the server       +*/

  int cap_lev;
  struct timespec cap_lasttime;
  int negotiation_in_progress;

  LEVEL scheduling_level;

} GRUBSTAR_level_des;


static void GRUBSTAR_timer_hardreservation(void *a)
{
  struct budget_struct *b = a;
  PID p;
  GRUBSTAR_level_des *lev;
  
  lev = (GRUBSTAR_level_des *)(level_table[b->l]);

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:HrdRes:");  
  #endif

  b->replenish_timer = NIL;

  b->last_reclaiming = 0;
  
  if (b->negotiation) {
    lev->negotiation_in_progress--;
    b->negotiation=0;
    b->Q=b->N_Q;
    b->T=b->N_T;
    b->D=b->N_D;

    b->N_Q=0;
    b->N_T=0;
    b->N_D=0;
  }

  b->avail += b->Q;
  if (b->avail > b->Q) b->avail = b->Q;

  if (b->avail > 0) b->flags = GRUBSTAR_ACTIVE;
 
  /* Tracer */
  TRACER_LOGEVENT(FTrace_EVT_server_replenish,0,(unsigned int)(b));
 
  if (b->flags==GRUBSTAR_RECLAIMING && b->avail>0) 
    if (lev->Uf < lev->U) lev->Uf += b->Ub;

  if (b->current == NIL && b->flags) {
    if (iq_query_first(&(b->tasks)) != NIL) {
	JOB_TASK_MODEL job;

        p = iq_getfirst(&b->tasks);
	
        #ifdef GRUBSTAR_DEBUG
          kern_printf("%d",p);
        #endif

	kern_gettime(&b->replenish);
        TIMESPEC_ASSIGN(&b->dline, &b->replenish);
	ADDUSEC2TIMESPEC(b->D, &b->dline);
        ADDUSEC2TIMESPEC(b->T, &b->replenish);


        b->current = p;
	
        job_task_default_model(job, b->dline);
	job_task_def_noexc(job);
 
	level_table[ lev->scheduling_level ]->
	  private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);
       
        event_need_reschedule();

    }
  }
  
  if (b->flags == GRUBSTAR_NOACTIVE) {
    kern_gettime(&b->replenish);
    ADDUSEC2TIMESPEC(b->T, &b->replenish);
  
    b->replenish_timer=kern_event_post(&b->replenish, GRUBSTAR_timer_hardreservation, b);
  }

  #ifdef GRUBSTAR_DEBUG
    kern_printf(")");
  #endif

}

void GRUBSTAR_ANC(void *arg)
{
  struct budget_struct *b = arg;
  GRUBSTAR_level_des *lev=(GRUBSTAR_level_des *)level_table[b->l];
 
  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:Rec:");
  #endif

  b->vtimer = NIL;
  
  if (b->current == NIL && iq_query_first(&(b->tasks)) == NIL && b->flags != GRUBSTAR_RECLAIMING) {
     event_need_reschedule();
     TRACER_LOGEVENT(FTrace_EVT_server_reclaiming,0,(unsigned int)b);
     b->flags = GRUBSTAR_RECLAIMING;
     lev->Uf -= b->Ub;
  }

  #ifdef GRUBSTAR_DEBUG
    kern_printf(")");
  #endif


}

static void GRUBSTAR_activation(GRUBSTAR_level_des *lev,
                           PID p,
                           struct timespec *acttime)
{
  JOB_TASK_MODEL job;
  struct budget_struct *b = &lev->b[lev->tb[p]];
  TIME t;
  struct timespec t2,t3;

  mul32div32to32(b->D,b->avail,b->Q,t);
  t3.tv_sec = t / 1000000;
  t3.tv_nsec = (t % 1000000) * 1000;

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:Act %d avail %d)",p,  b->avail);
  #endif

  TRACER_LOGEVENT(FTrace_EVT_server_active,proc_table[p].context,(unsigned int)b);

  if (b->vtimer!=NIL) {
     kern_event_delete(b->vtimer);
     //TIMESPEC_ASSIGN(acttime, &b->vtimer_istant);
  } 
  b->vtimer=NIL;
  SUBTIMESPEC(&b->dline, acttime, &t2);
  if (/* 1 */ TIMESPEC_A_LT_B(&b->dline, acttime) ||
       /* 2 */ TIMESPEC_A_GT_B(&t3, &t2) ) {     
  if (b->negotiation) {
	 lev->negotiation_in_progress--;
	 b->negotiation=0;
	 b->Q=b->N_Q;
	 b->T=b->N_T;
	 b->D=b->N_D;
	 
	 b->N_Q=0;
	 b->N_T=0;
	 b->N_D=0;
  }
  TIMESPEC_ASSIGN(&b->replenish, acttime);
  ADDUSEC2TIMESPEC(b->T, &b->replenish);
  TIMESPEC_ASSIGN(&b->dline, acttime);
  ADDUSEC2TIMESPEC(b->D, &b->dline);
  b->avail = b->Q;
  b->last_reclaiming = 0;
  }
  
  if (b->flags==GRUBSTAR_RECLAIMING)
    if (lev->Uf < lev->U) lev->Uf += b->Ub;

  b->flags=GRUBSTAR_ACTIVE;

  /* record the current task inserted in the master module */
  b->current = p;
  
  job_task_default_model(job, b->dline);
  job_task_def_noexc(job);
  level_table[ lev->scheduling_level ]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);

}

static void GRUBSTAR_account_capacity(GRUBSTAR_level_des *lev, PID p)
{
  struct timespec ty;
  TIME tx;
  struct budget_struct *b = &lev->b[lev->tb[p]];
  TIME t;
  struct timespec t2,t3,t4;

  if (b->vtimer!=NIL) kern_event_delete(b->vtimer);
  b->vtimer=NIL;
  tx = 0;

  if (lev->cap_lev != NIL) { 
    kern_event_delete(lev->cap_lev); 
    lev->cap_lev = NIL;
  }

  if (TIMESPEC_A_LT_B(&lev->cap_lasttime,&schedule_time)) {
     SUBTIMESPEC(&schedule_time, &lev->cap_lasttime, &ty);
     tx = TIMESPEC2USEC(&ty);
  }
 
  // recalculate the new effective last reclaiming
  mul32div32to32(tx,(lev->U - lev->Uf),lev->U,b->last_reclaiming);
  //b->avail -= tx; 
  b->avail -= (tx-b->last_reclaiming);

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:Cap p%d av=%d tx=%d lr=%d)", p, b->avail,(int)tx, (int)lev->b[lev->tb[p]].last_reclaiming);
  #endif
    // kern_printf("(U %d, Uf %d)", lev->U, lev->Uf);


  if (b->avail <= 0) {
    TRACER_LOGEVENT(FTrace_EVT_server_exhaust,0,(unsigned int)(b));
    b->flags = GRUBSTAR_NOACTIVE;
  }

  if (TIMESPEC_A_LT_B(&b->dline, &schedule_time)) {
    /* we modify the deadline ... */
    TIMESPEC_ASSIGN(&b->dline, &schedule_time);
    ADDUSEC2TIMESPEC(b->D, &b->dline);
    TIMESPEC_ASSIGN(&b->replenish, &schedule_time);
    ADDUSEC2TIMESPEC(b->T, &b->replenish);
  }

  if (b->flags == GRUBSTAR_NOACTIVE && b->replenish_timer == NIL)  {
    b->replenish_timer=kern_event_post(&b->replenish, GRUBSTAR_timer_hardreservation, b);
  } else {
    mul32div32to32(b->D,b->avail,b->Q,t);
    t3.tv_sec = t / 1000000;
    t3.tv_nsec = (t % 1000000) * 1000;
    SUBTIMESPEC(&b->dline, &t3, &t2);
    TIMESPEC_ASSIGN(&b->vtimer_istant , &t2);
    b->vtimer = kern_event_post(&t2, GRUBSTAR_ANC, b);
  }
}

static void capacity_handler(void *l)
{
  
  GRUBSTAR_level_des *lev = l;
  lev->cap_lev = NIL;
  #ifdef GRUBSTAR_DEBUG
    kern_printf("(*)");
  #endif  
  event_need_reschedule();
 
}

static int GRUBSTAR_private_eligible(LEVEL l, PID p)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  struct budget_struct *b=&lev->b[lev->tb[p]];
  JOB_TASK_MODEL job; 
 
  if ( TIMESPEC_A_LT_B(&b->dline, &schedule_time)) {
#ifdef GRUBSTAR_DEBUG 
      kern_printf("(GS:Eli:%d)",p);
#endif
      if (lev->cap_lev!=NIL) {
        kern_event_delete(lev->cap_lev);
        lev->cap_lev=NIL; 
      } 
	    
      /* we kill the current activation */
      level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level, p);
      
      /* and the capacity */
      if (b->negotiation) {
	lev->negotiation_in_progress--;
	b->negotiation=0;
	b->Q=b->N_Q;
	b->T=b->N_T;
	b->D=b->N_D;
	
	b->N_Q=0;
	b->N_T=0;
	b->N_D=0;
      }

      /* we modify the deadline ... */
      kern_gettime(&b->replenish);
      TIMESPEC_ASSIGN(&b->dline, &b->replenish);
      ADDUSEC2TIMESPEC(b->D, &b->dline);
      ADDUSEC2TIMESPEC(b->T, &b->replenish);      

      b->avail = b->Q;
      b->last_reclaiming = 0;
  
      /* Tracer */
      TRACER_LOGEVENT(FTrace_EVT_server_replenish,0,(unsigned int)(b));

      if (b->flags == GRUBSTAR_RECLAIMING) {
        if (lev->Uf < lev->U) lev->Uf += b->Ub;
      }

      b->flags = GRUBSTAR_ACTIVE;

      if (b->replenish_timer!=NIL)  {
        kern_event_delete(b->replenish_timer);
	b->replenish_timer=NIL;
      }

      if (b->vtimer!=NIL)  {
            kern_event_delete(b->vtimer);
            b->vtimer=NIL;
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

static void GRUBSTAR_private_insert(LEVEL l, PID p, TASK_MODEL *m)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  BUDGET_TASK_MODEL *budget;

  if (m->pclass != BUDGET_PCLASS ||
      (m->level != 0 && m->level != l)) {
    kern_raise(XINVALID_TASK, p);
    return;
  }
  budget = (BUDGET_TASK_MODEL *)m;

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:PriIns:%d:%d", p, budget->b);
  #endif
  if (budget->b == -1)
    return;
  lev->tb[p] = budget->b;

  if (lev->b[budget->b].current == NIL && lev->b[budget->b].flags ) {
    /* This is the first task in the budget,
       the task have to be inserted into the master module */
    struct timespec t;
   
    kern_gettime(&t);
    GRUBSTAR_activation(lev,p,&t);

  } else {
    /* The budget is not empty, another task is already into the
       master module, so the task is inserted at the end of the budget
       queue */
 
      iq_insertlast(p,&lev->b[budget->b].tasks);
 

  }

  #ifdef GRUBSTAR_DEBUG
    kern_printf(")");
  #endif

}

static void GRUBSTAR_private_extract(LEVEL l, PID p)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:Ext:%d)", p);
  #endif
  /* a task is removed from execution for some reasons. It must be
     that it is the first in its budget queue (only the first task in
     a budget queue is put into execution!) */

  /* remove the task from execution (or from the ready queue) */
  if (lev->b[lev->tb[p]].current == p) {

    GRUBSTAR_account_capacity(lev,p);
    /* remove the task from the master module */
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level, p);

    //iq_extract(p, &lev->b[lev->tb[p]].tasks);
    //lev->b[lev->tb[p]].current = NIL;

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
      GRUBSTAR_activation(lev,n,&t);  // it modifies b[lev->tb[p]].current
    }
    else
      lev->b[lev->tb[p]].current=NIL;

  }
  else  {
    iq_extract(p, &lev->b[lev->tb[p]].tasks);
  }
}

static void GRUBSTAR_private_dispatch(LEVEL l, PID p, int nostop)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  struct timespec ty;
  int temp;

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:Dsp:%d)", p);
  #endif

  /* the current task (that is the only one inserted in the master module
     for the corresponding budget) is dispatched. Note that the current
     task is not inserted in any FIFO queue, so the task does not have to
     be extracted! */

  /* ... then, we dispatch it to the master level */
  if (!nostop)
  	level_table[ lev->scheduling_level ]->
    		private_dispatch(lev->scheduling_level,p,nostop);

  TIMESPEC_ASSIGN(&ty, &schedule_time);
  TIMESPEC_ASSIGN(&lev->cap_lasttime, &schedule_time);

  /* ...and finally, we have to post a capacity event */
 
  /* Reclaiming Computation */
  
  if ((lev->U - lev->Uf) < 0) {
    cprintf("GRUBSTAR Error: Bad Reclaiming Computation\n");
    kern_raise(XUNSPECIFIED_EXCEPTION, exec_shadow);
  }
  
  mul32div32to32(lev->b[lev->tb[exec]].avail,(lev->U-lev->Uf),lev->U,lev->b[lev->tb[exec]].last_reclaiming);
 

  if (lev->b[lev->tb[exec]].last_reclaiming > 0)
    TRACER_LOGEVENT(FTrace_EVT_server_using_rec,(unsigned short int)lev->b[lev->tb[exec]].last_reclaiming,(unsigned int)(&lev->b[lev->tb[exec]]));

  temp = lev->b[lev->tb[exec]].avail + lev->b[lev->tb[exec]].last_reclaiming;

  ADDUSEC2TIMESPEC(temp,&ty);

  lev->cap_lev = kern_event_post(&ty,capacity_handler, lev);
  
}

static void GRUBSTAR_private_epilogue(LEVEL l, PID p)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  struct budget_struct *b;
  int skip_epilog;
  skip_epilog=0;
 
  #ifdef GRUBSTAR_DEBUG
     kern_printf("(GS:Epi:%d)",p);
  #endif

  if (p==exec) b = &lev->b[lev->tb[p]];
     else if (lev->tb[exec]!=NIL)  {
	  b = &lev->b[lev->tb[exec]];
	  p=exec;
	  skip_epilog=1;
       }
       else return; 	
   

    GRUBSTAR_account_capacity(lev,p);
    if (b->flags)  {

      /* there is capacity available, maybe it is simply a preemption;
	 the task have to return to the ready queue */
      if (!skip_epilog)
      	level_table[ lev->scheduling_level ]->
	  private_epilogue(lev->scheduling_level,p); //else kern_printf("(SP)");
    
    } else {
      /* we kill the current activation */
      #ifdef GRUBSTAR_DEBUG
        kern_printf("(GS:HRExt:%d)",p);
      #endif  
    
      level_table[ lev->scheduling_level ]->
	  private_extract(lev->scheduling_level, p);    
      
      //iq_extract(p, &b->tasks);

      iq_insertfirst(p, &b->tasks);
      
      b->current = NIL;
 
    }
   
}

static int GRUBSTAR_public_message(LEVEL l, PID p, void *m)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  struct budget_struct *b = &lev->b[lev->tb[p]]; 

  switch((long)(m)) {

    case (long)(NULL):
 
      #ifdef GRUBSTAR_DEBUG
        kern_printf("(GS:EndCycle:%d:%d)",p,lev->tb[p]);
      #endif     

      if (b->current == NIL && iq_query_first(&(b->tasks)) == NIL && b->flags != GRUBSTAR_RECLAIMING) {
        
        if (b->vtimer!=NIL)  {
            kern_event_delete(b->vtimer);
            b->vtimer=NIL;
        }

        TRACER_LOGEVENT(FTrace_EVT_server_reclaiming,0,(unsigned int)(b));
        b->flags = GRUBSTAR_RECLAIMING;
        lev->Uf -= b->Ub;

      }

      break;

    case 1:

      break;

  }

  return 0;

}

/* Registration functions }*/

LEVEL GRUBSTAR_register_level(int n, LEVEL master)
{
  LEVEL l;            /* the level that we register */
  GRUBSTAR_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

  printk("GRUBSTAR_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(GRUBSTAR_level_des));

  lev = (GRUBSTAR_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.private_insert   = GRUBSTAR_private_insert;
  lev->l.private_extract  = GRUBSTAR_private_extract;
  lev->l.private_eligible = GRUBSTAR_private_eligible;
  lev->l.private_dispatch = GRUBSTAR_private_dispatch;
  lev->l.private_epilogue = GRUBSTAR_private_epilogue;

  lev->l.public_guarantee = NULL;
  lev->l.public_message = GRUBSTAR_public_message;

  lev->b = (struct budget_struct *)kern_alloc(sizeof(struct budget_struct)*n);

  for (i=0; i<n; i++) {
    lev->b[i].Q = 0;
    lev->b[i].T = 0;
    lev->b[i].D = 0;
  
    lev->b[i].N_Q = 0;
    lev->b[i].N_T = 0;
    lev->b[i].N_D = 0;

    lev->b[i].Ub = 0;
    NULL_TIMESPEC(&lev->b[i].dline);
    NULL_TIMESPEC(&lev->b[i].replenish);
    NULL_TIMESPEC(&lev->b[i].vtimer_istant);
    lev->b[i].replenish_timer = NIL;
    lev->b[i].vtimer = NIL;
    lev->b[i].avail = 0;
    lev->b[i].current = -1;
    lev->b[i].flags = GRUBSTAR_ACTIVE;
    lev->b[i].l = l;
    lev->b[i].last_reclaiming = 0;
    lev->b[i].negotiation = 0;
    lev->b[i].loc_sched_level=-1;
    iq_init(&lev->b[i].tasks, NULL, 0);
    
  }

  lev->n = n;
  lev->freebudgets = 0;

  for (i=0; i<MAX_PROC; i++) 
    lev->tb[i] = NIL;

  lev->negotiation_in_progress=0;
  lev->U = 0;
  lev->Uf = 0;
  lev->cap_lev = NIL;
  NULL_TIMESPEC(&lev->cap_lasttime);
  lev->scheduling_level = master;

  return l;

}

int GRUBSTAR_setbudget(LEVEL l, TIME Q, TIME T, TIME D, LEVEL local_scheduler_level, int scheduler_id)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  int r;

  #ifdef GRUBSTAR_DEBUG
    kern_printf("(GS:SetBud)");
  #endif

  for (r = 0; r < lev->n; r++)
    if (lev->b[r].Q == 0) break;

  if (r != lev->n) {
    bandwidth_t b;
    mul32div32to32(MAX_BANDWIDTH,Q,T,b);
    
    /* really update lev->U, checking an overflow... */
    if (Q< T /* && MAX_BANDWIDTH - lev->U > b */) {
      
      lev->U += b;
      //lev->Uf += b; Start in Reclaiming
      lev->freebudgets++;
      
      lev->b[r].Q = Q;
      lev->b[r].T = T;
      lev->b[r].D = D;

      lev->b[r].N_Q = 0;
      lev->b[r].N_T = 0;
      lev->b[r].N_D = 0;
      lev->b[r].negotiation=0;

      lev->b[r].Ub = b;
      lev->b[r].avail = Q;
      lev->b[r].flags = GRUBSTAR_RECLAIMING;
      lev->b[r].loc_sched_id = scheduler_id;
      lev->b[r].loc_sched_level = local_scheduler_level;
      lev->b[r].last_reclaiming = 0;      

      return r;
    }
    else
      return -2;
  }
  else
    return -1;
}

int GRUBSTAR_removebudget(LEVEL l, int budget)
{

  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  bandwidth_t b;
  
  mul32div32to32(MAX_BANDWIDTH,lev->b[budget].Q,lev->b[budget].T,b);

  lev->U -= b;
  if (lev->b[budget].flags != GRUBSTAR_RECLAIMING) lev->Uf -= b;

  lev->b[budget].Q = 0;
  lev->b[budget].T = 0;
  lev->b[budget].D = 0;

  lev->b[budget].N_Q = 0;
  lev->b[budget].N_T = 0;
  lev->b[budget].N_D = 0;

  lev->b[budget].negotiation=0;

  lev->b[budget].Ub = 0;
  NULL_TIMESPEC(&lev->b[budget].dline);
  NULL_TIMESPEC(&lev->b[budget].replenish);
  lev->b[budget].replenish_timer = NIL;
  lev->b[budget].avail = 0;
  lev->b[budget].current = -1;
  lev->b[budget].flags = GRUBSTAR_RECLAIMING;
  lev->b[budget].last_reclaiming = 0;
  lev->b[budget].loc_sched_level = -1;

  return 0;

}

/* Change Q and T parameters of a budget
 * It returns the time to wait before the change is effective
 */
int GRUBSTAR_adjust_budget(LEVEL l, TIME Q, TIME T, TIME D, int budget)
{

  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  bandwidth_t b;
  TIME OldPeriod =  TIMESPEC2USEC(&lev->b[budget].dline);

  mul32div32to32(MAX_BANDWIDTH,lev->b[budget].Q,lev->b[budget].T,b);

  lev->U -= b;
  if (lev->b[budget].flags != GRUBSTAR_RECLAIMING) lev->Uf -= b;

  lev->b[budget].N_Q = Q;
  lev->b[budget].N_T = T;
  lev->b[budget].N_D = D;

  if (0 && lev->b[budget].current!=NIL && !(lev->b[budget].Q==Q && lev->b[budget].T==T)) {
    lev->b[budget].N_Q = Q;
    lev->b[budget].N_T = T;
    lev->b[budget].N_D = D;
    if (!lev->b[budget].negotiation) {
      lev->negotiation_in_progress++;
      lev->b[budget].negotiation=1;
    }				      		       
  } else {
    lev->b[budget].Q = Q;
    lev->b[budget].T = T;
    lev->b[budget].D = D;
  }

  mul32div32to32(MAX_BANDWIDTH,Q,T,lev->b[budget].Ub);

  lev->U += lev->b[budget].Ub;
  if (lev->b[budget].flags != GRUBSTAR_RECLAIMING) lev->Uf += lev->b[budget].Ub;  

  #ifdef GRUBSTAR_DEBUG
    cprintf("[%d:Q=%d:T=%d:D=%d]",budget,(int)Q,(int)T,(int)D);
  #endif

  if (lev->b[budget].flags == GRUBSTAR_NOACTIVE)
    return 0;
  if (lev->b[budget].flags == GRUBSTAR_RECLAIMING)
    return 0;
  
  return OldPeriod;

}

int GRUBSTAR_getbudgetinfo(LEVEL l, TIME *Q, TIME *T, TIME *D, int budget)
{

  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  if (lev->b[budget].Q) {
    if (Q) *Q = lev->b[budget].Q;
    if (T) *T = lev->b[budget].T;
    if (D) *D = lev->b[budget].D;
    return 0;
  }

  return -1;

}

int GRUBSTAR_get_last_reclaiming(LEVEL l, PID p)
{

  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->b[lev->tb[p]].last_reclaiming;

}

int GRUBSTAR_is_active(LEVEL l, int budget)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->b[budget].flags;

}

int GRUBSTAR_get_remain_capacity(LEVEL l, int budget)
{
  struct timespec actual,ty;
  int    tx;
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  TIME reclaim=0;
  TIME remain=0;
  SYS_FLAGS f;
  
  f=kern_fsave();
  kern_gettime(&actual);
  SUBTIMESPEC(&actual, &schedule_time, &ty);

  tx = TIMESPEC2USEC(&ty);
  mul32div32to32(tx,(lev->U - lev->Uf),lev->U,reclaim);

  // the remain capacity is from the first dispatch so is less then
  // actual capacity
  remain=lev->b[budget].avail-tx+reclaim;
  kern_frestore(f);

  return remain;

}

int GRUBSTAR_get_local_scheduler_level_from_budget(LEVEL l, int budget)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->b[budget].loc_sched_level;

}

int GRUBSTAR_get_local_scheduler_level_from_pid(LEVEL l, PID p)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->b[lev->tb[p]].loc_sched_level;

}

int GRUBSTAR_get_local_scheduler_id_from_budget(LEVEL l, int budget)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->b[budget].loc_sched_id;

}

/* return the sum of servers bandwith */
bandwidth_t GRUBSTAR_return_bandwidth(LEVEL l) {
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->U;
}

int GRUBSTAR_get_local_scheduler_id_from_pid(LEVEL l, PID p)
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

  return lev->b[lev->tb[p]].loc_sched_id;

}

void GRUBSTAR_disable_server(LEVEL l, int budget)
{
 GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
 int avail_budget;
 int spare;

 /* force a hard reservation event */
 avail_budget=lev->b[lev->tb[exec]].avail;
 lev->b[lev->tb[exec]].flags = GRUBSTAR_NOACTIVE;
 //kern_printf("(GS DS %d)", exec);
 /* save the unused capacity */
 spare=avail_budget+lev->b[lev->tb[exec]].avail;
 if (spare<=0) spare=0;
 
}

int GRUBSTAR_get_renegotiation_status(LEVEL l, int budget) 
{
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);
  if (lev->negotiation_in_progress) return 1;
  else return 0;
}

void  GRUBSTAR_getdeadline(LEVEL l, int budget, struct timespec *t) {
  
  GRUBSTAR_level_des *lev = (GRUBSTAR_level_des *)(level_table[l]);

   TIMESPEC_ASSIGN(t,&lev->b[budget].dline);
}
