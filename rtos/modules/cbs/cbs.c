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
 CVS :        $Id: cbs.c,v 1.1 2005/02/25 10:53:02 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:02 $
 ------------

 This file contains the aperiodic server CBS (Total Bandwidth Server)

 Read CBS.h for further details.

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


#include <cbs/cbs/cbs.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/*+ 4 debug purposes +*/
#undef CBS_TEST
#undef CBS_COUNTER

#ifdef TESTG
#include "drivers/glib.h"
TIME x,oldx;
extern TIME starttime;
#endif


/*+ Status used in the level +*/
#define CBS_IDLE          APER_STATUS_BASE   /*+ waiting the activation +*/
#define CBS_ZOMBIE        APER_STATUS_BASE+1 /*+ waiting the period end +*/

/*+ task flags +*/
#define CBS_SAVE_ARRIVALS 1
#define CBS_APERIODIC     2
#define CBS_SLEEP	  4

/*+ the level redefinition for the Total Bandwidth Server level +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  /* The wcet are stored in the task descriptor, but we need
     an array for the deadlines. We can't use the timespec_priority
     field because it is used by the master level!!!...
     Notice that however the use of the timespec_priority field
     does not cause any problem...                     */

  struct timespec cbs_dline[MAX_PROC]; /*+ CBS deadlines      +*/

  TIME period[MAX_PROC]; /*+ CBS activation period            +*/

  struct timespec reactivation_time[MAX_PROC];
        /*+ the time at witch  the reactivation timer is post +*/
  int reactivation_timer[MAX_PROC];
                                   /*+ the recativation timer +*/

  int nact[MAX_PROC]; /*+ number of pending activations       +*/

  BYTE flag[MAX_PROC]; /*+ task flags                         +*/

  int flags;       /*+ the init flags...                      +*/

  bandwidth_t U;   /*+ the used bandwidth by the server       +*/

  LEVEL scheduling_level;

} CBS_level_des;

#ifdef CBS_COUNTER
int cbs_counter=0;
int cbs_counter2=0;
#endif


static void CBS_activation(CBS_level_des *lev,
                           PID p,
                           struct timespec *acttime)
{
  JOB_TASK_MODEL job;

  /* we have to check if the deadline and the wcet are correct before
     activating a new task or an old task... */

  /* check 1: if the deadline is before than the actual scheduling time */
  /* check 2: if ( avail_time >= (cbs_dline - acttime)* (wcet/period) )
     (rule 7 in the CBS article!) */

  /* CHANGES by Anton Cervin 2004-06-23:
     a) only check: if ( acttime + (avail_time * period) / wcet > cbs_dline )
     b) without (long long) in the computation we can have an overflow!!   */
 
  TIME t;
  struct timespec t2;

  t = ((long long)lev->period[p] * (long long)proc_table[p].avail_time)
    / (long long)proc_table[p].wcet;
  t2 = *acttime;
  ADDUSEC2TIMESPEC(t, &t2);

  if (TIMESPEC_A_GT_B(&t2, &lev->cbs_dline[p])) {

    /* we modify the deadline ... */
    TIMESPEC_ASSIGN(&lev->cbs_dline[p], acttime);
    ADDUSEC2TIMESPEC(lev->period[p], &lev->cbs_dline[p]);

    /* and the capacity */
    proc_table[p].avail_time = proc_table[p].wcet;
  }

#ifdef TESTG
      if (starttime && p == 3) {
      oldx = x;
      x = ((lev->cbs_dline[p].tv_sec*1000000+lev->cbs_dline[p].tv_nsec/1000)/5000 - starttime) + 20;
//      kern_printf("(a%d)",lev->cbs_dline[p].tv_sec*1000000+lev->cbs_dline[p].tv_nsec/1000);
      if (oldx > x) kern_raise(XUNSPECIFIED_EXCEPTION, exec_shadow);
      if (x<640)
        grx_plot(x, 15, 8);
      }
#endif

  /* and, finally, we reinsert the task in the master level */
  job_task_default_model(job, lev->cbs_dline[p]);
  job_task_def_noexc(job);
  level_table[ lev->scheduling_level ]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);
}

static void CBS_avail_time_check(CBS_level_des *lev, PID p)
{
  /* there is a while because if the wcet is << than the system tick
     we need to postpone the deadline many times */
  while (proc_table[p].avail_time <= 0) {
    ADDUSEC2TIMESPEC(lev->period[p], &lev->cbs_dline[p]);
    proc_table[p].avail_time += proc_table[p].wcet;

#ifdef TESTG
    if (starttime && p == 3) {
    oldx = x;
    x = ((lev->cbs_dline[p].tv_sec*1000000+lev->cbs_dline[p].tv_nsec/1000)/5000 - starttime) + 20;
//      kern_printf("(e%d avail%d)",lev->cbs_dline[p].tv_sec*1000000+lev->cbs_dline[p].tv_nsec/1000,proc_table[p].avail_time);
    if (oldx > x) kern_raise(XUNSPECIFIED_EXCEPTION, exec_shadow);
    if (x<640)
      grx_plot(x, 15, 2);
    }
#endif
  }
}


/* this is the periodic reactivation of the task... it is posted only
   if the task is a periodic task */
static void CBS_timer_reactivate(void *par)
{
  PID p = (PID) par;
  CBS_level_des *lev;

  lev = (CBS_level_des *)level_table[proc_table[p].task_level];

  #ifdef CBS_COUNTER
    if (p==5) cbs_counter++;
  #endif

  if (lev->flag[p] & CBS_SLEEP && proc_table[p].status == CBS_IDLE) {
    proc_table[p].status = SLEEP;
    proc_table[p].avail_time = proc_table[p].wcet;
    NULL_TIMESPEC(&lev->cbs_dline[p]);
    return;
  }

  if (proc_table[p].status == CBS_IDLE) {
    /* the task has finished the current activation and must be
       reactivated */
    CBS_activation(lev,p,&lev->reactivation_time[p]);

    event_need_reschedule();
  }
  else if (lev->flag[p] & CBS_SAVE_ARRIVALS)
    /* the task has not completed the current activation, so we save
       the activation incrementing nact... */
    lev->nact[p]++;

  /* repost the event at the next period end... */
  ADDUSEC2TIMESPEC(lev->period[p], &lev->reactivation_time[p]);
  lev->reactivation_timer[p] = kern_event_post(&lev->reactivation_time[p],
                                               CBS_timer_reactivate,
                                               (void *)p);
  #ifdef CBS_COUNTER
    if (p==5) cbs_counter2++;
  #endif
  /* tracer stuff */
  TRACER_LOGEVENT(FTrace_EVT_task_timer,(unsigned short int)proc_table[p].context,(unsigned int)proc_table[p].task_level);

}

/*+ this function is called when a killed or ended task reach the
    period end +*/
static void CBS_timer_zombie(void *par)
{
  PID p = (PID) par;
  CBS_level_des *lev;

  lev = (CBS_level_des *)level_table[proc_table[p].task_level];

  /* we finally put the task in the ready queue */
  proc_table[p].status = FREE;
  iq_insertfirst(p,&freedesc);

  /* and free the allocated bandwidth */
  lev->U -= (MAX_BANDWIDTH/lev->period[p]) * proc_table[p].wcet;

}


/* The on-line guarantee is enabled only if the appropriate flag is set... */
static int CBS_public_guarantee(LEVEL l, bandwidth_t *freebandwidth)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

static int CBS_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);
  SOFT_TASK_MODEL *soft;

  if (m->pclass != SOFT_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  soft = (SOFT_TASK_MODEL *)m;
  if (!(soft->met && soft->period)) return -1;

  soft = (SOFT_TASK_MODEL *)m;

  if (lev->flags & CBS_ENABLE_GUARANTEE) {
    bandwidth_t b;
    b = (MAX_BANDWIDTH / soft->period) * soft->met;

    /* really update lev->U, checking an overflow... */
    if (MAX_BANDWIDTH - lev->U > b)
      lev->U += b;
    else
      return -1;
  }

  /* Enable wcet check */
  proc_table[p].avail_time = soft->met;
  proc_table[p].wcet       = soft->met;
  proc_table[p].control   |= CONTROL_CAP;

  lev->nact[p] = 0;
  lev->period[p] = soft->period;
  NULL_TIMESPEC(&lev->cbs_dline[p]);

  if (soft->periodicity == APERIODIC)
    lev->flag[p] = CBS_APERIODIC;
  else
    lev->flag[p] = 0;

  if (soft->arrivals == SAVE_ARRIVALS)
    lev->flag[p] |= CBS_SAVE_ARRIVALS;

  return 0; /* OK, also if the task cannot be guaranteed... */
}

static void CBS_public_detach(LEVEL l, PID p)
{
  /* the CBS level doesn't introduce any dinamic allocated new field.
     we have only to decrement the allocated bandwidth */

  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  if (lev->flags & CBS_ENABLE_GUARANTEE) {
    lev->U -= (MAX_BANDWIDTH / lev->period[p]) * proc_table[p].wcet;
  }
}

static int CBS_public_eligible(LEVEL l, PID p)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);
  JOB_TASK_MODEL job;

  /* we have to check if the deadline and the wcet are correct...
     if the CBS level schedules in background with respect to others
     levels, there can be the case in witch a task is scheduled by
     schedule_time > CBS_deadline; in this case (not covered in the
     article because if there is only the standard scheduling policy
     this never apply) we reassign the deadline */

  if ( TIMESPEC_A_LT_B(&lev->cbs_dline[p], &schedule_time) ) {
    /* we kill the current activation */
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level, p);

    /* we modify the deadline ... */
    TIMESPEC_ASSIGN(&lev->cbs_dline[p], &schedule_time);
    ADDUSEC2TIMESPEC(lev->period[p], &lev->cbs_dline[p]);

    /* and the capacity */
    proc_table[p].avail_time = proc_table[p].wcet;
  
    /* and, finally, we reinsert the task in the master level */
    job_task_default_model(job, lev->cbs_dline[p]);
    job_task_def_noexc(job);
    level_table[ lev->scheduling_level ]->
      private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);

    return -1;
  }

  return 0;
}

static void CBS_public_dispatch(LEVEL l, PID p, int nostop)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);
  level_table[ lev->scheduling_level ]->
    private_dispatch(lev->scheduling_level,p,nostop);
}

static void CBS_public_epilogue(LEVEL l, PID p)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);
  JOB_TASK_MODEL job;

  /* check if the wcet is finished... */
  if ( proc_table[p].avail_time <= 0) {
    /* we kill the current activation */
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level, p);

    /* we modify the deadline according to rule 4 ... */
    CBS_avail_time_check(lev, p);

    /* and, finally, we reinsert the task in the master level */
    job_task_default_model(job, lev->cbs_dline[p]);
    job_task_def_noexc(job);
    level_table[ lev->scheduling_level ]->
      private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);
//    kern_printf("epil : dl %d per %d p %d |\n",
//              lev->cbs_dline[p].tv_nsec/1000,lev->period[p],p);

  }
  else
    /* the task has been preempted. it returns into the ready queue by
       calling the guest_epilogue... */
    level_table[ lev->scheduling_level ]->
      private_epilogue(lev->scheduling_level,p);
}

static void CBS_public_activate(LEVEL l, PID p, struct timespec *t)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  if (lev->flag[p] & CBS_SLEEP) {
    lev->flag[p] &= ~CBS_SLEEP;
    if (proc_table[p].status != SLEEP) return;
  }

  /* save activation (only if needed... */
  if (proc_table[p].status != SLEEP) {
    if (lev->flag[p] & CBS_SAVE_ARRIVALS)
      lev->nact[p]++;
    return;
  }

  CBS_activation(lev, p, t);

  /* Set the reactivation timer */
  if (!(lev->flag[p] & CBS_APERIODIC))
  {
    /* we cannot use the deadline computed by CBS_activation because
       the deadline may be != from actual_time + period
       (if we call the task_activate after a task_sleep, and the
       deadline was postponed a lot...) */
    TIMESPEC_ASSIGN(&lev->reactivation_time[p], t);
    ADDUSEC2TIMESPEC(lev->period[p], &lev->reactivation_time[p]);
//    TIMESPEC_ASSIGN(&lev->reactivation_time[p], &lev->cbs_dline[p]);
    lev->reactivation_timer[p] = kern_event_post(&lev->reactivation_time[p],
                                                 CBS_timer_reactivate,
                                                 (void *)p);
    #ifdef CBS_COUNTER
      if (p==5) cbs_counter2++;
    #endif
  }
//  kern_printf("act : %d %d |",lev->cbs_dline[p].tv_nsec/1000,p);
}

static void CBS_public_unblock(LEVEL l, PID p)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);
  struct timespec acttime;

  kern_gettime(&acttime);

  CBS_activation(lev,p,&acttime);
}

static void CBS_public_block(LEVEL l, PID p)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  /* check if the wcet is finished... */
  CBS_avail_time_check(lev, p);

  level_table[ lev->scheduling_level ]->
    private_extract(lev->scheduling_level,p);
}

static int CBS_public_message(LEVEL l, PID p, void *m)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  switch((long)(m)) {

    case (long)(NULL):

      /* check if the wcet is finished... */
      CBS_avail_time_check(lev, p);

      if (lev->nact[p]) {
        /* continue!!!! */
        lev->nact[p]--;
        level_table[ lev->scheduling_level ]->
          private_epilogue(lev->scheduling_level,p);
      } else {
        level_table[ lev->scheduling_level ]->
          private_extract(lev->scheduling_level,p);

        if (lev->flag[p] & CBS_APERIODIC)
          proc_table[p].status = SLEEP;
        else  /* the task is soft_periodic */
          proc_table[p].status = CBS_IDLE;
      }

      jet_update_endcycle(); /* Update the Jet data... */
      TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l); 

      break;

    case 1:

      lev->flag[p] |= CBS_SLEEP;
      TRACER_LOGEVENT(FTrace_EVT_task_disable,(unsigned short int)proc_table[p].context,(unsigned int)l);

      break;

  }

  return 0;

}

static void CBS_public_end(LEVEL l, PID p)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  /* check if the wcet is finished... */
  CBS_avail_time_check(lev, p);

  level_table[ lev->scheduling_level ]->
    private_extract(lev->scheduling_level,p);

  /* we delete the reactivation timer */
  if (!(lev->flag[p] & CBS_APERIODIC)) {
    kern_event_delete(lev->reactivation_timer[p]);
    lev->reactivation_timer[p] = -1;
  }

  /* Finally, we post the zombie event. when the end period is reached,
     the task descriptor and banwidth are freed */
  proc_table[p].status = CBS_ZOMBIE;
  lev->reactivation_timer[p] = kern_event_post(&lev->cbs_dline[p],
                                               CBS_timer_zombie,
                                               (void *)p);
}

/* Registration functions */

/*+ Registration function:
    int flags                 the init flags ... see CBS.h +*/
LEVEL CBS_register_level(int flags, LEVEL master)
{
  LEVEL l;            /* the level that we register */
  CBS_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

  printk("CBS_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(CBS_level_des));

  lev = (CBS_level_des *)level_table[l];

  /* fill the standard descriptor */
  if (flags & CBS_ENABLE_GUARANTEE)
    lev->l.public_guarantee = CBS_public_guarantee;
  else
    lev->l.public_guarantee = NULL;
  lev->l.public_create    = CBS_public_create;
  lev->l.public_detach    = CBS_public_detach;
  lev->l.public_end       = CBS_public_end;
  lev->l.public_eligible  = CBS_public_eligible;
  lev->l.public_dispatch  = CBS_public_dispatch;
  lev->l.public_epilogue  = CBS_public_epilogue;
  lev->l.public_activate  = CBS_public_activate;
  lev->l.public_unblock   = CBS_public_unblock;
  lev->l.public_block     = CBS_public_block;
  lev->l.public_message   = CBS_public_message;

  /* fill the CBS descriptor part */
  for (i=0; i<MAX_PROC; i++) {
     NULL_TIMESPEC(&lev->cbs_dline[i]);
     lev->period[i] = 0;
     NULL_TIMESPEC(&lev->reactivation_time[i]);
     lev->reactivation_timer[i] = -1;
     lev->nact[i] = 0;
     lev->flag[i] = 0;
  }


  lev->U = 0;

  lev->scheduling_level = master;

  lev->flags = flags;

  return l;
}

bandwidth_t CBS_usedbandwidth(LEVEL l)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  return lev->U;
}

int CBS_get_nact(LEVEL l, PID p)
{
  CBS_level_des *lev = (CBS_level_des *)(level_table[l]);

  return lev->nact[p];
}

