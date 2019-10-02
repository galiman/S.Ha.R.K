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
 CVS :        $Id: tbs.c,v 1.1 2005/02/25 10:43:38 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:43:38 $
 ------------

 This file contains the aperiodic server TBS (Total Bandwidth Server)

 Read tbs.h for further details.

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


#include <tbs/tbs/tbs.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/*+ 4 debug purposes +*/
#undef TBS_TEST

/*+ Status used in the level +*/
#define TBS_WCET_VIOLATED APER_STATUS_BASE+2  /*+ when wcet is finished +*/
#define TBS_WAIT          APER_STATUS_BASE    /*+ waiting the service   +*/

/*+ task flags +*/
#define TBS_SAVE_ARRIVALS 1

/*+ the level redefinition for the Total Bandwidth Server level +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  /* The wcet are stored in the task descriptor's priority
     field.                     */

  int nact[MAX_PROC];    /*+ used to record activations +*/
  BYTE flag[MAX_PROC];

  struct timespec lastdline; /*+ the last deadline assigned to
                                 a TBS task                   +*/

  IQUEUE wait;      /*+ the wait queue of the TBS              +*/
  PID activated;   /*+ the task inserted in another queue     +*/

  int flags;       /*+ the init flags...                      +*/

  bandwidth_t U;   /*+ the used bandwidth by the server       +*/
  int band_num;
  int band_den;

  LEVEL scheduling_level;

} TBS_level_des;

#ifdef TESTG
#include "drivers/glib.h"
#endif

/* This static function activates the task pointed by lev->activated) */
static __inline__ void TBS_activation(TBS_level_des *lev)
{
    PID p;             /* for readableness    */
    JOB_TASK_MODEL j;  /* the guest model     */
    TIME drel;         /* the relative deadline of the task */
    LEVEL m;           /* the master level... only for readableness */

#ifdef TESTG
    TIME x;
    extern TIME starttime;
#endif

    p = lev->activated;
    /* we compute a suitable deadline for the task */
    drel = (proc_table[p].wcet * lev->band_den) / lev->band_num;

    ADDUSEC2TIMESPEC(drel, &lev->lastdline);

#ifdef TESTG
    if (starttime) {
    x = ((lev->lastdline.tv_sec*1000000+lev->lastdline.tv_nsec/1000)/5000 - starttime) + 20;
    if (x<640)
      grx_plot(x, 15, 7);
    }
#endif

    /* and we insert the task in another level */
    m = lev->scheduling_level;
    job_task_default_model(j,lev->lastdline);
    level_table[m]->private_insert(m,p,(TASK_MODEL *)&j);

    #ifdef TBS_TEST
    kern_printf("TBS_activation: lastdline %ds %dns\n",lev->lastdline.tv_sec,lev->lastdline.tv_nsec);
    #endif
}

/* This static function reclaims the unused time of the task p */
static __inline__ void TBS_bandwidth_reclaiming(TBS_level_des *lev, PID p)
{
    TIME reclaimed;
    struct timespec r, sos;

//    kern_printf("%d ", proc_table[p].avail_time);
    reclaimed = (proc_table[p].avail_time * lev->band_den) / lev->band_num;

    r.tv_nsec = (reclaimed % 1000000) * 1000;
    r.tv_sec  = reclaimed / 1000000;

    SUBTIMESPEC(&lev->lastdline, &r, &sos);
    TIMESPEC_ASSIGN(&lev->lastdline, &sos);

    #ifdef TBS_TEST
    kern_printf("TBS_bandwidth_reclaiming: lastdline %ds %dns, reclaimed %d, avail %d\n",
              lev->lastdline.tv_sec, lev->lastdline.tv_nsec, reclaimed, proc_table[p].avail_time);
    #endif
}

/* The on-line guarantee is enabled only if the appropriate flag is set... */
static int TBS_public_guarantee(LEVEL l, bandwidth_t *freebandwidth)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

static int TBS_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  /* if the TBS_task_create is called, then the pclass must be a
     valid pclass. */
  SOFT_TASK_MODEL *s; 

  if (m->pclass != SOFT_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  s = (SOFT_TASK_MODEL *)m;
  if (!(s->wcet && s->periodicity == APERIODIC)) return -1;
  
  proc_table[p].wcet       = s->wcet;

  /* Enable wcet check */
  if (lev->flags & TBS_ENABLE_WCET_CHECK) {
    proc_table[p].avail_time = s->wcet;
    proc_table[p].control   |= CONTROL_CAP;
  }

  lev->nact[p] = 0;
  if (s->arrivals == SAVE_ARRIVALS)
    lev->flag[p] = TBS_SAVE_ARRIVALS;

  return 0; /* OK, also if the task cannot be guaranteed... */
}

static void TBS_public_dispatch(LEVEL l, PID p, int nostop)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  /* there is at least one task ready inserted in an EDF or similar
     level */

  level_table[ lev->scheduling_level ]->
    private_dispatch(lev->scheduling_level,p,nostop);
}

static void TBS_public_epilogue(LEVEL l, PID p)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  /* check if the wcet is finished... */
  if ((lev->flags & TBS_ENABLE_WCET_CHECK) && proc_table[p].avail_time <= 0) {
    /* if it is, raise a XWCET_VIOLATION exception */
    kern_raise(XWCET_VIOLATION,p);
    proc_table[p].status = TBS_WCET_VIOLATED;

    /* the current task have to die in the scheduling queue, and another
       have to be put in place... this code is identical to the
       TBS_task_end */
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level,p);

    /* we reclaim an avail time that can be <0 due to the timer
       approximations -> we have to postpone the deadline a little!
       we can use the ADDUSEC2TIMESPEC because the time postponed is
       less than 55ms */
    ADDUSEC2TIMESPEC((-proc_table[p].avail_time * lev->band_den)
                     / lev->band_num, &lev->lastdline);

    #ifdef TBS_TEST
    kern_printf("TBS_task_epilogue: Deadline posponed to %ds %dns\n",
              lev->lastdline.tv_sec, lev->lastdline.tv_nsec);
    #endif

    lev->activated = iq_getfirst(&lev->wait);
    if (lev->activated != NIL)
      TBS_activation(lev);
  }
  else
    /* the task has been preempted. it returns into the ready queue by
       calling the guest_epilogue... */
    level_table[ lev->scheduling_level ]->
      private_epilogue(lev->scheduling_level,p);
}

static void TBS_public_activate(LEVEL l, PID p, struct timespec *t)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  if (proc_table[p].status == SLEEP ||
       proc_table[p].status == TBS_WCET_VIOLATED) {

    if (TIMESPEC_A_GT_B(t, &lev->lastdline))
      TIMESPEC_ASSIGN(&lev->lastdline, t );


    if (lev->activated == NIL) {
      /* This is the first task in the level, so we activate it immediately */
      lev->activated = p;
      TBS_activation(lev);
    }
    else {
      proc_table[p].status = TBS_WAIT;
      iq_insertlast(p, &lev->wait);
    }
  }
  else if (lev->flag[p] & TBS_SAVE_ARRIVALS)
    lev->nact[p]++;
/*  else
    kern_printf("TBSREJ!!!");*/
}

static void TBS_public_unblock(LEVEL l, PID p)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);
  JOB_TASK_MODEL j;

  job_task_default_model(j,lev->lastdline);
  level_table[lev->scheduling_level]->
    private_insert(lev->scheduling_level,p,(TASK_MODEL *)&j);
}

static void TBS_public_block(LEVEL l, PID p)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  level_table[ lev->scheduling_level ]->
    private_extract(lev->scheduling_level,p);
}

static int TBS_public_message(LEVEL l, PID p, void *m)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  /* a task activation is finished, but we are using a JOB_TASK_MODEL
     that implements a single activation, so we have to call
     the guest_end, that representsa single activation... */
  level_table[ lev->scheduling_level ]->
    private_extract(lev->scheduling_level,p);

  TBS_bandwidth_reclaiming(lev,p);

  /* we reset the capacity counters... */
  if (lev->flags & TBS_ENABLE_WCET_CHECK)
    proc_table[p].avail_time = proc_table[p].wcet;

  if (lev->nact[p]) {
    // lev->nact[p] can be >0 only if the SAVE_ARRIVALS bit is set
    lev->nact[p]--;
    proc_table[p].status = TBS_WAIT;
    iq_insertlast(p, &lev->wait);
  }
  else
    proc_table[p].status = SLEEP;

  lev->activated = iq_getfirst(&lev->wait);
  if (lev->activated != NIL)
    TBS_activation(lev);

  jet_update_endcycle(); /* Update the Jet data... */
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l);
  
  return 0;
}

static void TBS_public_end(LEVEL l, PID p)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  level_table[ lev->scheduling_level ]->
    private_extract(lev->scheduling_level,p);

  TBS_bandwidth_reclaiming(lev,p);

  proc_table[p].status = FREE;
  iq_insertfirst(p,&freedesc);

  lev->activated = iq_getfirst(&lev->wait);
  if (lev->activated != NIL)
    TBS_activation(lev);
}

/* Registration functions */

/*+ Registration function:
    int flags                 the init flags ... see TBS.h +*/
void TBS_register_level(int flags, LEVEL master, int num, int den)
{
  LEVEL l;            /* the level that we register */
  TBS_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

  printk("TBS_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(TBS_level_des));

  lev = (TBS_level_des *)level_table[l];

  /* fill the standard descriptor */
  if (flags & TBS_ENABLE_GUARANTEE)
    lev->l.public_guarantee = TBS_public_guarantee;
  else
    lev->l.public_guarantee = NULL;

  lev->l.public_guarantee = TBS_public_guarantee;
  lev->l.public_create    = TBS_public_create;
  lev->l.public_end       = TBS_public_end;
  lev->l.public_dispatch  = TBS_public_dispatch;
  lev->l.public_epilogue  = TBS_public_epilogue;
  lev->l.public_activate  = TBS_public_activate;
  lev->l.public_unblock   = TBS_public_unblock;
  lev->l.public_block     = TBS_public_block;
  lev->l.public_message   = TBS_public_message;

  /* fill the TBS descriptor part */

  for (i = 0; i < MAX_PROC; i++) {
    lev->nact[i] = 0;
    lev->flag[i] = 0;
  }

  NULL_TIMESPEC(&lev->lastdline);

  iq_init(&lev->wait, &freedesc, 0);
  lev->activated = NIL;

  lev->U = (MAX_BANDWIDTH / den) * num;
  lev->band_num = num;
  lev->band_den = den;

  lev->scheduling_level = master;

  lev->flags = flags & 0x07;
}

bandwidth_t TBS_usedbandwidth(LEVEL l)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  return lev->U;
}

int TBS_get_nact(LEVEL l, PID p)
{
  TBS_level_des *lev = (TBS_level_des *)(level_table[l]);

  return lev->nact[p];
}

