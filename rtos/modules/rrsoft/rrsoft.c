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
 CVS :        $Id: rrsoft.c,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains the scheduling module RRSOFT (Round Robin)

 Read rrsoft.h for further details.

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
 * but WITHOUT ANY WARRSOFTANTY; without even the implied waRRSOFTanty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <rrsoft/rrsoft/rrsoft.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/*+ Status used in the level +*/
#define RRSOFT_READY   MODULE_STATUS_BASE
#define RRSOFT_IDLE    MODULE_STATUS_BASE+2

/*+ the level redefinition for the Round Robin level +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  int nact[MAX_PROC]; /*+ number of pending activations       +*/

  IQUEUE ready;    /*+ the ready queue                        +*/

  int slice;       /*+ the level's time slice                 +*/

  TIME period[MAX_PROC]; /*+  activation period               +*/

  struct timespec reactivation_time[MAX_PROC];
        /*+ the time at witch  the reactivation timer is post +*/
  int reactivation_timer[MAX_PROC];
                                   /*+ the recativation timer +*/

  BYTE periodic[MAX_PROC];


  struct multiboot_info *multiboot; /*+ used if the level have to insert
                                        the main task +*/

  BYTE models;      /*+ Task Model that the Module can Handle +*/
} RRSOFT_level_des;


/* this is the periodic reactivation of the task... it is posted only
   if the task is a periodic task */
static void RRSOFT_timer_reactivate(void *par)
{
  PID p = (PID) par;
  RRSOFT_level_des *lev;
//  kern_printf("react");

  lev = (RRSOFT_level_des *)level_table[proc_table[p].task_level];

  if (proc_table[p].status == RRSOFT_IDLE) {
    /* the task has finished the current activation and must be
       reactivated */
    proc_table[p].status = RRSOFT_READY;
    iq_insertlast(p,&lev->ready);

    event_need_reschedule();
  }
  else if (lev->nact[p] >= 0)
    /* the task has not completed the current activation, so we save
       the activation incrementing nact... */
    lev->nact[p]++;

  /* repost the event at the next period end... */
  ADDUSEC2TIMESPEC(lev->period[p], &lev->reactivation_time[p]);
  lev->reactivation_timer[p] = kern_event_post(&lev->reactivation_time[p],
                                               RRSOFT_timer_reactivate,
                                               (void *)p);
}

/* This is not efficient but very fair :-)
   The need of all this stuff is because if a task execute a long time
   due to (shadow!) priority inheritance, then the task shall go to the
   tail of the queue many times... */
static PID RRSOFT_public_scheduler(LEVEL l)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  PID p;

  for (;;) {
    p = iq_query_first(&lev->ready);
    if (p == -1)
      return p;
//{kern_printf("(s%d)",p);      return p;}

//    kern_printf("(p=%d l=%d avail=%d wcet =%d)\n",p,l,proc_table[p].avail_time, proc_table[p].wcet);
    if (proc_table[p].avail_time <= 0) {
      proc_table[p].avail_time += proc_table[p].wcet;
      iq_extract(p,&lev->ready);
      iq_insertlast(p,&lev->ready);
    }
    else
//{kern_printf("(s%d)",p);      return p;}
      return p;

  }
}

static int RRSOFT_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

//  kern_printf("create %d mod %d\n",p,m->pclass);
  /* the task state is set at SLEEP by the general task_create
     the only thing to set remains the capacity stuffs that are set
     to the values passed in the model... */

  if (m->pclass==NRT_PCLASS  && !(lev->models & RRSOFT_ONLY_NRT) ) return -1;
  if (m->pclass==SOFT_PCLASS && !(lev->models & RRSOFT_ONLY_SOFT) ) return -1;
  if (m->pclass==HARD_PCLASS && !(lev->models & RRSOFT_ONLY_HARD) ) return -1;
  if (m->level != 0 && m->level != l) return -1;

  /* I used the wcet field because using wcet can account if a task
     consume more than the timeslice... */

  if (lev->models & RRSOFT_ONLY_NRT &&
      (m->pclass == NRT_PCLASS || m->pclass == (NRT_PCLASS | l))) {
    NRT_TASK_MODEL *nrt = (NRT_TASK_MODEL *)m;

//    kern_printf("nrt");
    if (nrt->slice) {
      proc_table[p].avail_time = nrt->slice;
      proc_table[p].wcet       = nrt->slice;
    }
    else {
      proc_table[p].avail_time = lev->slice;
      proc_table[p].wcet       = lev->slice;
    }
    proc_table[p].control   |= CONTROL_CAP;
  
    if (nrt->arrivals == SAVE_ARRIVALS)
      lev->nact[p] = 0;
    else
      lev->nact[p] = -1;

    lev->periodic[p] = 0;
    lev->period[p] = 0;
  }
  else if (lev->models & RRSOFT_ONLY_SOFT &&
           (m->pclass == SOFT_PCLASS || m->pclass == (SOFT_PCLASS | l))) {
    SOFT_TASK_MODEL *soft = (SOFT_TASK_MODEL *)m;
//    kern_printf("soft");
    proc_table[p].avail_time = lev->slice;
    proc_table[p].wcet       = lev->slice;
    proc_table[p].control   |= CONTROL_CAP;
  
    if (soft->arrivals == SAVE_ARRIVALS)
      lev->nact[p] = 0;
    else
      lev->nact[p] = -1;

    if (soft->periodicity == PERIODIC) {
      lev->periodic[p] = 1;
      lev->period[p] = soft->period;
    }
  }
  else if (lev->models & RRSOFT_ONLY_HARD &&
           (m->pclass == HARD_PCLASS || m->pclass == (HARD_PCLASS | l))) {
    HARD_TASK_MODEL *hard = (HARD_TASK_MODEL *)m;
//    kern_printf("hard");
    proc_table[p].avail_time = lev->slice;
    proc_table[p].wcet       = lev->slice;
    proc_table[p].control   |= CONTROL_CAP;
  
    lev->nact[p] = 0;

    if (hard->periodicity == PERIODIC) {
      lev->periodic[p] = 1;
      lev->period[p] = hard->mit;
    }
  }

  return 0; /* OK */
}

static void RRSOFT_public_dispatch(LEVEL l, PID p, int nostop)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  /* the task state is set EXE by the scheduler()
     we extract the task from the ready queue
     NB: we can't assume that p is the first task in the queue!!! */
  iq_extract(p, &lev->ready);
}

static void RRSOFT_public_epilogue(LEVEL l, PID p)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  /* check if the slice is finished and insert the task in the coRRSOFTect
     qqueue position */
  if (proc_table[p].avail_time <= 0) {
    proc_table[p].avail_time += proc_table[p].wcet;
    iq_insertlast(p,&lev->ready);
  }
  else
    /* curr is >0, so the running task have to run for another cuRRSOFT usec */
    iq_insertfirst(p,&lev->ready);

  proc_table[p].status = RRSOFT_READY;
}

static void RRSOFT_public_activate(LEVEL l, PID p, struct timespec *t)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  /* Test if we are trying to activate a non sleeping task    */
  /* save activation (only if needed... */
  if (proc_table[p].status != SLEEP && proc_table[p].status != RRSOFT_IDLE) {
    if (lev->nact[p] != -1)
      lev->nact[p]++;
    return;
  }

  /* Insert task in the correct position */
  proc_table[p].status = RRSOFT_READY;
  iq_insertlast(p,&lev->ready);

  /* Set the reactivation timer */
  if (lev->periodic[p])
  {
    kern_gettime(&lev->reactivation_time[p]);
    ADDUSEC2TIMESPEC(lev->period[p], &lev->reactivation_time[p]);
    lev->reactivation_timer[p] = kern_event_post(&lev->reactivation_time[p],
                                                 RRSOFT_timer_reactivate,
                                                 (void *)p);
  }
}

static void RRSOFT_public_unblock(LEVEL l, PID p)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  /* Similar to RRSOFT_task_activate, but we don't check in what state
     the task is */

  /* Insert task in the coRRSOFTect position */
  proc_table[p].status = RRSOFT_READY;
  iq_insertlast(p,&lev->ready);
}

static void RRSOFT_public_block(LEVEL l, PID p)
{
  /* Extract the running task from the level
     . we have already extract it from the ready queue at the dispatch time.
     . the capacity event have to be removed by the generic kernel
     . the wcet don't need modification...
     . the state of the task is set by the calling function

     So, we do nothing!!!
  */
}

static int RRSOFT_public_message(LEVEL l, PID p, void *m)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  if (lev->nact[p] > 0) {
    /* continue!!!! */
    lev->nact[p]--;
//    qq_insertlast(p,&lev->ready);
    iq_insertfirst(p,&lev->ready);
    proc_table[p].status = RRSOFT_READY;
  }
  else
    proc_table[p].status = RRSOFT_IDLE;

  jet_update_endcycle(); /* Update the Jet data... */
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l);
  
  return 0;
}

static void RRSOFT_public_end(LEVEL l, PID p)
{
  RRSOFT_level_des *lev = (RRSOFT_level_des *)(level_table[l]);

  lev->nact[p] = -1;

  /* we delete the reactivation timer */
  if (lev->periodic[p]) {
    kern_event_delete(lev->reactivation_timer[p]);
    lev->reactivation_timer[p] = -1;
  }

  /* then, we insert the task in the free queue */
  proc_table[p].status = FREE;
  iq_insertlast(p,&freedesc);
}

/* Registration functions */

/*+ This init function install the "main" task +*/
static void RRSOFT_call_main(void *l)
{
  LEVEL lev;
  PID p;
  NRT_TASK_MODEL m;
  void *mb;

  lev = (LEVEL)l;

  nrt_task_default_model(m);
  nrt_task_def_level(m,lev); /* with this we are sure that the task aRRSOFTives
                                to the coRRSOFTect level */

  mb = ((RRSOFT_level_des *)level_table[lev])->multiboot;
  nrt_task_def_arg(m,mb);
  nrt_task_def_usemath(m);
  nrt_task_def_nokill(m);
  nrt_task_def_ctrl_jet(m);

  p = task_create("Main", __init__, (TASK_MODEL *)&m, NULL);

  if (p == NIL)
    printk("\nPanic!!! can't create main task...\n");

  RRSOFT_public_activate(lev,p,NULL);
}


/*+ Registration function:
    TIME slice                the slice for the Round Robin queue
    int createmain            1 if the level creates the main task 0 otherwise
    struct multiboot_info *mb used if createmain specified   +*/
LEVEL RRSOFT_register_level(TIME slice,
                       int createmain,
                       struct multiboot_info *mb,
                       BYTE models)
{
  LEVEL l;            /* the level that we register */
  RRSOFT_level_des *lev;  /* for readableness only */
  PID i;

  printk("RRSOFT_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(RRSOFT_level_des));

  lev = (RRSOFT_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.public_scheduler = RRSOFT_public_scheduler;
  lev->l.public_create    = RRSOFT_public_create;
  lev->l.public_end       = RRSOFT_public_end;
  lev->l.public_dispatch  = RRSOFT_public_dispatch;
  lev->l.public_epilogue  = RRSOFT_public_epilogue;
  lev->l.public_activate  = RRSOFT_public_activate;
  lev->l.public_unblock   = RRSOFT_public_unblock;
  lev->l.public_block     = RRSOFT_public_block;
  lev->l.public_message   = RRSOFT_public_message;

  /* fill the RRSOFT descriptor part */
  for (i = 0; i < MAX_PROC; i++) {
    lev->nact[i] = -1;
    NULL_TIMESPEC(&lev->reactivation_time[i]);
    lev->reactivation_timer[i] = -1;
    lev->periodic[i] = 0;
    lev->period[i] = 0;
  }

  iq_init(&lev->ready, &freedesc, 0);

  if (slice < RRSOFT_MINIMUM_SLICE) slice = RRSOFT_MINIMUM_SLICE;
  if (slice > RRSOFT_MAXIMUM_SLICE) slice = RRSOFT_MAXIMUM_SLICE;
  lev->slice      = slice;

  lev->multiboot  = mb;

  lev->models     = models;

  if (createmain)
    sys_atrunlevel(RRSOFT_call_main,(void *) l, RUNLEVEL_INIT);

  return l;
}




