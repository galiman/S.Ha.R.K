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
 CVS :        $Id: ps.c,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 This file contains the aperiodic server PS (Polling Server)

 when scheduling in background  the flags field has the PS_BACKGROUND bit set

 when scheduling a task because it is pointed by another task via shadows,
 the task have to be extracted from the wait queue or the master level. To
 check this we have to look at the activated field; it is != NIL if a task
 is inserted into the master level. Only a task at a time can be inserted
 into the master level.

 The capacity of the server must be updated
 - when scheduling a task normally
 - when scheduling a task because it is pointed by a shadow
 but not when scheduling in background.

 When a task is extracted from the system no scheduling has to be done
 until the task reenter into the system. to implement this, when a task
 is extracted we block the background scheduling (the scheduling with the
 master level is already blocked because the activated field is not
 reset to NIL) using the PS_BACKGROUNDBLOCK bit.

 nact[p] is -1 if the task set the activations to SKIP, >= 0 otherwise

 Note that if the period event fires and there aren't any task to schedule,
 the server capacity is set to 0. This is correct, but there is a subtle
 variant: the server capacity may be set to 0 later because if at the
 period end the running task have priority > than the server, the capacity
 may be set to zero the first time the server become the highest priority
 running task and there aren't task to serve. The second implementation
 is more efficient but more complicated, because normally we don't know the
 priority of the running task.

 An implementation can be done in this way: when there are not task to
 schedule, we does not set the lev->activated field to nil, but to a "dummy"
 task that is inserted into the master level queue.
 When the master level scheduler try to schedule the "dummy" task (this is
 the situation in witch there are not task to schedule and the PS is the
 task with greater priority), it calls the PS_task_eligible, that set the
 server capacity to 0, remove the dummy task from the queue with a guest_end
 and ask to reschedule.

 Because this implementation is more complex than the first, I don't
 implement it... see (*), near line 169, 497 and 524


 Read PS.h for further details.

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


#include <ps/ps/ps.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/*+ Status used in the level +*/
#define PS_WAIT          APER_STATUS_BASE    /*+ waiting the service   +*/

/*+ the level redefinition for the Total Bandwidth Server level +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  /* The wcet are stored in the task descriptor's priority
     field, so no other fields are needed                      */

  int nact[MAX_PROC]; /*+ number of pending activations       +*/

  struct timespec lastdline; /*+ the last deadline assigned to
                                 a PS task                    +*/

  int Cs;          /*+ server capacity                        +*/
  int availCs;     /*+ server avail time                      +*/

  IQUEUE wait;      /*+ the wait queue of the PS              +*/
  PID activated;   /*+ the task inserted in another queue     +*/

  int flags;       /*+ the init flags...                      +*/

  bandwidth_t U;   /*+ the used bandwidth by the server       +*/
  int period;

  LEVEL scheduling_level;

} PS_level_des;

/* This static function activates the task pointed by lev->activated) */
static __inline__ void PS_activation(PS_level_des *lev)
{
    PID p;                     /* for readableness    */
    JOB_TASK_MODEL j;          /* the guest model     */
    LEVEL m;                   /* the master level... only for readableness*/

    p = lev->activated;
    m = lev->scheduling_level;
    job_task_default_model(j,lev->lastdline);
    job_task_def_period(j,lev->period);
    level_table[m]->private_insert(m,p,(TASK_MODEL *)&j);
//    kern_printf("(%d %d)",lev->lastdline.tv_sec,lev->lastdline.tv_nsec);
}

static void PS_deadline_timer(void *a)
{
  PS_level_des *lev = (PS_level_des *)(level_table[(LEVEL)a]);

  ADDUSEC2TIMESPEC(lev->period, &lev->lastdline);

//  kern_printf("(%d:%d %d)",lev->lastdline.tv_sec,lev->lastdline.tv_nsec, lev->period);
  if (lev->availCs >= 0)
    lev->availCs = lev->Cs;
  else
    lev->availCs += lev->Cs;

  /* availCs may be <0 because a task executed via a shadow fo many time
     lev->activated == NIL only if the prec task was finished and there
     was not any other task to be put in the ready queue
     ... we are now activating the next task */
  if (lev->availCs > 0 && lev->activated == NIL) {
      if (iq_query_first(&lev->wait) != NIL) {
        lev->activated = iq_getfirst(&lev->wait);
        PS_activation(lev);
        event_need_reschedule();
      }
      else
        lev->availCs = 0; /* see note (*) at the begin of the file */
  }

  kern_event_post(&lev->lastdline, PS_deadline_timer, a);
//  kern_printf("!");
}

static PID PS_public_schedulerbackground(LEVEL l)
{
  /* the PS catch the background time to exec aperiodic activities */
  PS_level_des *lev = (PS_level_des *)(level_table[l]);

  lev->flags |= PS_BACKGROUND;

  if (lev->flags & PS_BACKGROUND_BLOCK)
    return NIL;
  else
    return iq_query_first(&lev->wait);
}

/* The on-line guarantee is enabled only if the appropriate flag is set... */
static int PS_public_guaranteeEDF(LEVEL l, bandwidth_t *freebandwidth)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

static int PS_public_guaranteeRM(LEVEL l, bandwidth_t *freebandwidth)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);

  if (*freebandwidth > lev->U + RM_MINFREEBANDWIDTH) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

static int PS_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);
  SOFT_TASK_MODEL *s;

  if (m->pclass != SOFT_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  s = (SOFT_TASK_MODEL *)m;
  if (s->periodicity != APERIODIC) return -1;
 
  s = (SOFT_TASK_MODEL *)m;

  if (s->arrivals == SAVE_ARRIVALS)
    lev->nact[p] = 0;
  else
    lev->nact[p] = -1;

  return 0; /* OK, also if the task cannot be guaranteed... */
}

static void PS_public_dispatch(LEVEL l, PID p, int nostop)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);
  struct timespec ty;

//  if (nostop) kern_printf("NOSTOP!!!!!!!!!!!!");
  /* there is at least one task ready inserted in an EDF or similar
     level note that we can't check the status because the scheduler set it
     to exe before calling task_dispatch. we have to check
     lev->activated != p instead */
  if (lev->activated != p) {
    iq_extract(p, &lev->wait);
    //kern_printf("#%d#",p);
  }
  else {
    //if (nostop) kern_printf("(gd status=%d)",proc_table[p].status);
    level_table[ lev->scheduling_level ]->
      private_dispatch(lev->scheduling_level,p,nostop);
  }

  /* set the capacity timer */
  if (!nostop) {
    TIMESPEC_ASSIGN(&ty, &schedule_time);
    ADDUSEC2TIMESPEC(lev->availCs,&ty);
    cap_timer = kern_event_post(&ty, capacity_timer, NULL);
  }

//  kern_printf("(disp %d %d)",ty.tv_sec, ty.tv_nsec);
}

static void PS_public_epilogue(LEVEL l, PID p)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);
  struct timespec ty;
  TIME tx;

  /* update the server capacity */
  if (lev->flags & PS_BACKGROUND)
    lev->flags &= ~PS_BACKGROUND;
  else {
    SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
    tx = TIMESPEC2USEC(&ty);
    lev->availCs -= tx;
  }

//  kern_printf("(epil %d %d)",lev->availCs, proc_table[p].avail_time);

  /* check if the server capacity is finished... */
  if (lev->availCs < 0) {
//    kern_printf("(epil Cs%d %d:%d act%d p%d)",
//              lev->availCs,proc_table[p].timespec_priority.tv_sec,
//              proc_table[p].timespec_priority.tv_nsec,
//              lev->activated,p);
    /* the server slice has finished... do the task_end!!!
       a first version of the module used the task_endcycle, but it was
       not conceptually correct because the task didn't stop because it
       finished all the work but because the server didn't have budget!
       So, if the task_endcycle is called, the task remain into the
       master level, and we can't wake him up if, for example, another
       task point the shadow to it!!!*/
    if (lev->activated == p)
      level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level,p);
    iq_insertfirst(p, &lev->wait);
    proc_table[p].status = PS_WAIT;
    lev->activated = NIL;
  }
  else
    /* the task has been preempted. it returns into the ready queue or to the
       wait queue by calling the guest_epilogue... */
    if (lev->activated == p) {//kern_printf("Û1");
      level_table[ lev->scheduling_level ]->
        private_epilogue(lev->scheduling_level,p);
    } else { //kern_printf("Û2");
      iq_insertfirst(p, &lev->wait);
      proc_table[p].status = PS_WAIT;
    }
}

static void PS_public_activate(LEVEL l, PID p, struct timespec *t)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);

  if (lev->activated == p || proc_table[p].status == PS_WAIT) {
    if (lev->nact[p] != -1)
      lev->nact[p]++;
  }
  else if (proc_table[p].status == SLEEP) {

    if (lev->activated == NIL && lev->availCs > 0) {
      lev->activated = p;
      PS_activation(lev);
    }
    else {
      iq_insertlast(p, &lev->wait);
      proc_table[p].status = PS_WAIT;
    }
  }
  else
  {  kern_printf("PS_REJ%d %d %d %d ",p, proc_table[p].status, lev->activated, lev->wait.first);
     return; }

}

static void PS_public_unblock(LEVEL l, PID p)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);

  lev->flags &= ~PS_BACKGROUND_BLOCK;

  lev->activated = -1;

  /* when we reinsert the task into the system, the server capacity
     is always 0 because nobody executes with the PS before... */
  iq_insertfirst(p, &lev->wait);
  proc_table[p].status = PS_WAIT;
}

static void PS_public_block(LEVEL l, PID p)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);

  /* update the server capacity */
  lev->availCs = 0;

  lev->flags |= PS_BACKGROUND_BLOCK;

  if (lev->activated == p)
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level,p);
}

static int PS_public_message(LEVEL l, PID p, void *m)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);
  struct timespec ty;
  TIME tx;

  /* update the server capacity */
  if (lev->flags & PS_BACKGROUND)
    lev->flags &= ~PS_BACKGROUND;
  else {
    SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
    tx = TIMESPEC2USEC(&ty);
    lev->availCs -= tx;
  }

  if (lev->activated == p)
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level,p);
  else
    iq_extract(p, &lev->wait);

  if (lev->nact[p] > 0)
  {
    lev->nact[p]--;
    iq_insertlast(p, &lev->wait);
    proc_table[p].status = PS_WAIT;
  }
  else
    proc_table[p].status = SLEEP;
  
  lev->activated = iq_getfirst(&lev->wait);
  if (lev->activated == NIL)
    lev->availCs = 0; /* see note (*) at the begin of the file */
  else
    PS_activation(lev);

  jet_update_endcycle(); /* Update the Jet data... */
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l);

  return 0;
}

static void PS_public_end(LEVEL l, PID p)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);
  struct timespec ty;
  TIME tx;

  /* update the server capacity */
  if (lev->flags & PS_BACKGROUND)
    lev->flags &= ~PS_BACKGROUND;
  else {
    SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
    tx = TIMESPEC2USEC(&ty);
    lev->availCs -= tx;
  }

  if (lev->activated == p)
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level,p);

  proc_table[p].status = FREE;
  iq_insertfirst(p,&freedesc);

  lev->activated = iq_getfirst(&lev->wait);
  if (lev->activated == NIL)
    lev->availCs = 0; /* see note (*) at the begin of the file */
  else
    PS_activation(lev);
}

/* Registration functions */


/*+ This init function install the PS deadline timer
    +*/
static void PS_dline_install(void *l)
{
  PS_level_des *lev = (PS_level_des *)(level_table[(LEVEL)l]);

  kern_gettime(&lev->lastdline);
  ADDUSEC2TIMESPEC(lev->period, &lev->lastdline);

  kern_event_post(&lev->lastdline, PS_deadline_timer, l);
}



/*+ Registration function:
    int flags                 the init flags ... see PS.h +*/
LEVEL PS_register_level(int flags, LEVEL master, int Cs, int per)
{
  LEVEL l;            /* the level that we register */
  PS_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

  printk("PS_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(PS_level_des));

  lev = (PS_level_des *)level_table[l];

  printk("    lev=%d\n",(int)lev);

  /* fill the standard descriptor */

  if (flags & PS_ENABLE_BACKGROUND)
    lev->l.public_scheduler = PS_public_schedulerbackground;

  if (flags & PS_ENABLE_GUARANTEE_EDF)
    lev->l.public_guarantee = PS_public_guaranteeEDF;
  else if (flags & PS_ENABLE_GUARANTEE_RM)
    lev->l.public_guarantee = PS_public_guaranteeRM;
  else
    lev->l.public_guarantee = NULL;

  lev->l.public_create    = PS_public_create;
  lev->l.public_end       = PS_public_end;
  lev->l.public_dispatch  = PS_public_dispatch;
  lev->l.public_epilogue  = PS_public_epilogue;
  lev->l.public_activate  = PS_public_activate;
  lev->l.public_unblock   = PS_public_unblock;
  lev->l.public_block     = PS_public_block;
  lev->l.public_message   = PS_public_message;

  /* fill the PS descriptor part */

  for (i=0; i<MAX_PROC; i++)
     lev->nact[i] = -1;

  lev->Cs = Cs;
  lev->availCs = 0;

  lev->period = per;

  iq_init(&lev->wait, &freedesc, 0);
  lev->activated = NIL;

  lev->U = (MAX_BANDWIDTH / per) * Cs;

  lev->scheduling_level = master;

  lev->flags = flags & 0x07;

  sys_atrunlevel(PS_dline_install,(void *) l, RUNLEVEL_INIT);

  return l;
}

bandwidth_t PS_usedbandwidth(LEVEL l)
{
  PS_level_des *lev = (PS_level_des *)(level_table[l]);
  
  return lev->U;
}

