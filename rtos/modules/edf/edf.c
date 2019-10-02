/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors: 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   Anton Cervin
 *   Tullio Facchinetti  <tullio.facchinetti@unipv.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: edf.c,v 1.2 2006/03/22 16:06:19 tullio Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2006/03/22 16:06:19 $
 ------------

 This file contains the scheduling module EDF (Earliest Deadline First)

 Read edf.h for further details.

**/

/*
 * Copyright (C) 2000,2002 Paolo Gai
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


#include <edf/edf/edf.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <tracer.h>

//#define EDF_DEBUG
#define edf_printf kern_printf
#ifdef EDF_DEBUG
char *pnow() {
  static char buf[40];
  struct timespec t;
  sys_gettime(&t);
  sprintf(buf, "%ld.%06ld", t.tv_sec, t.tv_nsec/1000);
  return buf;
}
char *ptime1(struct timespec *t) {
  static char buf[40];
  sprintf(buf, "%ld.%06ld", t->tv_sec, t->tv_nsec/1000);
  return buf;
}
char *ptime2(struct timespec *t) {
  static char buf[40];
  sprintf(buf, "%ld.%06ld", t->tv_sec, t->tv_nsec/1000);
  return buf;
}
#endif

/* Statuses used in the level */
#define EDF_READY      MODULE_STATUS_BASE    /* ready */
#define EDF_IDLE       MODULE_STATUS_BASE+1  /* idle, waiting for offset/eop */
#define EDF_WAIT       MODULE_STATUS_BASE+2  /* to sleep, waiting for eop */
#define EDF_ZOMBIE     MODULE_STATUS_BASE+3  /* to free, waiting for eop */

/* Task flags */
#define EDF_FLAG_SPORADIC    1   /* the task is sporadic */
#define EDF_FLAG_SPOR_LATE   2   /* sporadic task with period overrun */ 


/* Task descriptor */
typedef struct {
  int flags;                    /* task flags                         */
  TIME period;                  /* period (or inter-arrival interval) */
  TIME rdeadline;               /* relative deadline                  */
  TIME offset;                  /* release offset                     */
  struct timespec release;      /* release time of current instance   */
  struct timespec adeadline;    /* latest assigned deadline           */
  int dl_timer;                 /* deadline timer                     */
  int eop_timer;                /* end of period timer                */
  int off_timer;		/* timer offset			      */
  int dl_miss;                  /* deadline miss counter              */
  int wcet_miss;                /* WCET miss counter                  */
  int act_miss;                 /* activation miss counter            */
  int nact;                     /* number of pending periodic jobs    */
} EDF_task_des;


/* Level descriptor */
typedef struct {
  level_des l;                  /* standard level descriptor          */
  int flags;                    /* level flags                        */
  IQUEUE ready;                 /* the ready queue                    */
  bandwidth_t U;                /* used bandwidth                     */
  EDF_task_des tvec[MAX_PROC];  /* vector of task descriptors         */
} EDF_level_des;


/* Module function cross-references */
static void EDF_intern_release(PID p, EDF_level_des *lev);


/**** Timer event handler functions ****/

/* This timer event handler is called at the end of the period */
static void EDF_timer_endperiod(void *par)
{
  PID p = (PID) par;
  EDF_level_des *lev = (EDF_level_des *)level_table[proc_table[p].task_level];
  EDF_task_des *td = &lev->tvec[p]; 
  
  td->eop_timer = -1;

  if (proc_table[p].status == EDF_ZOMBIE) {
    /* put the task in the FREE state */
    proc_table[p].status = FREE;
    iq_insertfirst(p,&freedesc);
    /* free the allocated bandwidth */
    lev->U -= (MAX_BANDWIDTH/td->rdeadline) * proc_table[p].wcet;
    return;
  }

  if (proc_table[p].status == EDF_WAIT) {
    proc_table[p].status = SLEEP;
    return;
  }
  
  if (td->flags & EDF_FLAG_SPORADIC) {
    /* the task is sporadic and still busy, mark it as late */
    td->flags |= EDF_FLAG_SPOR_LATE;
  } else {
    /* the task is periodic, release/queue another instance */
    EDF_intern_release(p, lev);
  }
}

/* This timer event handler is called when a task misses its deadline */
static void EDF_timer_deadline(void *par)
{
  PID p = (PID) par;
  EDF_level_des *lev = (EDF_level_des *)level_table[proc_table[p].task_level];
  EDF_task_des *td = &lev->tvec[p]; 

  td->dl_timer = -1;

  TRACER_LOGEVENT(FTrace_EVT_task_deadline_miss,
		  (unsigned short int)proc_table[p].context,0);

  if (lev->flags & EDF_ENABLE_DL_EXCEPTION) {
    kern_raise(XDEADLINE_MISS,p);
  } else {
    td->dl_miss++;
  }
}

/* This timer event handler is called after waiting for an offset */
static void EDF_timer_offset(void *par)
{
  PID p = (PID) par;
  EDF_level_des *lev;
  lev = (EDF_level_des *)level_table[proc_table[p].task_level];
  EDF_task_des *td = &lev->tvec[p];

  td->off_timer = -1;

  /* release the task now */
  EDF_intern_release(p, lev);

}

/* This function is called when a guest task misses its deadline */
static void EDF_timer_guest_deadline(void *par)
{
  PID p = (PID) par;
  EDF_level_des *lev = (EDF_level_des *)level_table[proc_table[p].task_level];
  EDF_task_des *td = &lev->tvec[p];

  td->dl_timer = -1;

  TRACER_LOGEVENT(FTrace_EVT_task_deadline_miss,
		  (unsigned short int)proc_table[p].context,0);

  kern_raise(XDEADLINE_MISS,p);

}


/**** Internal utility functions ****/

/* Release (or queue) a task, post deadline and endperiod timers */
static void EDF_intern_release(PID p, EDF_level_des *lev)
{
  struct timespec temp; 
  EDF_task_des *td = &lev->tvec[p]; 

  /* post deadline timer */
  if (lev->flags & EDF_ENABLE_DL_CHECK) {
    temp = td->release;
    if (td->rdeadline == td->period) ADDUSEC2TIMESPEC((td->rdeadline - 1), &temp); 
		else ADDUSEC2TIMESPEC(td->rdeadline, &temp); 
    if (td->dl_timer != -1) {
	kern_event_delete(td->dl_timer);
	td->dl_timer = -1;
    }
    td->dl_timer = kern_event_post(&temp,EDF_timer_deadline,(void *)p);
  }

  /* release or queue next job */
  if (proc_table[p].status == EDF_IDLE) {
    /* assign deadline, insert task in the ready queue */
    proc_table[p].status = EDF_READY;
    *iq_query_timespec(p,&lev->ready) = td->adeadline;
    iq_timespec_insert(p,&lev->ready);
#ifdef EDF_DEBUG
    edf_printf("At %s: releasing %s with deadline %s\n", pnow(),
       proc_table[p].name, ptime1(&td->adeadline));
#endif
    /* increase assigned deadline */
    ADDUSEC2TIMESPEC(td->period, &td->adeadline);
    /* reschedule */
    event_need_reschedule();
  } else {
    /* queue */
    td->nact++;
  }

  /* increase release time */
  ADDUSEC2TIMESPEC(td->period, &td->release);
  /* post end of period timer */ 
  if (td->eop_timer != -1) {
    kern_event_delete(td->eop_timer);
    td->eop_timer = -1;
  }

  td->eop_timer = kern_event_post(&td->release, EDF_timer_endperiod,(void *)p);

  TRACER_LOGEVENT(FTrace_EVT_task_timer,
		  (unsigned short int)proc_table[p].context,
		  (unsigned int)proc_table[p].task_level);
}


/**** Public generic kernel interface functions ****/

/* Returns the first task in the ready queue */
static PID EDF_public_scheduler(LEVEL l)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  return iq_query_first(&lev->ready);
}

/* Checks and decreases the available system bandwidth */
static int EDF_public_guarantee(LEVEL l, bandwidth_t *freebandwidth)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

/* Called by task_create: Checks task model and creates a task */
static int EDF_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 
  HARD_TASK_MODEL *h;

  if (m->pclass != HARD_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  h = (HARD_TASK_MODEL *)m;
  if (!h->wcet || !h->mit) return -1;
  if (h->drel > h->mit) return -1;  /* only D <= T supported */

  if (!h->drel) {
    td->rdeadline = h->mit;
  } else {
    td->rdeadline = h->drel;
  }

  /* check the free bandwidth... */
  if (lev->flags & EDF_ENABLE_GUARANTEE) {
    bandwidth_t b;
    b = (MAX_BANDWIDTH / td->rdeadline) * h->wcet;

    /* really update lev->U, checking an overflow... */
    if (MAX_BANDWIDTH - lev->U > b) {
      lev->U += b;
    } else {
      return -1;
    }
  }

  td->flags = 0;
  if (h->periodicity == APERIODIC) {
    td->flags |= EDF_FLAG_SPORADIC;
  }
  td->period = h->mit;
  
	td->offset = h->offset;
  td->dl_timer = -1;
  td->eop_timer = -1;
  td->off_timer = -1;
  td->dl_miss = 0;
  td->wcet_miss = 0;
  td->act_miss = 0;
  td->nact = 0;

  /* Enable wcet check */
  if (lev->flags & EDF_ENABLE_WCET_CHECK) {
    proc_table[p].avail_time = h->wcet;
    proc_table[p].wcet       = h->wcet;
    proc_table[p].control |= CONTROL_CAP; /* turn on measurement */
  }

  return 0; /* OK, also if the task cannot be guaranteed... */
}

/* Reclaim the bandwidth used by the task */
static void EDF_public_detach(LEVEL l, PID p)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  if (lev->flags & EDF_ENABLE_GUARANTEE) {
    lev->U -= (MAX_BANDWIDTH / td->rdeadline) * proc_table[p].wcet;
  }
}

/* Extracts the running task from the ready queue */
static void EDF_public_dispatch(LEVEL l, PID p, int nostop)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  iq_extract(p, &lev->ready);
}

/* Called when the task is preempted or when its budget is exhausted */
static void EDF_public_epilogue(LEVEL l, PID p)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  /* check if the wcet is finished... */
  if (lev->flags & EDF_ENABLE_WCET_CHECK) {
    if (proc_table[p].avail_time <= 0) {
      TRACER_LOGEVENT(FTrace_EVT_task_wcet_violation,
		      (unsigned short int)proc_table[p].context,0);
      if (lev->flags & EDF_ENABLE_WCET_EXCEPTION) {
	kern_raise(XWCET_VIOLATION,p);
      } else {
	proc_table[p].control &= ~CONTROL_CAP;
	td->wcet_miss++;
      }
    }
  }

  /* the task returns to the ready queue */
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].status = EDF_READY;

}

/* Called by task_activate or group_activate: Activates the task at time t */
static void EDF_public_activate(LEVEL l, PID p, struct timespec *t)
{
  struct timespec clocktime;
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  kern_gettime(&clocktime);

  /* check if we are not in the SLEEP state */
  if (proc_table[p].status != SLEEP) {
    if (lev->flags & EDF_ENABLE_ACT_EXCEPTION) {
      /* too frequent or wrongful activation: raise exception */
      kern_raise(XACTIVATION,p);
    } else {
      /* skip the sporadic job, but increase a counter */
#ifdef EDF_DEBUG
      edf_printf("At %s: activation of %s skipped\n", pnow(),
		 proc_table[p].name);
#endif
      td->act_miss++;
    }
    return;
  }
 
  /* set the release time to the activation time + offset */
  td->release = *t;
  ADDUSEC2TIMESPEC(td->offset, &td->release);

  /* set the absolute deadline to the activation time + offset + rdeadline */
  td->adeadline = td->release;
  ADDUSEC2TIMESPEC(td->rdeadline, &td->adeadline);

  /* Check if release > clocktime. If yes, release it later,
     otherwise release it now. */

  proc_table[p].status = EDF_IDLE;

  if (TIMESPEC_A_GT_B(&td->release, &clocktime)) {
    /* release later, post an offset timer */
    if (td->off_timer != -1) {
	kern_event_delete(td->off_timer);
	td->off_timer = -1;
    }
    td->off_timer = kern_event_post(&td->release,EDF_timer_offset,(void *)p);
  } else {
    /* release now */
    EDF_intern_release(p, lev);
  }
}

/* Reinserts a task that has been blocked into the ready queue */
static void EDF_public_unblock(LEVEL l, PID p)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);

  /* Insert task in the correct position */
  proc_table[p].status = EDF_READY;
  iq_timespec_insert(p,&lev->ready);
}

/* Called when a task experiences a synchronization block */
static void EDF_public_block(LEVEL l, PID p)
{
  /* Extract the running task from the level
     . we have already extract it from the ready queue at the dispatch time.
     . the capacity event have to be removed by the generic kernel
     . the wcet don't need modification...
     . the state of the task is set by the calling function
     . the deadline must remain...

     So, we do nothing!!!
  */
}

/* Called by task_endcycle or task_sleep: Ends the current instance */
static int EDF_public_message(LEVEL l, PID p, void *m)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  switch((long)(m)) {
    /* task_endcycle() */
  case 0:
    /* if there are no pending jobs */
    if (td->nact == 0) {
      /* remove deadline timer, if any */
      if (td->dl_timer != -1) {
	kern_event_delete(td->dl_timer);
	td->dl_timer = -1;
      }
      if (td->flags & EDF_FLAG_SPORADIC) {
	/* sporadic task */
	if (!(td->flags & EDF_FLAG_SPOR_LATE)) {
	  proc_table[p].status = EDF_WAIT;
	} else {
	  /* it's late, move it directly to SLEEP */
	  proc_table[p].status = SLEEP;
	  td->flags &= ~EDF_FLAG_SPOR_LATE;
	}
      } else {
	/* periodic task */
	proc_table[p].status = EDF_IDLE;
      }
    } else {
      /* we are late / there are pending jobs */
      td->nact--;
      /* compute and assign absolute deadline */
      *iq_query_timespec(p,&lev->ready) = td->adeadline;
      iq_timespec_insert(p,&lev->ready);
      /* increase assigned deadline */
      ADDUSEC2TIMESPEC(td->period, &td->adeadline);
#ifdef EDF_DEBUG
      edf_printf("(Late) At %s: releasing %s with deadline %s\n",
	 pnow(),proc_table[p].name,ptime1(&td->adeadline));
#endif
    }
    break;
    
    /* task_sleep() */
  case 1:
    /* remove deadline timer, if any */
    if (td->dl_timer != -1) {
      kern_event_delete(td->dl_timer);
      td->dl_timer = -1;
    }
    if (td->flags & EDF_FLAG_SPORADIC) {
      /* sporadic task */
      if (!(td->flags & EDF_FLAG_SPOR_LATE)) {
	proc_table[p].status = EDF_WAIT;
      } else {
	/* it's late, move it directly to SLEEP */
	proc_table[p].status = SLEEP;
	td->flags &= ~EDF_FLAG_SPOR_LATE;
      }
    } else {
      /* periodic task */
      if (!(td->nact > 0)) {
	/* we are on time. go to the EDF_WAIT state */
	proc_table[p].status = EDF_WAIT;
      } else {
	/* we are late. delete pending activations and go to SLEEP */
	td->nact = 0;
	proc_table[p].status = SLEEP;
	/* remove end of period timer */
	if (td->eop_timer != -1) {
	  kern_event_delete(td->eop_timer);
	  td->eop_timer = -1;
	}
      }
    }
    break;
  }

  if (lev->flags & EDF_ENABLE_WCET_CHECK) {
    proc_table[p].control |= CONTROL_CAP;
  }
  jet_update_endcycle(); /* Update the Jet data... */
  proc_table[p].avail_time = proc_table[p].wcet;
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,
		  (unsigned short int)proc_table[p].context,(unsigned int)l);
  
  return 0;

}

/* End the task and free the resources at the end of the period */
static void EDF_public_end(LEVEL l, PID p)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  if (!(td->flags & EDF_FLAG_SPOR_LATE)) {
    /* remove the deadline timer (if any) */
    if (td->dl_timer != -1) {
      kern_event_delete(td->dl_timer);
      td->dl_timer = -1;
    }
    proc_table[p].status = EDF_ZOMBIE;
  } else {
    /* no endperiod timer will be fired, free the task now! */
    proc_table[p].status = FREE;
    iq_insertfirst(p,&freedesc);
    /* free the allocated bandwidth */
    lev->U -= (MAX_BANDWIDTH/td->rdeadline) * proc_table[p].wcet;
  }
}

/**** Private generic kernel interface functions (guest calls) ****/

/* Insert a guest task */
static void EDF_private_insert(LEVEL l, PID p, TASK_MODEL *m)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 
  JOB_TASK_MODEL *job;

  if (m->pclass != JOB_PCLASS || (m->level != 0 && m->level != l) ) {
    kern_raise(XINVALID_TASK, p);
    return;
  }

  job = (JOB_TASK_MODEL *)m;

  /* Insert task in the correct position */
  *iq_query_timespec(p, &lev->ready) = job->deadline;
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].status = EDF_READY;
  
  if (td->dl_timer != -1) {
	kern_event_delete(td->dl_timer);
	td->dl_timer = -1;
  }

  td->period = job->period;

  if (!job->noraiseexc) {
    td->dl_timer = kern_event_post(iq_query_timespec(p, &lev->ready),
                                       EDF_timer_guest_deadline,(void *)p);
  }
}

/* Dispatch a guest task */
static void EDF_private_dispatch(LEVEL l, PID p, int nostop)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);

  /* the task state is set to EXE by the scheduler()
     we extract the task from the ready queue
     NB: we can't assume that p is the first task in the queue!!! */
  iq_extract(p, &lev->ready);
}

/* Called when a guest task is preempted/out of budget */
static void EDF_private_epilogue(LEVEL l, PID p)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);

  /* the task has been preempted. it returns into the ready queue... */
  iq_timespec_insert(p,&lev->ready);
  proc_table[p].status = EDF_READY;
}

/* Extract a guest task */
static void EDF_private_extract(LEVEL l, PID p)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  if (proc_table[p].status == EDF_READY)
    iq_extract(p, &lev->ready);

  /* we remove the deadline timer, because the slice is finished */
  if (td->dl_timer != -1) {
    kern_event_delete(td->dl_timer);
    td->dl_timer = -1;
  }

}


/**** Level registration function ****/

LEVEL EDF_register_level(int flags)
{
  LEVEL l;            /* the level that we register */
  EDF_level_des *lev;  /* for readableness only */
  int i;

  printk("EDF_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(EDF_level_des));

  lev = (EDF_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.private_insert   = EDF_private_insert;
  lev->l.private_extract  = EDF_private_extract;
  lev->l.private_dispatch = EDF_private_dispatch;
  lev->l.private_epilogue = EDF_private_epilogue;

  lev->l.public_scheduler = EDF_public_scheduler;
  if (flags & EDF_ENABLE_GUARANTEE)
    lev->l.public_guarantee = EDF_public_guarantee;
  else
    lev->l.public_guarantee = NULL;

  lev->l.public_create    = EDF_public_create;
  lev->l.public_detach    = EDF_public_detach;
  lev->l.public_end       = EDF_public_end;
  lev->l.public_dispatch  = EDF_public_dispatch;
  lev->l.public_epilogue  = EDF_public_epilogue;
  lev->l.public_activate  = EDF_public_activate;
  lev->l.public_unblock   = EDF_public_unblock;
  lev->l.public_block     = EDF_public_block;
  lev->l.public_message   = EDF_public_message;

  iq_init(&lev->ready, &freedesc, 0);

  lev->flags = flags;
  if (lev->flags & EDF_ENABLE_WCET_EXCEPTION) {
    lev->flags |= EDF_ENABLE_WCET_CHECK;
  }
  if (lev->flags & EDF_ENABLE_DL_EXCEPTION) {
    lev->flags |= EDF_ENABLE_DL_CHECK;
  }

  lev->U = 0;

  for (i=0;i<MAX_PROC;i++) {
    EDF_task_des *td = &lev->tvec[i];
    td->flags = 0;
    td->dl_timer = -1;
    td->eop_timer = -1;
    td->off_timer = -1;
    td->dl_miss = 0;
    td->wcet_miss = 0;
    td->act_miss = 0;
    td->nact = 0;
  }

  return l;
}


/**** Public utility functions ****/

/* Get the bandwidth used by the level */
bandwidth_t EDF_usedbandwidth(LEVEL l)
{
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);

  return lev->U;
}

/* Get the number of missed deadlines for a task */
int EDF_get_dl_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  return td->dl_miss;
}

/* Get the number of execution overruns for a task */
int EDF_get_wcet_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 

  return td->wcet_miss;
}

/* Get the number of skipped activations for a task */
int EDF_get_act_miss(PID p)
{
  LEVEL l = proc_table[p].task_level;
  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 
  
  return td->act_miss;
}

/* Get the current number of queued activations for a task */
int EDF_get_nact(PID p)
{
  LEVEL l = proc_table[p].task_level;

  EDF_level_des *lev = (EDF_level_des *)(level_table[l]);
  EDF_task_des *td = &lev->tvec[p]; 
  
  return td->nact;
}

