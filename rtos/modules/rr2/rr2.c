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
 CVS :        $Id: rr2.c,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 This file contains the scheduling module RR2 (Round Robin) version 2

 Read rr2.h for further details.

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


#include <rr2/rr2/rr2.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/*+ Status used in the level +*/
#define RR2_READY   MODULE_STATUS_BASE

/*+ the level redefinition for the Round Robin level +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  int nact[MAX_PROC]; /*+ number of pending activations       +*/

  IQUEUE ready;    /*+ the ready queue                        +*/

  int slice;       /*+ the level's time slice                 +*/

  struct multiboot_info *multiboot; /*+ used if the level have to insert
                                        the main task +*/
} RR2_level_des;


/* This is not efficient but very fair :-)
   The need of all this stuff is because if a task execute a long time
   due to (shadow!) priority inheritance, then the task shall go to the
   tail of the queue many times... */
static PID RR2_public_scheduler(LEVEL l)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  PID p;

  for (;;) {
    p = iq_query_first(&lev->ready);
    if (p == -1)
      return p;

    if (proc_table[p].avail_time <= 0) {
      proc_table[p].avail_time += proc_table[p].wcet;
      iq_extract(p,&lev->ready);
      iq_insertlast(p,&lev->ready);
    }
    else
      return p;
  }
}

static int RR2_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);
  NRT_TASK_MODEL *nrt;

  if (m->pclass != NRT_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  nrt = (NRT_TASK_MODEL *)m;

  /* the task state is set at SLEEP by the general task_create
     the only thing to set remains the capacity stuffs that are set
     to the values passed in the model... */

  /* I used the wcet field because using wcet can account if a task
     consume more than the timeslice... */

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

  return 0; /* OK */
}

static void RR2_public_dispatch(LEVEL l, PID p, int nostop)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  /* the task state is set EXE by the scheduler()
     we extract the task from the ready queue
     NB: we can't assume that p is the first task in the queue!!! */
  iq_extract(p, &lev->ready);
}

static void RR2_public_epilogue(LEVEL l, PID p)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  /* check if the slice is finished and insert the task in the coRR2ect
     qqueue position */
  if (proc_table[p].avail_time <= 0) {
    proc_table[p].avail_time += proc_table[p].wcet;
    iq_insertlast(p,&lev->ready);
  }
  else
    /* cuRR2 is >0, so the running task have to run for another cuRR2 usec */
    iq_insertfirst(p,&lev->ready);

  proc_table[p].status = RR2_READY;
}

static void RR2_public_activate(LEVEL l, PID p, struct timespec *t)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  /* Test if we are trying to activate a non sleeping task    */
  /* save activation (only if needed... */
  if (proc_table[p].status != SLEEP) {
    if (lev->nact[p] != -1)
      lev->nact[p]++;
    return;
  }

  /* Insert task in the coRR2ect position */
  proc_table[p].status = RR2_READY;
  iq_insertlast(p,&lev->ready);
}

static void RR2_public_unblock(LEVEL l, PID p)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  /* Similar to RR2_task_activate, 
     but we don't check in what state the task is */

  /* Insert task in the coRR2ect position */
  proc_table[p].status = RR2_READY;
  iq_insertlast(p,&lev->ready);
}

static void RR2_public_block(LEVEL l, PID p)
{
  /* Extract the running task from the level
     . we have already extract it from the ready queue at the dispatch time.
     . the capacity event have to be removed by the generic kernel
     . the wcet don't need modification...
     . the state of the task is set by the calling function

     So, we do nothing!!!
  */
}

static int RR2_public_message(LEVEL l, PID p, void *m)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  if (lev->nact[p] > 0) {
    /* continue!!!! */
    lev->nact[p]--;
    iq_insertfirst(p,&lev->ready);
    proc_table[p].status = RR2_READY;
  }
  else
    proc_table[p].status = SLEEP;

  jet_update_endcycle(); /* Update the Jet data... */
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l);

  return 0;
}

static void RR2_public_end(LEVEL l, PID p)
{
  RR2_level_des *lev = (RR2_level_des *)(level_table[l]);

  lev->nact[p] = -1;

  /* then, we insert the task in the free queue */
  proc_table[p].status = FREE;
  iq_insertlast(p,&freedesc);
}

/* Registration functions */

/*+ This init function install the "main" task +*/
static void RR2_call_main(void *l)
{
  LEVEL lev;
  PID p;
  NRT_TASK_MODEL m;
  void *mb;

  lev = (LEVEL)l;

  nrt_task_default_model(m);
  nrt_task_def_level(m,lev); /* with this we are sure that the task aRR2ives
                                to the coRR2ect level */

  mb = ((RR2_level_des *)level_table[lev])->multiboot;
  nrt_task_def_arg(m,mb);
  nrt_task_def_usemath(m);
  nrt_task_def_nokill(m);
  nrt_task_def_ctrl_jet(m);

  p = task_create("Main", __init__, (TASK_MODEL *)&m, NULL);

  if (p == NIL)
    printk("\nPanic!!! can't create main task...\n");

  RR2_public_activate(lev,p,NULL);
}


/*+ Registration function:
    TIME slice                the slice for the Round Robin queue
    int createmain            1 if the level creates the main task 0 otherwise
    struct multiboot_info *mb used if createmain specified   +*/
LEVEL RR2_register_level(TIME slice,
                       int createmain,
                       struct multiboot_info *mb)
{
  LEVEL l;             /* the level that we register */
  RR2_level_des *lev;  /* for readableness only */
  PID i;

  printk("RR2_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(RR2_level_des));

  lev = (RR2_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.public_scheduler = RR2_public_scheduler;
  lev->l.public_create    = RR2_public_create;
  lev->l.public_end       = RR2_public_end;
  lev->l.public_dispatch  = RR2_public_dispatch;
  lev->l.public_epilogue  = RR2_public_epilogue;
  lev->l.public_activate  = RR2_public_activate;
  lev->l.public_unblock   = RR2_public_unblock;
  lev->l.public_block     = RR2_public_block;
  lev->l.public_message   = RR2_public_message;

  /* fill the RR2 descriptor part */
  for (i = 0; i < MAX_PROC; i++)
    lev->nact[i] = -1;

  iq_init(&lev->ready, &freedesc, 0);

  if (slice < RR2_MINIMUM_SLICE) slice = RR2_MINIMUM_SLICE;
  if (slice > RR2_MAXIMUM_SLICE) slice = RR2_MAXIMUM_SLICE;
  lev->slice      = slice;

  lev->multiboot  = mb;

  if (createmain)
    sys_atrunlevel(RR2_call_main,(void *) l, RUNLEVEL_INIT);

  return l;
}


