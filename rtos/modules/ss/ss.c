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
 CVS :        $Id: ss.c,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains the aperiodic Sporadic Server (SS).

 Note: in the following, server capacity and server budget are used as
       synonyms.

 When scheduling in background  the flags field has the SS_BACKGROUND bit set

 When scheduling a task because it is pointed by another task via shadows,
 the task have to be extracted from the wait queue or the master level. To
 check this we have to look at the activated field; it is != NIL if a task
 is inserted into the master level. Only a task at a time can be inserted
 into the master level.

 The capacity of the server must be updated
 - when scheduling a task normally
 - when scheduling a task because it is pointed by a shadow
   but not when scheduling in background.

 When a task is extracted from the system no scheduling has to be done
 until the task reenter into the system. To implement this, when a task
 is extracted we block the background scheduling (the scheduling with the
 master level is already blocked because the activated field is not
 reset to NIL) using the SS_BACKGROUNDBLOCK bit.

 nact[p] is -1 if the task set the activations to SKIP, >= 0 otherwise

 In contrast to classic SS scheme, the activation happens when
 a task does a create request while there is positive budget (instead to
 becomes active when there is a running task with priority higger then or
 equal to the server).
 So the replenish time is estabished on task arrival time. Replenish time
 is calculated as usual: activation time + server period.
 When the server ends its budget, becomes not active until a replenishment
 occurs.

 When a task ends its computation and there are no tasks to schedule or,
 again, the server budget ends, a replenish amount is posted so that, when
 replenish time fires, the server budget will be updated. Replenish
 amount is determined depending on how much time tasks have ran.
 Replenish amount does't takes into account periods during witch tasks
 handled by SS are preempted.

 There are two models used to handle a task is running into a critic section
 (owning a mutex): "nostop" model and "stop" model.
 Using the "nostop" model, a task that runs into a critic section is not 
 stopped when server ends its budget. This is done so higger priority tasks
 waiting for mutex are not blocked for so much time to replenish time occurs.
 When this happens the server capacity becomes negative and the replenish
 amount takes into account the negative budget part.
 With "stop" model running task is always suspended when server budget ends.
 If suspended task owns a mutex shared with higger priority task, the last
 one cannot runs until the mutex will be released. Higger priority task
 must waits at least upto next replenish time, when server budget will be
 refulled and suspended task runs again.

 Using "nostop" model, SS can uses more bandwidth respect to assigned
 capacity (due to negative budgets). So, calculating the guarantee, the
 longer critic section of all tasks handled by SS must be considered.

 SS can be used either with EDF or RM master level.

 Read SS.h for further details.

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


#include <stdlib.h>
#include <ss/ss/ss.h>
#include <ll/stdio.h>
#include <ll/string.h>

#include <ll/sys/ll/event.h>

#include <kernel/const.h>
#include <kernel/model.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/* For debugging purpose */
//#define DEBUG 1

/*+ Status used in the level +*/
#define SS_WAIT          APER_STATUS_BASE    /*+ waiting the service   +*/

/*+ Some useful macros +*/
#define BACKGROUND_ON  (lev->flags & SS_BACKGROUND)

extern struct event *firstevent;

/*+ the level redefinition for the Sporadic Server +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  /* The wcet are stored in the task descriptor's priority
     field, so no other fields are needed                      */

  int nact[MAX_PROC]; /*+ number of pending activations       +*/

  struct timespec lastdline; /*+ the last deeadline assigned to
                                 a SS task                    +*/

  int Cs;          /*+ server capacity                        +*/
  int availCs;     /*+ server avail time                      +*/
  int period;      /*+ Server period +*/
  
  bandwidth_t U;   /*+ the used bandwidth by the server       +*/

  IQUEUE wait;     /*+ the wait queue of the SS               +*/
  PID activated;   /*+ the task inserted in another queue     +*/

  int flags;       /*+ the init flags...                      +*/


  LEVEL scheduling_level;

  int replenishment[SS_MAX_REPLENISH]; /*+ contains replenish amounts +*/
  int rfirst,rlast;                    /*+ first and last valid replenish
                                            in replenish queue +*/
  int rcount;                           /*+ queued replenishments +*/

  int replenish_amount;            /*+ partial replenishments before post +*/
  ss_status server_active;         /*+ Is server active? +*/

} SS_level_des;

/*+ function prototypes +*/
void SS_internal_status(LEVEL l);
static void SS_replenish_timer(void *arg);
/*-------------------------------------------------------------------*/

/*** Utility functions ***/


/* These are for dinamic queue. **Disabled** */
#if 0
/* These routines are not tested, be carefull */

/*+ SS local memory allocator.
    Can be used for performance optimization.
    The interface is the same of kern_alloc() +*/
void inline * ss_alloc(DWORD b) {
	/* Now simply wraps to standard kernel alloc */
	return kern_alloc(b);
}

void ssq_inslast(LEVEL l, replenishq *elem) {

        SS_level_des *lev = (SS_level_des *) level_table[l];

	if(lev->rqueue_last == NULL) { /* empty queue */
		lev->rqueue_last=elem;
		lev->rqueue_first=elem;
		return;
	}
	elem->next = NULL;
	lev->rqueue_last->next = elem;
	lev->rqueue_last = elem;
}

replenishq *ssq_getfirst(LEVEL l) {

	SS_level_des *lev = (SS_level_des *) level_table[l];
	replenishq *tmp;
	
	if(lev->rqueue_first == NULL) { /* empty queue */
		return 0;
	}
	tmp = lev->rqueue_first;
	lev->rqueue_first = tmp->next;
	if(lev->rqueue_first == NULL) {	/* no more elements */
		lev->rqueue_last = NULL;
	}
	tmp->next = NULL;	/* to remove dangling pointer */
	return tmp;
}
#endif

/* For queue implemented with array.
   SS_MAX_REPLENISH array size assumed */

/*+ Insert an element at tail of replenish queue
 	LEVEL l			module level
 	int   amount		element to insert

	RETURNS:
 	  0	seccesfull insertion
 	  NIL	no more space for insertion +*/
static inline int ssq_inslast (LEVEL l, int amount) {

  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  #ifdef DEBUG
  kern_printf("insl ");
  #endif

  if (lev->rcount == SS_MAX_REPLENISH) {
    return NIL; /* no more space in the queue */
  }

  lev->replenishment[lev->rlast++] = amount;
  lev->rlast %= SS_MAX_REPLENISH;
  lev->rcount++;
  #ifdef DEBUG
  printf_xy(0,0,WHITE,"%d",lev->rcount);
  #endif

  return 0;
}

/*+ Get first element from replenish queue
	LEVEL l		module level

	RETURS:
	  extracted element
	  NIL on empty queue +*/
static inline int ssq_getfirst (LEVEL l) {

  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  int tmp;

  #ifdef DEBUG
  kern_printf("getf ");
  #endif

  if (lev->rcount == 0) {
    return NIL; /* empty queue */
  }
  tmp = lev->replenishment[lev->rfirst++];
  lev->rfirst %= SS_MAX_REPLENISH;
  lev->rcount--;
  #ifdef DEBUG
  printf_xy(0,0,WHITE,"%d",lev->rcount);
  #endif
  return tmp;
}

/*+ Enquire for empty queue
	LEVEL l		module level

	RETURS:
	  0	queue is not empty
	  1	queue is empty +*/
static inline int ssq_isempty (LEVEL l) {

  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  
  return !(lev->rcount);

//  if(lev->rcount == 0)
//    return 1;
//  return 0;
}

/*+ Set replenish amount for budget used during task execution
	LEVEL l		module level */
static inline void SS_set_ra(LEVEL l)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  /* replenish must be set when the server is still active */ 
  if(lev->server_active == SS_SERVER_ACTIVE) {
    lev->server_active = SS_SERVER_NOTACTIVE;
    if(ssq_inslast(l, lev->replenish_amount) == NIL) {
      kern_printf("SS: no more space to post replenishment\n");
      kern_printf("You should recompile setting higher SS_MAX_REPLENISH into include/modules/ss.h\n");
      SS_internal_status(l);
      kern_raise(XINVALID_SS_REPLENISH,exec_shadow);
      #ifdef DEBUG
      exit(-1);
      #endif
    }
    lev->replenish_amount = 0;
  }
  else {
    kern_printf("SS not active when posting R.A.\n");
    SS_internal_status(l);
    kern_raise(XINVALID_SS_REPLENISH,exec_shadow);
    #ifdef DEBUG
    exit(-1);
    #endif
  }
}
/* ------------------------------------------------------------------ */

/* This static function activates the task pointed by lev->activated) */
static inline void SS_activation(SS_level_des *lev)
{
    /* those two defines are for readableness */
    PID   p;
    LEVEL m;

    JOB_TASK_MODEL j;          /* the guest model */
//    struct timespec ty;

    #ifdef DEBUG
    kern_printf("SS_acti ");
    #endif

    p = lev->activated;
    m = lev->scheduling_level;

#if 0
    /* if server is active, replenish time already set */
    if (lev->server_active == SS_SERVER_NOTACTIVE) {
       lev->server_active = SS_SERVER_ACTIVE;
       /* set replenish time */
       TIMESPEC_ASSIGN(&ty, &proc_table[p].request_time);
       ADDUSEC2TIMESPEC(lev->period, &ty);
       TIMESPEC_ASSIGN(&lev->lastdline, &ty);
       #ifdef DEBUG
       kern_printf("RT:%d.%d ",ty.tv_sec,ty.tv_nsec);
       #endif
       kern_event_post(&ty, SS_replenish_timer, (void *) l);
    }
#endif

    job_task_default_model(j,lev->lastdline);
    job_task_def_period(j,lev->period);
    level_table[m]->private_insert(m,p,(TASK_MODEL *)&j);

    #ifdef DEBUG
    kern_printf("PID:%p lastdl:%d.%d ",p,lev->lastdline.tv_sec,lev->lastdline.tv_nsec);
    #endif
}

/*+ 
    Before call capacity_timer, update server capacity
    and replenish amount.
+*/
static void SS_capacity_timer(void *arg) {

        LEVEL l = (LEVEL)arg;
        SS_level_des *lev = (SS_level_des *)(level_table[l]);
	struct timespec ty;
	int tx;

        #ifdef DEBUG
        kern_printf("SS_captim ");
        #endif

	/* set replenish amount */
	/* task was running while budget ends */
	lev->server_active = SS_SERVER_NOTACTIVE;
	SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
	tx = TIMESPEC2USEC(&ty);
	lev->availCs -= tx;
	if(ssq_inslast(l, tx+lev->replenish_amount) == NIL) {
 	   kern_printf("SS: no more space to post replenishment\n");
	   kern_printf("    You should recompile setting higher SS_MAX_REPLENISH into include/modules/ss.h\n");
	   SS_internal_status(l);
	   kern_raise(XINVALID_SS_REPLENISH,exec_shadow);
	   #ifdef DEBUG
	   exit(-1);
	   #endif
	}
	lev->replenish_amount = 0;
	capacity_timer(NULL);
}

static void SS_replenish_timer(void *arg)
{
  LEVEL l = (LEVEL)arg;
  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  struct timespec ty;
  int amount;

    #ifdef DEBUG
    kern_printf("SS_reptim ");
    #endif

  /* availCs may be <0 because a task executed via a shadow for many time
     lev->activated == NIL only if the prec task was finished and there
     was not any other task to be put in the ready queue
     ... we are now activating the next task */
  if ((amount = ssq_getfirst(l)) != NIL) {
    lev->availCs += amount;
    #ifdef DEBUG
    kern_printf("AvaCs=%d ",lev->availCs);
    #endif
    if (lev->availCs > lev->Cs) {
      /* This should not be possible. I do so for robustness. */
      lev->availCs = lev->Cs;
      #ifdef DEBUG
      kern_printf("SS warning: budget higher then server capacity. Set to Cs.");
      #endif
    }
    if (lev->availCs <= 0) {
      /* we can be here if nostop model is used */
      #ifdef DEBUG
      kern_printf("WARNING: SS has non positive capacity after replenish.");
      #endif
      /* if there isn't pending replenishment and server
         is not active we must refull somehow.
         Otherwise SS remains not active forever */
      if(ssq_isempty(l) && lev->server_active == SS_SERVER_NOTACTIVE) {
	lev->availCs = lev->Cs;
	kern_printf("SS was full replenished due to irreversible non positive budget!!!\n");
	kern_printf("You should review your time extimation for critical sections ;)\n");
      }
    }
  }
  else {
    /* replenish queue is empty */
    kern_printf("Replenish Timer fires but no Replenish Amount defined\n");
    SS_internal_status(l);
    kern_raise(XINVALID_SS_REPLENISH,exec_shadow);
    #ifdef DEBUG
    exit(-1);
    #endif
  }

  if (lev->availCs > 0 && lev->activated == NIL) {
    if (iq_query_first(&lev->wait) != NIL) {
      lev->activated = iq_getfirst(&lev->wait);
      /* if server is active, replenish time already set */
      if (lev->server_active == SS_SERVER_NOTACTIVE) {
         lev->server_active = SS_SERVER_ACTIVE;
         /* set replenish time */
         kern_gettime(&ty);
         ADDUSEC2TIMESPEC(lev->period, &ty);
         TIMESPEC_ASSIGN(&lev->lastdline, &ty);
         #ifdef DEBUG
         kern_printf("RT:%d.%d ",ty.tv_sec,ty.tv_nsec);
         #endif
         kern_event_post(&ty, SS_replenish_timer, (void *) l);
      }
      SS_activation(lev);
      event_need_reschedule();
    }
  }
}

static char *SS_status_to_a(WORD status)
{
  if (status < MODULE_STATUS_BASE)
    return "Unavailable"; //status_to_a(status);

  switch (status) {
    case SS_WAIT         : return "SS_Wait";
    default              : return "SS_Unknown";
  }
}


/*-------------------------------------------------------------------*/

/*** Level functions ***/

void SS_internal_status(LEVEL l)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  PID p = iq_query_first(&lev->wait);

  kern_printf("On-line guarantee : %s\n",
    (lev->flags & SS_ENABLE_GUARANTEE_EDF ||
     lev->flags & SS_ENABLE_GUARANTEE_RM  )?"On":"Off");

  kern_printf("Used Bandwidth    : %u/%u\n",lev->U,MAX_BANDWIDTH);
  kern_printf("Period            : %d\n",lev->period);
  kern_printf("Capacity          : %d\n",lev->Cs);
  kern_printf("Avail capacity    : %d\n",lev->availCs);
  kern_printf("Server is %sactive\n",
     (lev->server_active == SS_SERVER_NOTACTIVE ? "not ":""));
  kern_printf("Pending RAs       : %d\n",lev->rcount);

  if (lev->activated != NIL)
    kern_printf("Activated: Pid: %d Name: %10s Dl: %ld.%ld Nact: %d Stat: %s\n",
                lev->activated,
                proc_table[lev->activated].name,
                iq_query_timespec(lev->activated,&lev->wait)->tv_sec,
                iq_query_timespec(lev->activated,&lev->wait)->tv_nsec,
                lev->nact[lev->activated],
                SS_status_to_a(proc_table[lev->activated].status));

  while (p != NIL) {
    kern_printf("Pid: %d\tName: %10s\tStatus: %s\n",
                p,
                proc_table[p].name,
                SS_status_to_a(proc_table[p].status));
    p = iq_query_next(p, &lev->wait);
  }
}

static PID SS_public_schedulerbackground(LEVEL l)
{
  /* the SS catch the background time to exec aperiodic activities */
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  #ifdef DEBUG
  kern_printf("SS_levschbg ");
  #endif

  lev->flags |= SS_BACKGROUND;

  if (lev->flags & SS_BACKGROUND_BLOCK)
    return NIL;
  else
    return iq_query_first(&lev->wait);
}

/* The on-line guarantee is enabled only if the appropriate flag is set... */
static int SS_public_guaranteeEDF(LEVEL l, bandwidth_t *freebandwidth)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  #ifdef DEBUG
  kern_printf("SS_levguarEDF ");
  #endif

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

static int SS_public_guaranteeRM(LEVEL l, bandwidth_t *freebandwidth)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  #ifdef DEBUG
  kern_printf("SS_levguarRM ");
  #endif

  if (*freebandwidth > lev->U + RM_MINFREEBANDWIDTH) {
    *freebandwidth -= lev->U;
    return 1;
  }
  else
    return 0;
}

/*-------------------------------------------------------------------*/

/***  Task functions  ***/


static int SS_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  SOFT_TASK_MODEL *s;

  #ifdef DEBUG
  kern_printf("SS_taskcre ");
  #endif

  if (m->pclass != SOFT_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  s = (SOFT_TASK_MODEL *)m;
  if (s->periodicity != APERIODIC) return -1;
 
  s = (SOFT_TASK_MODEL *)m;

  if (s->arrivals == SAVE_ARRIVALS)
    lev->nact[p] = 0;
  else
    lev->nact[p] = -1;

  return 0; /* OK, also if the task cannot be guaranteed */
}

static void SS_public_dispatch(LEVEL l, PID p, int nostop)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  struct timespec ty;

  #ifdef DEBUG
  kern_printf("SS_tdi ");
  #endif

  TIMESPEC_ASSIGN(&ty, &schedule_time);
  /* set replenish time */
  if(!BACKGROUND_ON) {
     if(lev->server_active == SS_SERVER_NOTACTIVE) {
        lev->server_active = SS_SERVER_ACTIVE;
        ADDUSEC2TIMESPEC(lev->period,&ty);
        TIMESPEC_ASSIGN(&lev->lastdline, &ty);
        #ifdef DEBUG
        kern_printf("tdiPID:%d RT:%d.%d ",p,ty.tv_sec,ty.tv_nsec);
        #endif
        kern_event_post(&ty, SS_replenish_timer,(void *) l);
     }
  }
  
  #ifdef DEBUG
  if (nostop) kern_printf("NOSTOP!!! ");
  #endif

  /* there is at least one task ready inserted in an RM or similar level.
     Note that we can't check the status because the scheduler sets it
     to exe before calling task_dispatch.
     We have to check lev->activated != p instead */
  if (lev->activated != p) {
    iq_extract(p, &lev->wait);
    #ifdef DEBUG
    kern_printf("extr task:%d ",p);
    #endif
  }
  else {
    #ifdef DEBUG
    if (nostop) kern_printf("(gd status=%d)",proc_table[p].status);
    #endif
    level_table[lev->scheduling_level]->
       private_dispatch(lev->scheduling_level,p,nostop);
  }

  /* set capacity timer */
  if (!nostop && !BACKGROUND_ON) {
    TIMESPEC_ASSIGN(&ty, &schedule_time);
//    kern_printf("ty:%d.%d ",ty.tv_sec,ty.tv_nsec);
    ADDUSEC2TIMESPEC((lev->availCs<=0 ? 0:lev->availCs),&ty);
//    kern_printf("avCs:%d ty:%d.%d ",lev->availCs,ty.tv_sec,ty.tv_nsec);
    /* stop the task if budget ends */
    #ifdef DEBUG
    kern_printf("PID:%d ST=%d.%d  ",p,ty.tv_sec,ty.tv_nsec);
    #endif
    cap_timer = kern_event_post(&ty, SS_capacity_timer,(void *) l);
  }
}

static void SS_public_epilogue(LEVEL l, PID p) {

  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  struct timespec ty;
  int tx;

  #ifdef DEBUG
  kern_printf("SS_tep ");
  #endif
  
  /* update the server capacity */
  if (BACKGROUND_ON)
    lev->flags &= ~SS_BACKGROUND;
  else {
    SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
//    kern_printf("ty:%d.%d ",ty.tv_sec,ty.tv_nsec);
    tx = TIMESPEC2USEC(&ty);
    lev->availCs -= tx;
//    kern_printf("avCs:%d ty:%d.%d ",lev->availCs,ty.tv_sec,ty.tv_nsec);
    lev->replenish_amount += tx;
    #ifdef DEBUG
    kern_printf("RA:%d ",lev->replenish_amount);
    #endif
  }

  /* check if the server capacity is finished... */
  if (lev->availCs <= 0) {
    /* The server slice has finished... do the task_end!!!
       A first version of the module used the task_endcycle, but it was
       not conceptually correct because the task didn't stop because it
       finished all the work, but because the server didn't have budget!
       So, if the task_endcycle is called, the task remain into the
       master level, and we can't wake him up if, for example, another
       task point the shadow to it!!! */

    /* set replenish amount */
    if(!(BACKGROUND_ON)) {
      if(lev->server_active == SS_SERVER_ACTIVE) {
        lev->server_active = SS_SERVER_NOTACTIVE;
        if(ssq_inslast(l, lev->replenish_amount) == NIL) {
          kern_printf("SS: no more space to post replenishment\n");
          kern_printf("You should recompile setting higher SS_MAX_REPLENISH into include/modules/ss.h\n");
          SS_internal_status(l);
          kern_raise(XINVALID_SS_REPLENISH,exec_shadow);
          #ifdef DEBUG
          exit(-1);
          #endif
        }
        lev->replenish_amount = 0;
      }
    }

    if (lev->activated == p)
      level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);

    iq_insertfirst(p, &lev->wait);
    proc_table[p].status = SS_WAIT;
    lev->activated = NIL;
  }
  else {
    /* The task has been preempted.
       It returns into the ready queue or to the
       wait queue by calling the private_epilogue... */

    if (lev->activated == p) {  /* goes into ready queue */
      level_table[ lev->scheduling_level ]->
        private_epilogue(lev->scheduling_level,p);
    }
    else {                      /* goes into wait queue */
      iq_insertfirst(p, &lev->wait);
      proc_table[p].status = SS_WAIT;
    }
  }
}

static void SS_public_activate(LEVEL l, PID p, struct timespec *t)
{
	SS_level_des *lev = (SS_level_des *)(level_table[l]);

        #ifdef DEBUG
        kern_printf("SS_tacti ");
        #endif

        if (lev->activated == p || proc_table[p].status == SS_WAIT) {
		if (lev->nact[p] != -1) lev->nact[p]++;
	}
	else if (proc_table[p].status == SLEEP) {
		if (lev->activated == NIL && lev->availCs > 0) {
		  if(!BACKGROUND_ON) {
		    /* if server is active, replenish time already set */
		    if (lev->server_active == SS_SERVER_NOTACTIVE) {
		      lev->server_active = SS_SERVER_ACTIVE;
		      /* set replenish time */
                      ADDUSEC2TIMESPEC(lev->period, t);
                      TIMESPEC_ASSIGN(&lev->lastdline, t);
		      #ifdef DEBUG
		      kern_printf("RT=%d.%d ",t->tv_sec,t->tv_nsec);
		      #endif
                      kern_event_post(t, SS_replenish_timer, (void *) l);
                    }
		  }
		  lev->activated = p;
		  SS_activation(lev);
		}
		else {
			iq_insertlast(p, &lev->wait);
			proc_table[p].status = SS_WAIT;
		}
	} 
	else {
		kern_printf("SS_REJ%d %d %d %d ",
		            p,
		            proc_table[p].status,
		            lev->activated,
		            lev->wait.first);
		return;
	}
}

static void SS_public_unblock(LEVEL l, PID p)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  #ifdef DEBUG
  kern_printf("SS_tins ");
  #endif
  lev->flags &= ~SS_BACKGROUND_BLOCK;

  lev->activated = NIL;

  /* when we reinsert the task into the system, the server capacity
     is always 0 because nobody executes with the SS before... */
  iq_insertfirst(p, &lev->wait);
  proc_table[p].status = SS_WAIT;
}

static void SS_public_block(LEVEL l, PID p)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  #ifdef DEBUG
  kern_printf("SS_textr ");
  #endif

  /* set replenish amount */
  if(!(BACKGROUND_ON)) {
    SS_set_ra(l);
  }  

  /* clear the server capacity */
  lev->availCs = 0;

  lev->flags |= SS_BACKGROUND_BLOCK;

  if (lev->activated == p)
    level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);
}

static int SS_public_message(LEVEL l, PID p, void *m)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  struct timespec ty;
  int tx;

  #ifdef DEBUG
  kern_printf("SS_tendcy ");
  #endif

  /* update the server capacity */
  if (BACKGROUND_ON)
    lev->flags &= ~SS_BACKGROUND;
  else {
    SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
    tx = TIMESPEC2USEC(&ty);
    lev->availCs -= tx;
    lev->replenish_amount += tx;
    #ifdef DEBUG
    kern_printf("PID:%d RA=%d ",lev->replenish_amount);
    #endif
  }

  if (lev->activated == p)
    level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);
  else
    iq_extract(p, &lev->wait);

  if (lev->nact[p] > 0) {
    lev->nact[p]--;
    iq_insertlast(p, &lev->wait);
    proc_table[p].status = SS_WAIT;
  }
  else {
    proc_table[p].status = SLEEP;
  }
  
  lev->activated = iq_getfirst(&lev->wait);
  if (lev->activated != NIL) {
    SS_activation(lev);
  }
  else {
    /* No more task to schedule; set replenish amount */
    if(!(BACKGROUND_ON)) {
      SS_set_ra(l);
    }
  }

  jet_update_endcycle(); /* Update the Jet data... */
  TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l);

  return 0;
}

static void SS_public_end(LEVEL l, PID p)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);
  struct timespec ty;
  int tx;

  #ifdef DEBUG
  kern_printf("SS_tend ");
  #endif

  /* update the server capacity */
  if (BACKGROUND_ON)
    lev->flags &= ~SS_BACKGROUND;
  else {
    SUBTIMESPEC(&schedule_time, &cap_lasttime, &ty);
    tx = TIMESPEC2USEC(&ty);
    lev->availCs -= tx;
    lev->replenish_amount += tx;
    #ifdef DEBUG
    kern_printf("PID:%d RA=%d ",p,lev->replenish_amount);
    #endif
  }

  if (lev->activated == p)
    level_table[lev->scheduling_level]->private_extract(lev->scheduling_level,p);

  proc_table[p].status = FREE;
  iq_insertfirst(p,&freedesc);

  lev->activated = iq_getfirst(&lev->wait);
  if (lev->activated != NIL) {
    SS_activation(lev);
  }
  else {
    if(!(BACKGROUND_ON)){
      /* No more task to schedule; set replenish amount */
      SS_set_ra(l);
    }
  }
}

/*-------------------------------------------------------------------*/

/*** Registration functions ***/


/*+ Registration function:
    int flags                 the init flags ... see SS.h +*/
LEVEL SS_register_level(int flags, LEVEL master, int Cs, int per)
{
  LEVEL l;            /* the level that we register */
  SS_level_des *lev;  /* for readableness only */
  PID i;              /* a counter */

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(SS_level_des));

  printk("SS_register_level\n");

  lev = (SS_level_des *)level_table[l];

  /* fill the standard descriptor */

  if (flags & SS_ENABLE_BACKGROUND)
    lev->l.public_scheduler = SS_public_schedulerbackground;

  if (flags & SS_ENABLE_GUARANTEE_EDF)
    lev->l.public_guarantee = SS_public_guaranteeEDF;
  else if (flags & SS_ENABLE_GUARANTEE_RM)
    lev->l.public_guarantee = SS_public_guaranteeRM;
  else
    lev->l.public_guarantee = NULL;

  lev->l.public_create    = SS_public_create;
  lev->l.public_end       = SS_public_end;
  lev->l.public_dispatch  = SS_public_dispatch;
  lev->l.public_epilogue  = SS_public_epilogue;
  lev->l.public_activate  = SS_public_activate;
  lev->l.public_unblock   = SS_public_unblock;
  lev->l.public_block     = SS_public_block;
  lev->l.public_message   = SS_public_message;

  /* fill the SS descriptor part */

  for (i=0; i<MAX_PROC; i++)
     lev->nact[i] = -1;

  lev->Cs = Cs;
  lev->availCs = Cs;

  lev->period = per;

  iq_init(&lev->wait, &freedesc, 0);
  lev->activated = NIL;

  lev->U = (MAX_BANDWIDTH / per) * Cs;
  
  lev->scheduling_level = master;
  
  lev->flags = flags & 0x07;
  
  /* This is superfluos. I do it for robustness */
  for (i=0;i<SS_MAX_REPLENISH;lev->replenishment[i++]=0);
 
  /* Initialize replenishment stuff */
  lev->rfirst=0;
  lev->rlast=0;
  lev->rcount=0;
  lev->replenish_amount=0;
  lev->server_active=SS_SERVER_NOTACTIVE;

  return l;
}

bandwidth_t SS_usedbandwidth(LEVEL l)
{
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  return lev->U;
}

int SS_availCs(LEVEL l) {
  SS_level_des *lev = (SS_level_des *)(level_table[l]);

  return lev->availCs;
}
