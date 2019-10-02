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

#include "ll/i386/64bit.h"
#include "fedfstar.h"
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

//#define FEDFSTAR_DEBUG
//#define FDB_DEBUG

#ifdef FEDFSTAR_DEBUG

static __inline__ fake_printf(char *fmt, ...) {}

//#define fedfstar_printf fake_printf
//#define fedfstar_printf2 fake_printf
//#define fedfstar_printf3 fake_printf

#define fedfstar_printf kern_printf
#define fedfstar_printf2 kern_printf
#define fedfstar_printf3 kern_printf
#endif

/*
 * DEBUG stuffs end
 */

/* Status used in the level */
#define FEDFSTAR_READY         MODULE_STATUS_BASE    /* - Ready status        */
#define FEDFSTAR_IDLE          MODULE_STATUS_BASE+4  /* to wait the deadline  */

/* flags */
#define FEDFSTAR_CHANGE_LEVEL     8 
#define FEDFSTAR_FLAG_NOPREEMPT   4 
#define FEDFSTAR_FLAG_NORAISEEXC  2
#define FEDFSTAR_FLAG_SPORADIC    1


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

  int new_level[MAX_PROC];
  int wcet[MAX_PROC]; /* save the wcet fields */

  int ctime[MAX_PROC]; /* last computation time */
  int cavg[MAX_PROC]; /* avg of computation time */
  int ccount[MAX_PROC]; /* number of istance */

  long long int u_predicted;
  long long int u_p_avg;
  long long int sched_error_prev;
  int nro;
  long long int delta_old;
  long long int delta_new;
  

} FEDFSTAR_level_des;

#define HISTORY_BACK 4
#define GRANULARITY  5 // in microseconds

typedef bandwidth_t inv_bandwidth_t;

static long long int tita[HISTORY_BACK][FSF_MAX_N_SERVERS];
static bandwidth_t u[HISTORY_BACK][FSF_MAX_N_SERVERS];


void init_feedbackstruct(void) {
  int i=0, j=0;
  for (i=0; i<FSF_MAX_N_SERVERS; i++)  {
    for (j=0; j<HISTORY_BACK; j++) {
      u[j][i]=0;
      tita[j][i]=0;
    }
  }
}


long long int FDB_sample(FEDFSTAR_level_des *lev, int sched_error, TIME server_period, TIME cavg)
{
 
  long long int temp,temp1;
  long long int delta_new=0;
  int beta_1; //Not really this value
  int gamma_1;
  int beta_2;
  int gamma_2;

#ifdef FDB_DEBUG
     kern_printf("(FDB:sample:period:%d:cavg:%d:sched_error:%d)",(int)server_period,(int)cavg, sched_error);
#endif
     
  //if (delta_old==0)
  //delta_old=1000*server_period/cavg;
  temp=1000*server_period/cavg;
  beta_1=temp*6/5;
  gamma_1=temp*(-93)/100;
  beta_2=temp/5;
  gamma_2=temp*7/100;

#ifdef FDB_DEBUG
     kern_printf("(FDB:sample:b1:%d:g1:%d:b2:%d:g2:%d:temp:%d)",beta_1,gamma_1,beta_2,gamma_2, (int)temp);
#endif

  if ((long long int)sched_error>=(long long int)server_period) {
    temp=beta_1*(long long int)sched_error/server_period;
    temp1=gamma_1*(long long int)lev->sched_error_prev/server_period; 
    delta_new=lev->delta_old-temp-temp1;
  }
  else {
    temp=beta_2*(long long int)sched_error/server_period;
    temp1=gamma_2*(long long int)lev->sched_error_prev/server_period; 
    delta_new=lev->delta_old-temp-temp1;
  }
 #ifdef FDB_DEBUG
     kern_printf("(FDB1:sched_error:%d:delta_new:%lld)",sched_error, delta_new);
  #endif

 lev->delta_old=delta_new;
 
 lev->sched_error_prev=sched_error;

 return(delta_new);
}


/* This function preditcs the value of the next execution time based on the history of the task.
The algorithm is known as LMS and minimizes the square error.
It has to add the blocking time if the task may be block
And to determine the bandwidth it has to add the bandwidth determine in the previous function
*/
TIME FDB_activate(FEDFSTAR_level_des *lev,fsf_server_id_t server, TIME server_period, TIME blk_time, TIME c_time, TIME c_avg, bandwidth_t bw)
{
  //Filter coefficients
  static int mu=4; //convergence factor
  static long long int pred_error=0;
  long long int u_avg;
  bandwidth_t unew;
  int i;
  TIME Q_new;
  long long int u_new;
  
  //u_avg=server_bandwidth_avg;


  lev->nro++;
  pred_error=lev->u_predicted-(long long int)u[0][server];
  

  for (i=0;i<HISTORY_BACK;i++) {
    long long int temp=0;
    temp=(pred_error*(long long int)u[i][server])/MAX_BANDWIDTH*mu;
    tita[i][server]=tita[i][server]-temp;    
  }
  
  lev->u_predicted=0;
  for (i=0;i<HISTORY_BACK;i++)
    lev->u_predicted+=((tita[i][server]*(long long int)u[i][server])/MAX_BANDWIDTH);
  
  for (i=0; i<(HISTORY_BACK-1); i++) 
    u[i+1][server]=u[i][server];
 
  lev->u_p_avg=(lev->u_p_avg*(lev->nro-1)+lev->u_predicted)/lev->nro; //OK
  

  //  if (c_avg!=0)
  //  u_avg=MAX_BANDWIDTH*c_avg/server_period;
  //else
  //  u_avg=bw;

  if (lev->u_p_avg==0)
    lev->u_p_avg=bw;


#ifdef FDB_DEBUG
  kern_printf("(FDB:u_p_avg %lld)\n", lev->u_p_avg);
  kern_printf("(FDB:c_time %d)\n---------------\n",c_time);
#endif

  if (c_time!=0)  
    mul32div32to32(MAX_BANDWIDTH,c_time,server_period,u[0][server]); // last real computation value
  else u[0][server]=bw;
  
  // c_predicted+=blk_time;
  
  
 
  /* What follows is the update of the server budget based on the FDB_sample return value and the prediction function
   */
 if (lev->delta_new!=0)  
    lev->delta_new=MAX_BANDWIDTH*1000/lev->delta_new; /*1000; */
 u_new=lev->delta_new+lev->u_p_avg;
 //if (u_new<0) exit(-1);
 //if (u_new>MAX_BANDWIDTH) exit(-1);
 unew=u_new;

#ifdef FDB_DEBUG
 kern_printf("(FDB:unew %d)", unew);
#endif

 mul32div32to32(server_period,unew, MAX_BANDWIDTH, Q_new);
 

 
 //Q_new=server_period*1000/u;
#ifdef FDB_DEBUG
 kern_printf("(FDB:uavg %lld\n)", lev->u_p_avg);
#endif
#ifdef FDB_DEBUG 
 kern_printf("(FDB:act_int:qnew:%ld)",Q_new);
#endif
 return(Q_new);
}


static void capacity_handler(void *l)
{
  FEDFSTAR_level_des *lev = l;
  lev->cap_lev = NIL;
  event_need_reschedule();
}

static void FEDFSTAR_check_preemption(FEDFSTAR_level_des *lev)
{
  PID first=NIL;

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:chk)");
  #endif
  /* check if the task is preempteble or not */
  if (lev->activated != NIL && lev->flag[lev->activated] & FEDFSTAR_FLAG_NOPREEMPT) return;

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

static void FEDFSTAR_timer_deadline(void *par);

static void FEDFSTAR_internal_activate(FEDFSTAR_level_des *lev, PID p, 
				      struct timespec *t)
{
  TIME Q, D, T;
  int budget;
  bandwidth_t bw;

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:iact)");
  #endif

  ADDUSEC2TIMESPEC(lev->period[p], t);

  *iq_query_timespec(p, &lev->ready) = *t;
  lev->deadline_timespec[p] = *t;

  /* Insert task in the correct position */
  proc_table[p].status = FEDFSTAR_READY;
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].control &= ~CONTROL_CAP;

  budget=lev->budget[p];
  SERVER_getbudgetinfo(lev->scheduling_level, &Q, &T, &D, budget);

  mul32div32to32(MAX_BANDWIDTH, Q, T, bw);

  Q=FDB_activate(lev, budget, T, 0, lev->ctime[p],lev->cavg[p], bw);
  
  SERVER_adjust_budget(lev->scheduling_level, Q, T, D, budget);


  /* check for preemption */
  FEDFSTAR_check_preemption(lev);
}

static void FEDFSTAR_timer_deadline(void *par)
{
  PID p = (PID) par;
  FEDFSTAR_level_des *lev;

  lev = (FEDFSTAR_level_des *)level_table[proc_table[p].task_level];
  lev->deadline_timer[p] = NIL;   

  switch (proc_table[p].status) {
    case FEDFSTAR_IDLE:
      /* set the request time */
      if (!(lev->flag[p] & FEDFSTAR_FLAG_SPORADIC))
        FEDFSTAR_internal_activate(lev,p,iq_query_timespec(p, &lev->ready));

      event_need_reschedule();
      break;

    default:
      #ifdef FEDFSTAR_DEBUG
        kern_printf("(E:Dl:%d)",p);
      #endif
      /* else, a deadline miss occurred!!! */
      lev->dline_miss[p]++;
      TRACER_LOGEVENT(FTrace_EVT_task_deadline_miss,proc_table[p].context,proc_table[p].task_level);

      /* the task is into another state */
      if (!(lev->flag[p] & FEDFSTAR_FLAG_SPORADIC)) {
        lev->nact[p]++;
        ADDUSEC2TIMESPEC(lev->period[p], &lev->deadline_timespec[p]);
      }
  }

  /* Set the deadline timer */
  if (!(lev->flag[p] & FEDFSTAR_FLAG_SPORADIC))
    lev->deadline_timer[p] = kern_event_post(&lev->deadline_timespec[p],
                                             FEDFSTAR_timer_deadline,
                                             (void *)p);

}

static int FEDFSTAR_private_change_level(LEVEL l, PID p)
{

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  /* Change task level */
  if (lev->flag[p] & FEDFSTAR_CHANGE_LEVEL) {
    
#ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:clev)");
#endif
  
    STD_command_message msg;
    
    proc_table[p].status = SLEEP;
    lev->flag[p] &= ~ FEDFSTAR_CHANGE_LEVEL;
    
    level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);
    iq_extract(p,&lev->ready);
    
    if (lev->deadline_timer[p] != -1)
      kern_event_delete(lev->deadline_timer[p]);
    lev->deadline_timer[p]=NIL;
    
    FEDFSTAR_check_preemption(lev);
    
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


static void FEDFSTAR_timer_guest_deadline(void *par)
{
  PID p = (PID) par;

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:gdl)");
  #endif

  kern_raise(XDEADLINE_MISS,p);
}

static int FEDFSTAR_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  /* if the FEDFSTAR_task_create is called, then the pclass must be a
     valid pclass. */
  HARD_TASK_MODEL *h;

  if (m->pclass != HARD_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  h = (HARD_TASK_MODEL *)m;
  if (!h->wcet || !h->mit) return -1;
  /* now we know that m is a valid model */

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:Crt)");
  #endif

  lev->period[p] = h->mit;

  lev->flag[p] = 0;

  if (h->periodicity == APERIODIC)
       lev->flag[p] |= FEDFSTAR_FLAG_SPORADIC;

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


static void FEDFSTAR_account_capacity(FEDFSTAR_level_des *lev, PID p)
{
  struct timespec ty;
  TIME tx;


  SUBTIMESPEC(&schedule_time, &lev->cap_lasttime, &ty);
  tx = TIMESPEC2USEC(&ty);

  proc_table[p].avail_time -= tx;
}

static int FEDFSTAR_public_eligible(LEVEL l, PID p)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf2("(E:eli:%d)",p);
  #endif

  return level_table[ lev->scheduling_level ]->
    private_eligible(lev->scheduling_level,p);

}

static void FEDFSTAR_public_dispatch(LEVEL l, PID p, int nostop)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  struct timespec ty;

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:dis)");
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

static void FEDFSTAR_public_epilogue(LEVEL l, PID p)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:epi ");
  #endif

  if (lev->cap_lev!=NIL) {
       kern_event_delete(lev->cap_lev);
       lev->cap_lev=NIL;
  }

 
  if ( proc_table[exec].task_level==l ) {
    
     if (proc_table[exec].avail_time > 0) FEDFSTAR_account_capacity(lev,exec);

     if (FEDFSTAR_private_change_level(l, p)) return;

     /* check if the wcet is finished... */
     if (proc_table[exec].avail_time < 0) {
        /* wcet finished: disable wcet event and count wcet miss */

       #ifdef FEDFSTAR_DEBUG
         edfstar_printf2("W%d",p);
       #endif
        //proc_table[p].control &= ~CONTROL_CAP;
        lev->wcet_miss[exec]++;
	proc_table[exec].avail_time = 0;
	TRACER_LOGEVENT(FTrace_EVT_task_wcet_violation,proc_table[exec].context,proc_table[exec].task_level);
     }

     #ifdef FEDFSTAR_DEBUG
       edfstar_printf(")");
     #endif

     level_table[ lev->scheduling_level ]->
       private_epilogue(lev->scheduling_level,p);

      proc_table[exec].status = FEDFSTAR_READY;
    } else
        level_table[proc_table[exec].task_level]->public_epilogue(proc_table[exec].task_level,p);

}

static void FEDFSTAR_public_activate(LEVEL l, PID p, struct timespec *o)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  struct timespec t;

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:act:%d)",p);
  #endif

  /* Test if we are trying to activate a non sleeping task    */
  /* save activation (only if needed... */
  if (proc_table[p].status != SLEEP) {
    /* a periodic task cannot be activated when it is already active */
    /* but aperiodic task can be reactivate before */
    if (lev->flag[p] & FEDFSTAR_FLAG_SPORADIC) {
	if (proc_table[p].status != FEDFSTAR_IDLE) {
	  lev->nact[p]++;
	  //kern_printf("(Inc nact %d)",p);
	  //kern_printf("(%d STATUS %d %ds %dns)", p, proc_table[p].status, o->tv_sec, o->tv_nsec/1000);
	  return;
	}
    } else {
      return;
      //kern_raise(XACTIVATION,p);
    }
  }

  kern_gettime(&t);

  FEDFSTAR_internal_activate(lev,p, &t);

  /* Set the deadline timer */
  lev->deadline_timer[p] = kern_event_post(&lev->deadline_timespec[p],
                                           FEDFSTAR_timer_deadline,
                                           (void *)p);

}

static void FEDFSTAR_public_unblock(LEVEL l, PID p)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:ins)");
  #endif

  /* Insert task in the correct position */
  proc_table[p].status = FEDFSTAR_READY;
  iq_timespec_insert(p,&lev->ready);

  /* and check for preemption! */
  FEDFSTAR_check_preemption(lev);

}

static void FEDFSTAR_public_block(LEVEL l, PID p)
{

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  #ifdef FEDFSTAR_DEBUG
   edfstar_printf("(E:ext)");
  #endif

  /* the task is blocked on a synchronization primitive. we have to
     remove it from the master module -and- from the local queue! */
  iq_extract(p,&lev->ready);

  /* and finally, a preemption check! (it will also call guest_end) */
  FEDFSTAR_check_preemption(lev);
}

static int FEDFSTAR_public_message(LEVEL l, PID p, void *m)
{

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  struct timespec temp;
  STD_command_message *msg;
  HARD_TASK_MODEL *h;
  TIME Q, D, T;
  struct timespec t,diff;
  int sched_error_act;
  int budget;
  
  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:ecy ");
  #endif

  switch ((long)(m)) {

    /* Task EndCycle */
    case (long)(NULL):

      if (FEDFSTAR_private_change_level(l,p)) return 0;
    
      sched_error_act=0;
      /* Get the server deadline */
      budget=FEDFSTAR_getbudget(l,p);
      SERVER_getbudgetinfo(lev->scheduling_level, &Q, &T, &D, budget);
      SERVER_getdeadline(lev->scheduling_level, budget, &t);
      /* Get the task deadline */
      temp = *iq_query_timespec(p, &lev->ready);
#ifdef FDB_DEBUG
      kern_printf("(dt sec %ld, us %ld)", temp.tv_sec, temp.tv_nsec/1000);
      kern_printf("(ds sec %ld, us %ld)", t.tv_sec, t.tv_nsec/1000);
#endif
      if (/* 1 */ TIMESPEC_A_LT_B(&t,&temp)) {	 
	SUBTIMESPEC(&temp, &t, &diff);
	sched_error_act=-TIMESPEC2USEC(&diff);
      }
      else {
	SUBTIMESPEC(&t, &temp, &diff);
	sched_error_act=TIMESPEC2USEC(&diff);
      }
      
      if (proc_table[p].avail_time > 0) FEDFSTAR_account_capacity(lev,p);

       /* we call guest_end directly here because the same task may
         be reinserted in the queue before calling the preemption check! */
       level_table[ lev->scheduling_level ]->
         private_extract(lev->scheduling_level,p);
       lev->activated = NIL;

       iq_extract(p,&lev->ready);

  
       
       lev->ctime[p]=proc_table[p].wcet-proc_table[p].avail_time;

       lev->cavg[p]=(lev->ctime[p]+(lev->cavg[p])*(lev->ccount[p]))/(lev->ccount[p]+1);      
       
       lev->ccount[p]=(lev->ccount[p]%INT_MAX)+1;
       
       lev->delta_new=FDB_sample(lev,sched_error_act, T, lev->cavg[p]);

       /* we reset the capacity counters... */
       proc_table[p].avail_time = proc_table[p].wcet;

       if (lev->nact[p] > 0) {
    
         #ifdef FEDFSTAR_DEBUG
           kern_printf("E%d",p);
	 #endif
      
         /* Pending activation: reactivate the thread!!! */
         lev->nact[p]--;
      
         /* see also FEDFSTAR_timer_deadline */
         kern_gettime(&temp);
      
         FEDFSTAR_internal_activate(lev,p, &temp);
      
         /* check if the deadline has already expired */
         temp = *iq_query_timespec(p, &lev->ready);
         if (TIMESPEC_A_LT_B(&temp, &schedule_time)) {
	   /* count the deadline miss */
           lev->dline_miss[p]++;
	   kern_event_delete(lev->deadline_timer[p]);
	   lev->deadline_timer[p] = NIL;
         }

       } else {

         #ifdef FEDFSTAR_DEBUG
           edfstar_printf("e%d",p);
         #endif
      
         /* the task has terminated his job before it consume the wcet. All OK! */
         if (lev->flag[p] & FEDFSTAR_FLAG_SPORADIC)
		proc_table[p].status = SLEEP;
	 else
		proc_table[p].status = FEDFSTAR_IDLE;
          
         if (lev->flag[p] & FEDFSTAR_FLAG_SPORADIC && lev->deadline_timer[p] != NIL) {
           kern_event_delete(lev->deadline_timer[p]); 
	   lev->deadline_timer[p] = NIL;
	 }
      
         /* and finally, a preemption check! */
         FEDFSTAR_check_preemption(lev);
      
         /* when the deadline timer fire, it recognize the situation and set
	   correctly all the stuffs (like reactivation, etc... ) */
       }

    #ifdef FEDFSTAR_DEBUG
      edfstar_printf(")");
    #endif

    TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,proc_table[p].context,proc_table[p].task_level);
    jet_update_endcycle(); /* Update the Jet data... */
    break;

  default:
    msg = (STD_command_message *)m;
  
#ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:MSG %d)",msg->command);
#endif   
 
    switch(msg->command) {
    case STD_SET_NEW_MODEL:
      /* if the FEDFSTAR_task_create is called, then the pclass must be a
	 valid pclass. */
      h=(HARD_TASK_MODEL *)(msg->param);
   
      /* now we know that m is a valid model */
      lev->wcet[p] = h->wcet;
      lev->period[p] = h->mit;

#ifdef FEDFSTAR_DEBUG      
      kern_printf("(EDF:NM p%d w%d m%d)", p, h->wcet, h->mit);
#endif       
      lev->flag[p] = 0;
      lev->deadline_timer[p] = -1;
      lev->dline_miss[p]     = 0;
      lev->wcet_miss[p]      = 0;
      lev->nact[p]           = 0;

      break;

    case STD_SET_NEW_LEVEL:
      
      lev->flag[p] |= FEDFSTAR_CHANGE_LEVEL;
      lev->new_level[p] = (int)(msg->param);

      break;

    case STD_ACTIVATE_TASK:
#ifdef FEDFSTAR_DEBUG
      kern_printf("(EDF:SA)");
#endif       
      /* Enable wcet check */
      proc_table[p].avail_time = lev->wcet[p];
      proc_table[p].wcet       = lev->wcet[p];
      proc_table[p].control &= ~CONTROL_CAP;
      
      FEDFSTAR_public_activate(l, p,NULL);
      
      break;

      
    }

    break;
      
  }
  return 0;
}

static void FEDFSTAR_public_end(LEVEL l, PID p)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  #ifdef FEDFSTAR_DEBUG
    edfstar_printf("(E:end)");
  #endif

  iq_extract(p,&lev->ready);
  level_table[ lev->scheduling_level ]->
    private_extract(lev->scheduling_level, p);


  /* we finally put the task in the ready queue */
  proc_table[p].status = FREE;
  
  iq_insertfirst(p,&freedesc);
  lev->activated=NIL;
  
  if (lev->deadline_timer[p] != -1) {
    kern_event_delete(lev->deadline_timer[p]);
    lev->deadline_timer[p] = NIL;
  }

  /* and finally, a preemption check! (it will also call guest_end) */
  FEDFSTAR_check_preemption(lev);
}

/* Guest Functions
   These functions manages a JOB_TASK_MODEL, that is used to put
   a guest task in the FEDFSTAR ready queue. */

static void FEDFSTAR_private_insert(LEVEL l, PID p, TASK_MODEL *m)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  JOB_TASK_MODEL *job;

  if (m->pclass != JOB_PCLASS || (m->level != 0 && m->level != l) ) {
    kern_raise(XINVALID_TASK, p);
    return;
  }

  job = (JOB_TASK_MODEL *)m;

  /* if the FEDFSTAR_guest_create is called, then the pclass must be a
     valid pclass. */

  *iq_query_timespec(p, &lev->ready) = job->deadline;
  
  lev->deadline_timer[p] = -1;
  lev->dline_miss[p]     = 0;
  lev->wcet_miss[p]      = 0;
  lev->nact[p]           = 0;

  if (job->noraiseexc)
    lev->flag[p] |= FEDFSTAR_FLAG_NORAISEEXC;
  else {
    lev->flag[p] &= ~FEDFSTAR_FLAG_NORAISEEXC;
    lev->deadline_timer[p] = kern_event_post(iq_query_timespec(p, &lev->ready),
                                             FEDFSTAR_timer_guest_deadline,
                                             (void *)p);
  }

  lev->period[p] = job->period;

  /* Insert task in the correct position */
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].status = FEDFSTAR_READY;

  /* check for preemption */
  FEDFSTAR_check_preemption(lev);

  /* there is no bandwidth guarantee at this level, it is performed
     by the level that inserts guest tasks... */
}

static void FEDFSTAR_private_dispatch(LEVEL l, PID p, int nostop)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  level_table[ lev->scheduling_level ]->
    private_dispatch(lev->scheduling_level,p,nostop);
}

static void FEDFSTAR_private_epilogue(LEVEL l, PID p)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  /* the task has been preempted. it returns into the ready queue... */
  level_table[ lev->scheduling_level ]->
    private_epilogue(lev->scheduling_level,p);

  proc_table[p].status = FEDFSTAR_READY;
}

static void FEDFSTAR_private_extract(LEVEL l, PID p)
{
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

#ifdef FEDFSTAR_DEBUG
  kern_printf("FEDFSTAR_guest_end: dline timer %d\n",lev->deadline_timer[p]);
#endif

  iq_extract(p, &lev->ready);

  /* we remove the deadline timer, because the slice is finished */
  if (lev->deadline_timer[p] != NIL) {
#ifdef FEDFSTAR_DEBUG
    kern_printf("FEDFSTAR_guest_end: dline timer %d\n",lev->deadline_timer[p]);
#endif
    kern_event_delete(lev->deadline_timer[p]);
    lev->deadline_timer[p] = NIL;
  }

  /* and finally, a preemption check! (it will also call guest_end() */
  FEDFSTAR_check_preemption(lev);
}

/* Registration functions */

/* Registration function:
    int flags                 the init flags ... see FEDFSTAR.h */

LEVEL FEDFSTAR_register_level(int master)
{
  LEVEL l;            /* the level that we register */
  FEDFSTAR_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

#ifdef FEDFSTAR_DEBUG
  printk("FEDFSTAR_register_level\n");
#endif

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(FEDFSTAR_level_des));

  lev = (FEDFSTAR_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.private_insert   = FEDFSTAR_private_insert;
  lev->l.private_extract  = FEDFSTAR_private_extract;
  lev->l.private_dispatch = FEDFSTAR_private_dispatch;
  lev->l.private_epilogue = FEDFSTAR_private_epilogue;

  lev->l.public_guarantee = NULL;
  lev->l.public_eligible  = FEDFSTAR_public_eligible;
  lev->l.public_create    = FEDFSTAR_public_create;
  lev->l.public_end       = FEDFSTAR_public_end;
  lev->l.public_dispatch  = FEDFSTAR_public_dispatch;
  lev->l.public_epilogue  = FEDFSTAR_public_epilogue;
  lev->l.public_activate  = FEDFSTAR_public_activate;
  lev->l.public_unblock   = FEDFSTAR_public_unblock;
  lev->l.public_block     = FEDFSTAR_public_block;
  lev->l.public_message   = FEDFSTAR_public_message;

  /* fill the FEDFSTAR descriptor part */
  for(i=0; i<MAX_PROC; i++) {
    lev->period[i]         = 0;
    lev->deadline_timer[i] = -1;
    lev->flag[i]           = 0;
    lev->dline_miss[i]     = 0;
    lev->wcet_miss[i]      = 0;
    lev->nact[i]           = 0;
    lev->budget[i]         = NIL;
    lev->new_level[i]      = -1;
    lev->ctime[i]          = 0;
    lev->cavg[i]           = 0; 
    lev->ccount[i]         = 0; 
  }

  lev->u_predicted=0;
  lev->u_p_avg=0;
  lev->nro=0;
  lev->sched_error_prev=0;
  lev->delta_old=0;
  lev->delta_new=0;

  iq_init(&lev->ready, NULL, IQUEUE_NO_PRIORITY);
  lev->activated = NIL;

  lev->scheduling_level = master;
  lev->cap_lev = NIL;
  NULL_TIMESPEC(&lev->cap_lasttime);

  return l;
}

int FEDFSTAR_get_dline_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  
  return lev->dline_miss[p];
}

int FEDFSTAR_get_wcet_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  return lev->wcet_miss[p];
}

int FEDFSTAR_get_nact(PID p)
{
  LEVEL l = proc_table[p].task_level;
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  return lev->nact[p];
}

int FEDFSTAR_reset_dline_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  lev->dline_miss[p] = 0;
  return 0;
}

int FEDFSTAR_reset_wcet_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  lev->wcet_miss[p] = 0;
  return 0;
}

int FEDFSTAR_setbudget(LEVEL l, PID p, int budget)
{

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  
  lev->budget[p] = budget;

  return 0;

}

int FEDFSTAR_getbudget(LEVEL l, PID p)
{

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  return lev->budget[p];

}

void FEDFSTAR_set_nopreemtive_current(LEVEL l) {
  
  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
 
  lev->flag[lev->activated]|=FEDFSTAR_FLAG_NOPREEMPT;
}

void FEDFSTAR_unset_nopreemtive_current(LEVEL l) {

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);

  lev->flag[lev->activated]&=~FEDFSTAR_FLAG_NOPREEMPT;
}
 
int FEDFSTAR_budget_has_thread(LEVEL l, int budget)
{

  FEDFSTAR_level_des *lev = (FEDFSTAR_level_des *)(level_table[l]);
  int i;

  for(i = 0; i< MAX_PROC; i++)
    if (lev->budget[i] == budget) return 1;

  return 0;

}
