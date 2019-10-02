/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors:
 *	Giacomo Guidi	 <giacomo@gandalf.sssup.it>
 *	Mauro Marinoni
 *	Anton Cervin 
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
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

#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <ll/i386/64bit.h>

#include <stdlib.h>

#include <elastic/elastic/elastic.h>

#include <tracer.h>

/* Task flags */

#define ELASTIC_PRESENT       1
#define ELASTIC_JOB_PRESENT   2  

/* Task statuses */

#define ELASTIC_IDLE          APER_STATUS_BASE

//#define ELASTIC_DEBUG

#ifdef ELASTIC_DEBUG
char *pnow() {
  static char buf[40];
  struct timespec t;
  kern_gettime(&t);
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


typedef struct {

  /* Task parameters (set/changed by the user) */

  TIME Tmin;   /* The nominal (minimum) period */
  TIME Tmax;   /* The maximum tolerable period */
  TIME C;      /* The declared worst-case execution time */
  int  E;      /* The elasticity coefficient */
  int  beta;   /* PERIOD_SCALING or WCET_SCALING */

  /* Task variables (changed by the module) */

  struct timespec release;    /* The current activation time */
  struct timespec dline;      /* The current absolute deadline */
  int dltimer;                /* Deadline timer handle */
  
  ext_bandwidth_t Umax;       /* The maximum utilization, Umax = C/Tmin  */
  ext_bandwidth_t Umin;       /* The minimum utilization, Umin = C/Tmax  */

  ext_bandwidth_t U;          /* New assigned utilization             */
  ext_bandwidth_t oldU;       /* Old utilization                      */
  TIME T;                     /* The current period, T = C/U          */

  int  flags;

} ELASTIC_task_descr;

typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  ext_bandwidth_t U;   /*+ the bandwidth reserved for elastic tasks  +*/

  int c_scaling_factor;   /*+ the computation time scaling factor +*/

  ELASTIC_task_descr elist[MAX_PROC];

  LEVEL scheduling_level;

  LEVEL current_level;

  int flags;

} ELASTIC_level_des;


static void ELASTIC_activation(ELASTIC_level_des *lev, PID p,
                               struct timespec *acttime)
{
  JOB_TASK_MODEL job;
  ELASTIC_task_descr *et = &lev->elist[p];

  /* Assign release time */
  et->release = *acttime;

  /* Assign absolute deadline */
  et->dline = *acttime;
  ADDUSEC2TIMESPEC(et->T, &et->dline);

#ifdef ELASTIC_DEBUG
  /*  cprintf("At %s: activating %s; rel=%s; dl=%s\n", pnow(), proc_table[p].name,
      ptime1(&et->release), ptime2(&et->dline)); */
#endif  

  mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,proc_table[p].avail_time);
  mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,proc_table[p].wcet);

  /* Job insertion */
  job_task_default_model(job, et->dline);
  level_table[lev->scheduling_level]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&job);
  et->flags |= ELASTIC_JOB_PRESENT;
}


static void ELASTIC_timer_act(void *arg) {

  PID p = (PID)(arg);
  ELASTIC_level_des *lev;
                       
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];

  /* Use the current deadline as the new activation time */
  ELASTIC_activation(lev, p, &et->dline); 

  event_need_reschedule();

  /* Next activation */
  et->dltimer = kern_event_post(&et->dline, ELASTIC_timer_act, (void *)(p));
}


/* Check feasability and compute new utilizations for the task set */

static int ELASTIC_compress(ELASTIC_level_des *lev) {

  PID i;
  ELASTIC_task_descr *et;
  int ok;
  
  ext_bandwidth_t Umin;  // minimum utilization
  ext_bandwidth_t Umax;  // nominal (maximum) utilization of compressable tasks
  unsigned int temp;

  ext_bandwidth_t Uf;    // amount of non-compressable utilization
  int Ev;                // sum of elasticity among compressable tasks

  JOB_TASK_MODEL job;

  Umin = 0;
  Umax = 0;

  for (i=0; i<MAX_PROC; i++) {
    et = &lev->elist[i];
    if (et->flags & ELASTIC_PRESENT) {
      if (et->E == 0) {
	Umin += et->U;
	Umax += et->U;
      } else {
	Umin += et->Umin;
	Umax += et->Umax;
	et->U = et->Umax;   // reset previous saturations (if any)
      }
    }
  }

  if (Umin > lev->U) {
#ifdef ELASTIC_DEBUG
    cprintf("ELASTIC_compress: Task set not feasible\n");
#endif
    return -1;  // NOT FEASIBLE
  }

  if (Umax <= lev->U) {
#ifdef ELASTIC_DEBUG
    cprintf("ELASTIC_compress: Task set feasible with maximum utilizations\n");
#endif

  } else {

    do {
      Uf = 0;
      Ev = 0;
      Umax = 0;
      
      for (i=0; i<MAX_PROC; i++) {
	et = &lev->elist[i];
	if (et->flags & ELASTIC_PRESENT) {
	  if (et->E == 0 || et->U == et->Umin) {
	    Uf += et->U;
	  } else {
	    Ev += et->E;
	    Umax += et->Umax;
	  }
	}
      }
      
      ok = 1;
      
      for (i=0; i<MAX_PROC; i++) {
	et = &lev->elist[i];
	if (et->flags & ELASTIC_PRESENT) {
	  if (et->E > 0 && et->U > et->Umin) {
	    et->U = et->Umax - (Umax - lev->U + Uf) * et->E / Ev;
	    if (et->U < et->Umin) {
	      et->U = et->Umin;
	      ok = 0;
	    }
	  }
	}
      }
      
    } while (ok == 0);
  }

  // Increase periods of compressed tasks IMMEDIATELY.
  // The other ones will be changed at their next activation

  for (i=0; i<MAX_PROC; i++) {
    et = &lev->elist[i];
    if (et->flags & ELASTIC_PRESENT) {
      if (et->U != et->oldU) {
	/* Utilization has been changed. Compute new period */
	temp = (long long)et->C * (long long)MAX_BANDWIDTH / et->U;
        mul32div32to32(temp,lev->c_scaling_factor,SCALING_UNIT,et->T);
      }
      if (et->U < et->oldU) {
	/* Task has been compressed. Change its deadline NOW! */
	if (et->flags & ELASTIC_JOB_PRESENT) {
	  /* Remove job from level */
	  level_table[lev->scheduling_level]->
	    private_extract(lev->scheduling_level, i);
	}
	/* Compute new deadline */
	et->dline = et->release;
	ADDUSEC2TIMESPEC(et->T, &et->dline);
	if (et->dltimer != -1) {
	  /* Delete old deadline timer, post new one */
	  kern_event_delete(et->dltimer);
	  et->dltimer = kern_event_post(&et->dline, ELASTIC_timer_act,(void *)(i));
	}
	if (et->flags & ELASTIC_JOB_PRESENT) {
	  /* Reinsert job */
	  job_task_default_model(job, et->dline);
	  level_table[lev->scheduling_level]->
	    private_insert(lev->scheduling_level, i, (TASK_MODEL *)&job);
	}
      }
      et->oldU = et->U;  /* Update oldU */
    }
  }

#ifdef ELASTIC_DEBUG
  cprintf("ELASTIC_compress: New periods: ");
  for (i=0; i<MAX_PROC; i++) {
    et = &lev->elist[i];
    if (et->flags & ELASTIC_PRESENT) {
      cprintf("%s:%d ", proc_table[i].name, (int)et->T);
    }
  }
  cprintf("\n");
#endif

  return 0; // FEASIBLE

}


/* The on-line guarantee is enabled only if the appropriate flag is set... */
static int ELASTIC_public_guarantee(LEVEL l, bandwidth_t *freebandwidth)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);

  if (*freebandwidth >= lev->U) {
    *freebandwidth -= (unsigned int)lev->U;
    return 1;
  } else {
    return 0;
  }

}


static int ELASTIC_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);
  ELASTIC_TASK_MODEL *elastic = (ELASTIC_TASK_MODEL *)m;
  ELASTIC_task_descr *et = &lev->elist[p];
  unsigned int temp;

  if (m->pclass != ELASTIC_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;

  if (elastic->C == 0) return -1;
  if (elastic->Tmin > elastic->Tmax) return -1;
  if (elastic->Tmax == 0) return -1;
  if (elastic->Tmin == 0) return -1;

  NULL_TIMESPEC(&(et->dline));
  et->Tmin = elastic->Tmin;
  et->Tmax = elastic->Tmax;
  et->C = elastic->C;
  et->E = elastic->E;
  et->beta = elastic->beta;

  mul32div32to32(elastic->C,lev->c_scaling_factor,SCALING_UNIT,temp);
  et->Umax = ((long long)MAX_BANDWIDTH * (long long)temp) / (long long)elastic->Tmin;
  et->Umin = ((long long)MAX_BANDWIDTH * (long long)temp) / (long long)elastic->Tmax;

  et->U = et->Umax;
  et->oldU = 0;
  et->T = et->Tmin;
  et->dltimer = -1;

  et->flags |= ELASTIC_PRESENT;
  if (ELASTIC_compress(lev) == -1) {
    et->flags &= ~ELASTIC_PRESENT;
#ifdef ELASTIC_DEBUG
    cprintf("ELASTIC_public_create: compression failed!\n");
#endif
    return -1;
  }

  mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,proc_table[p].avail_time);
  mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,proc_table[p].wcet);

  proc_table[p].control    |= CONTROL_CAP; 

  return 0;
}


static void ELASTIC_public_detach(LEVEL l, PID p)
{
  //ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);

}

static int ELASTIC_public_eligible(LEVEL l, PID p)
{
  //ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);

  return 0;

}

static void ELASTIC_public_dispatch(LEVEL l, PID p, int nostop)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);

  level_table[ lev->scheduling_level ]->
    private_dispatch(lev->scheduling_level,p,nostop);

}

static void ELASTIC_public_epilogue(LEVEL l, PID p)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);

  /* check if the wcet is finished... */
  if (proc_table[p].avail_time <= 0) {
    
    TRACER_LOGEVENT(FTrace_EVT_task_wcet_violation,
		    (unsigned short int)proc_table[p].context,0);
    kern_raise(XWCET_VIOLATION,p);
    
  }

  level_table[lev->scheduling_level]->
      private_epilogue(lev->scheduling_level,p);

}

static void ELASTIC_public_activate(LEVEL l, PID p, struct timespec *t)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);
  ELASTIC_task_descr *et = &lev->elist[p];

  /* check if we are not in the SLEEP state */
  if (proc_table[p].status != SLEEP) {
    return;
  }

  ELASTIC_activation(lev,p,t);

  /* Next activation */
  et->dltimer = kern_event_post(&et->dline, ELASTIC_timer_act, (void *)(p));

}

static void ELASTIC_public_unblock(LEVEL l, PID p)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);
  struct timespec acttime;

  kern_gettime(&acttime);

  ELASTIC_activation(lev,p,&acttime);

}

static void ELASTIC_public_block(LEVEL l, PID p)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);
  ELASTIC_task_descr *et = &lev->elist[p];

  level_table[lev->scheduling_level]->
    private_extract(lev->scheduling_level,p);
  et->flags &= ~ELASTIC_JOB_PRESENT;

}

static int ELASTIC_public_message(LEVEL l, PID p, void *m)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);
  ELASTIC_task_descr *et = &lev->elist[p];

  switch((long)(m)) {

    case (long)(NULL):

      level_table[lev->scheduling_level]->
        private_extract(lev->scheduling_level,p);
      et->flags &= ~ELASTIC_JOB_PRESENT;

      proc_table[p].status = ELASTIC_IDLE;

      jet_update_endcycle(); /* Update the Jet data... */
      TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,(unsigned short int)proc_table[p].context,(unsigned int)l); 

      break;

    case 1:

      if (et->dltimer != -1)
        kern_event_delete(et->dltimer);

      if (et->flags & ELASTIC_JOB_PRESENT) {
        level_table[ lev->scheduling_level ]->
          private_extract(lev->scheduling_level,p);
        et->flags &= ~ELASTIC_JOB_PRESENT;
      }

      proc_table[p].status = SLEEP;

      TRACER_LOGEVENT(FTrace_EVT_task_disable,(unsigned short int)proc_table[p].context,(unsigned int)l);

      break;

  }

  return 0;

}

static void ELASTIC_public_end(LEVEL l, PID p)
{
  ELASTIC_level_des *lev = (ELASTIC_level_des *)(level_table[l]);
  ELASTIC_task_descr *et = &lev->elist[p];

  if (et->dltimer != -1) {
    kern_event_delete(et->dltimer);
  }

  if (et->flags & ELASTIC_JOB_PRESENT) {
    level_table[ lev->scheduling_level ]->
      private_extract(lev->scheduling_level,p);
    et->flags &= ~ELASTIC_JOB_PRESENT;
  }

  et->flags &= ~ELASTIC_PRESENT;

  ELASTIC_compress(lev); // Tasks may want to expand
}

/*+ Registration function +*/
LEVEL ELASTIC_register_level(int flags, LEVEL master, ext_bandwidth_t U)
{
  LEVEL l;            /* the level that we register */
  ELASTIC_level_des *lev;  /* for readableness only */
  PID i;

  printk("ELASTIC_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(ELASTIC_level_des));

  lev = (ELASTIC_level_des *)level_table[l];

  /* fill the standard descriptor */
  if (flags & ELASTIC_ENABLE_GUARANTEE)
    lev->l.public_guarantee = ELASTIC_public_guarantee;
  else
    lev->l.public_guarantee = NULL;
  lev->l.public_create    = ELASTIC_public_create;
  lev->l.public_detach    = ELASTIC_public_detach;
  lev->l.public_end       = ELASTIC_public_end;
  lev->l.public_eligible  = ELASTIC_public_eligible;
  lev->l.public_dispatch  = ELASTIC_public_dispatch;
  lev->l.public_epilogue  = ELASTIC_public_epilogue;
  lev->l.public_activate  = ELASTIC_public_activate;
  lev->l.public_unblock   = ELASTIC_public_unblock;
  lev->l.public_block     = ELASTIC_public_block;
  lev->l.public_message   = ELASTIC_public_message;

  /* fill the ELASTIC task descriptor part */
  for (i=0; i<MAX_PROC; i++) {
     NULL_TIMESPEC(&(lev->elist[i].dline));
     lev->elist[i].Tmin = 0;
     lev->elist[i].Tmax = 0;
     lev->elist[i].T = 0;
     lev->elist[i].U = 0;
     lev->elist[i].C = 0;
     lev->elist[i].E = 0;
     lev->elist[i].beta = 0;
     lev->elist[i].flags = 0;
  }

  lev->c_scaling_factor = SCALING_UNIT;

  lev->U = U;

  lev->scheduling_level = master;

  lev->current_level = l;

  lev->flags = 0;

  return l;
}



/* Force the period of task p to a given value (between Tmin and Tmax) */

int ELASTIC_set_period(PID p, TIME period) {

  SYS_FLAGS f;
  int saveE;          
  unsigned int temp; 
  ext_bandwidth_t saveU;
  TIME saveT;

  f = kern_fsave();

  ELASTIC_level_des *lev;
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];

  if (period < et->Tmin || period > et->Tmax) {
    kern_frestore(f);
    return -1;
  }

  saveE = et->E;
  saveU = et->U;
  saveT = et->T;

  et->E = 0;  /* set elasticity to zero to force period */
  mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,temp);
  et->U = ((long long)MAX_BANDWIDTH * (long long)(temp))/((long long)period);
  et->T = period;
  
  if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
    cprintf("ELASTIC_set_period failed!\n");
#endif
    et->E = saveE;
    et->U = saveU;
    et->T = saveT;
    kern_frestore(f);
    return -1;
  }

  et->E = saveE;     /* Restore E when compression is done */
  kern_frestore(f);
  return 0;
}

int ELASTIC_get_period(PID p) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev;
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  TIME retval;

  f = kern_fsave();

  if (lev->elist[p].flags & ELASTIC_PRESENT) {  
    retval = lev->elist[p].T;
    kern_frestore(f);
    return retval;

  } else {

    kern_frestore(f);
    return -1;

  }

}


int ELASTIC_set_Tmin(PID p, TIME Tmin)
{
  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];
  TIME saveTmin;
  TIME saveT;
  ext_bandwidth_t saveU;
  unsigned int temp; 

  f = kern_fsave();
  
  if (et->flags & ELASTIC_PRESENT) {
  
    saveTmin = et->Tmin;
    saveT = et->T;
    saveU = et->U;
    
    et->Tmin = Tmin;
    if (Tmin > et->T) {
      et->T = Tmin;
      mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,temp);
      et->U = ((long long)MAX_BANDWIDTH * (long long)(temp))/((long long)Tmin);
    }

    if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
      cprintf("ELASTIC_set_Tmin failed: could not compress\n");
#endif
      et->Tmin = saveTmin;
      et->T = saveT;
      et->U = saveU;
      kern_frestore(f);
      return -1;
    }
    
    kern_frestore(f);
    return 0;
    
  } else {

    kern_frestore(f);
    return -1;
  }
}


int ELASTIC_get_Tmin(PID p) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev;
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  TIME retval;

  f = kern_fsave();

  if (lev->elist[p].flags & ELASTIC_PRESENT) {  
    retval = lev->elist[p].Tmin;
    kern_frestore(f);
    return retval;

  } else {

    kern_frestore(f);
    return -1;

  }

}


int ELASTIC_set_Tmax(PID p, TIME Tmax)
{
  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];
  TIME saveTmax;
  TIME saveT;
  ext_bandwidth_t saveU;
  unsigned int temp; 

  f = kern_fsave();
  
  if (et->flags & ELASTIC_PRESENT) {
  
    saveTmax = et->Tmax;
    saveT = et->T;
    saveU = et->U;
    
    et->Tmax = Tmax;
    if (Tmax < et->T) {
      et->T = Tmax;
      mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,temp);
      et->U = ((long long)MAX_BANDWIDTH * (long long)(temp))/((long long)Tmax);
    }

    if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
      cprintf("ELASTIC_set_Tmax failed: could not compress\n");
#endif
      et->Tmax = saveTmax;
      et->T = saveT;
      et->U = saveU;
      kern_frestore(f);
      return -1;
    }
    
    kern_frestore(f);
    return 0;
    
  } else {

    kern_frestore(f);
    return -1;
  }
}


int ELASTIC_get_Tmax(PID p) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev;
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  TIME retval;

  f = kern_fsave();

  if (lev->elist[p].flags & ELASTIC_PRESENT) {  
    retval = lev->elist[p].Tmax;
    kern_frestore(f);
    return retval;

  } else {

    kern_frestore(f);
    return -1;

  }

}

int ELASTIC_set_C(PID p, TIME C)
{
  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];
  TIME saveC;
  ext_bandwidth_t saveU;
  unsigned int temp; 

  f = kern_fsave();
  
  if (et->flags & ELASTIC_PRESENT) {
  
    saveC = et->C;
    saveU = et->U;
    
    et->C = C;

    mul32div32to32(et->C,lev->c_scaling_factor,SCALING_UNIT,temp);
    et->U = ((long long)MAX_BANDWIDTH * (long long)(temp))/((long long)et->T);

    if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
      cprintf("ELASTIC_set_C failed: could not compress\n");
#endif
      et->C = saveC;
      et->U = saveU;
      kern_frestore(f);
      return -1;
    }
    
    kern_frestore(f);
    return 0;
    
  } else {

    kern_frestore(f);
    return -1;
  }
}


int ELASTIC_get_C(PID p) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev;
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  TIME retval;

  f = kern_fsave();

  if (lev->elist[p].flags & ELASTIC_PRESENT) {  
    retval = lev->elist[p].C;
    kern_frestore(f);
    return retval;

  } else {

    kern_frestore(f);
    return -1;

  }

}


int ELASTIC_set_E(PID p, int E)
{
  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];
  int saveE;

  f = kern_fsave();
  
  if (et->flags & ELASTIC_PRESENT) {
  
    saveE = et->E;
    
    et->E = E;
    if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
      cprintf("ELASTIC_set_E failed: could not compress\n");
#endif
      et->E = saveE;
      kern_frestore(f);
      return -1;
    }
    
    kern_frestore(f);
    return 0;
    
  } else {

    kern_frestore(f);
    return -1;
  }
}

int ELASTIC_get_E(PID p) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev;
  lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];

  f = kern_fsave();

  if (lev->elist[p].flags & ELASTIC_PRESENT) {

    kern_frestore(f);
    return lev->elist[p].E;

  } else {

    kern_frestore(f);
    return -1;
  }
}

int ELASTIC_set_beta(PID p, int beta) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  ELASTIC_task_descr *et = &lev->elist[p];
  int saveBeta;

  f = kern_fsave();

  if (et->flags & ELASTIC_PRESENT) {

    saveBeta = et->beta;

    et->beta = beta;

    if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
      cprintf("ELASTIC_set_beta failed: could not compress\n");
#endif
      et->beta = saveBeta;
      kern_frestore(f);
      return -1;
    }

    kern_frestore(f);
    return 0;

  } else {

    kern_frestore(f);
    return -1;

  }

}

int ELASTIC_get_beta(PID p) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[proc_table[p].task_level];
  int retval;

  f = kern_fsave();

  if (lev->elist[p].flags & ELASTIC_PRESENT) {
    retval = lev->elist[p].beta;
    kern_frestore(f);
    return retval;

  } else {

    kern_frestore(f);
    return -1;

  }

}

int ELASTIC_set_bandwidth(LEVEL level, ext_bandwidth_t U) {

  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[level];
  
  f = kern_fsave();
  
  lev->U = U;

  if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
    cprintf("ELASTIC_set_bandwidth failed: could not compress\n");
#endif
    kern_frestore(f);
    return -1;
  }

  kern_frestore(f);
  return 0;

}

ext_bandwidth_t ELASTIC_get_bandwidth(LEVEL level) {

  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[level];;
  
  return lev->U;

}

int ELASTIC_set_scaling_factor(LEVEL level, int scaling_factor) {
                                                                                                                             
  SYS_FLAGS f;
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[level];
                                                                                                                             
  f = kern_fsave();
                                                                                                                             
  lev->c_scaling_factor = scaling_factor;
                                                                                                                             
  if (ELASTIC_compress(lev) == -1) {
#ifdef ELASTIC_DEBUG
    cprintf("ELASTIC_set_scaling_factor failed: could not compress\n");
#endif
    kern_frestore(f);
    return -1;
  }
                                                                                                                             
  kern_frestore(f);
  return 0;
                                                                                                                             
}

int ELASTIC_get_scaling_factor(LEVEL level) {
                                                                                                                             
  ELASTIC_level_des *lev = (ELASTIC_level_des *)level_table[level];;
                                                                                                                             
  return lev->c_scaling_factor;
                                                                                                                             
}
