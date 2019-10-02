/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Trimarchi Michael   <trimarchi@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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


#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include "nonestar.h"
#include "fsf_configuration_parameters.h"
#include "fsf_core.h"
#include "fsf_server.h"
#include "tracer.h"
#include <posix/posix/comm_message.h>

//#define NONESTAR_DEBUG

#define NONESTAR_CHANGE_LEVEL 1

/*+ Status used in the level +*/
#define NONESTAR_READY   MODULE_STATUS_BASE

#define NONESTAR_IDLE 0

typedef struct {
  level_des l;          /*+ the standard level descriptor          +*/

  int server_Q;
  int server_T;

  int budget;
  int current;
  int status;
  int flag;
  
  int scheduling_level;
  int new_level[MAX_PROC];

} NONESTAR_level_des;

static int NONESTAR_public_eligible(LEVEL l, PID p)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
  return level_table[ lev->scheduling_level ]->
      private_eligible(lev->scheduling_level,p);
  
  return 0;
}

static int NONESTAR_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
 


  if (m->pclass != DUMMY_PCLASS) return -1;

  if (m->level != 0 && m->level != l) return -1; 
  if (lev->current != NIL) return -1; 

#ifdef NONESTAR_DEBUG
  kern_printf("(NN:Crt:%d)",p);
#endif

  lev->current = p;
  lev->flag = 0;
  lev->status = NONESTAR_IDLE;
 
  proc_table[p].avail_time = lev->server_Q;
  proc_table[p].wcet       = lev->server_Q;                                                                                    
  proc_table[p].control = (proc_table[p].control & ~CONTROL_CAP);                  
  
  return 0; /* OK */

}

static void NONESTAR_public_dispatch(LEVEL l, PID p, int nostop)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);

  #ifdef NONESTAR_DEBUG
     kern_printf("(NN:Dsp:%d)",p);
  #endif

  if (!nostop) {
    level_table[ lev->scheduling_level ] -> 
      private_dispatch(lev->scheduling_level, p, nostop);
  }

}

static int NONESTAR_change_level(LEVEL l, PID p)
{

  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);

  /* Change task level */
  if (lev->flag & NONESTAR_CHANGE_LEVEL) {
    STD_command_message msg;
    
    if (lev->current!=NIL && lev->status!=NONESTAR_IDLE) { 
       level_table[ lev->scheduling_level ]->
          private_extract(lev->scheduling_level, p);
    }
    lev->current = NIL; 
    lev->budget = -1;
    proc_table[p].task_level = lev->new_level[p];
    lev->flag=0;
    
    /* Send change level command to local scheduler */

    msg.command = STD_ACTIVATE_TASK;
    msg.param = NULL;

    level_table[ lev->new_level[p] ]->public_message(lev->new_level[p],p,&msg);
   
    return 1;

  }

  return 0;

}



static void NONESTAR_public_epilogue(LEVEL l, PID p)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
  //int r;

  #ifdef NONESTAR_DEBUG
     kern_printf("(NN:Epi:%d)",p);
  #endif
  if (NONESTAR_change_level(l, p)) return;

  level_table[ lev->scheduling_level ] -> 
    private_epilogue(lev->scheduling_level,p);
/*
  r = SERVER_get_remain_capacity(lev->scheduling_level,lev->budget);
  if (r < 0) {

    #ifdef NONESTAR_DEBUG
      kern_printf("(NN:NONE END %d)",p);
    #endif

    lev->status = NONESTAR_IDLE;

    level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level, p);

  }  
*/
}

static void NONESTAR_public_activate(LEVEL l, PID p, struct timespec *o)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
  BUDGET_TASK_MODEL b;
  TIME Q;
  budget_task_default_model(b, lev->budget);
     
  #ifdef NONESTAR_DEBUG
    kern_printf("(NN:Act:%d:%d)",p,lev->budget);
  #endif
  GRUBSTAR_getbudgetinfo(lev->scheduling_level, &Q, NULL, NULL, lev->budget);
  lev->server_Q=Q;
  proc_table[p].avail_time = lev->server_Q;
  proc_table[p].wcet       = lev->server_Q;                                                                               
  lev->status = NONESTAR_READY;

  level_table[ lev->scheduling_level ]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&b);

}

static void NONESTAR_public_unblock(LEVEL l, PID p)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
  BUDGET_TASK_MODEL b;

#ifdef NONESTAR_DEBUG
  kern_printf("(NS:pu %d)",p);
#endif
  lev->current=p;

  budget_task_default_model(b, lev->budget);
  lev->status=NONESTAR_READY;
  level_table[ lev->scheduling_level ]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&b);

}

static void NONESTAR_public_block(LEVEL l, PID p)
{  
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);

#ifdef NONESTAR_DEBUG
  kern_printf("(NN:pb %d)", p);
#endif
  lev->status=NONESTAR_IDLE;
  /* the task is blocked on a synchronization primitive. we have to
     remove it from the master module -and- from the local queue! */
  lev->current=NIL;
  level_table[ lev->scheduling_level ]->
     private_extract(lev->scheduling_level, p);

}

static int NONESTAR_public_message(LEVEL l, PID p, void *m)
{
  NONESTAR_level_des  *lev = (NONESTAR_level_des *)(level_table[l]);
  STD_command_message *msg;
  DUMMY_TASK_MODEL    *h;

  switch ((long)(m)) {

    /* Task EndCycle */
    case (long)(NULL):

      #ifdef NONESTAR_DEBUG
        kern_printf("(NN:EndCycle:%d)",p);
      #endif

      if (NONESTAR_change_level(l, p)) return 0;

      lev->status = NONESTAR_IDLE;

      level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level, p);

      level_table[ lev->scheduling_level ]->
        public_message(lev->scheduling_level, p, NULL);

      TRACER_LOGEVENT(FTrace_EVT_task_end_cycle,proc_table[p].context,proc_table[p].task_level);
      jet_update_endcycle(); /* Update the Jet data... */
      break;

    /* Task Disable */
    case (long)(1):

      break; 

    default:
      msg = (STD_command_message *)m;
    
      switch(msg->command) {
      case STD_SET_NEW_MODEL:
	/* if the NONESTAR_task_create is called, then the pclass must be a
	   valid pclass. */
	
	/* now we know that m is a valid model */
	lev->flag = 0;      
	lev->current = p;
	lev->status = NONESTAR_IDLE;

	
	break;
	
      case STD_SET_NEW_LEVEL:
	
	lev->flag |= NONESTAR_CHANGE_LEVEL;
	lev->new_level[p] = (int)(msg->param);
	break;
	
      case STD_ACTIVATE_TASK:
	
      /* Enable wcet check */
	proc_table[p].avail_time = lev->server_Q;
	proc_table[p].wcet       = lev->server_Q;
	proc_table[p].control = (proc_table[p].control & ~CONTROL_CAP);                 
  
	NONESTAR_public_activate(l, p, NULL);
	
	break;
      }

      break;
      
  }



  return 0;

}

static void NONESTAR_public_end(LEVEL l, PID p)
{
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
 
  #ifdef NONESTAR_DEBUG
    kern_printf("(NN:End:%d)", p);
  #endif

  lev->current = NIL;
  lev->status = NONESTAR_IDLE;
  proc_table[p].status = FREE;

}

/* Registration functions */

/*+ Registration function:
    TIME slice                the slice for the Round Robin queue +*/
LEVEL NONESTAR_register_level(int master)
{
  LEVEL l;            /* the level that we register */
  NONESTAR_level_des *lev;  /* for readableness only */
  int i=0;
 
  l = level_alloc_descriptor(sizeof(NONESTAR_level_des));

  lev = (NONESTAR_level_des *)level_table[l];

  lev->l.public_guarantee = NULL;
  lev->l.public_create    = NONESTAR_public_create;
  lev->l.public_end       = NONESTAR_public_end;
  lev->l.public_dispatch  = NONESTAR_public_dispatch;
  lev->l.public_epilogue  = NONESTAR_public_epilogue;
  lev->l.public_activate  = NONESTAR_public_activate;
  lev->l.public_unblock   = NONESTAR_public_unblock;
  lev->l.public_block     = NONESTAR_public_block;
  lev->l.public_message   = NONESTAR_public_message;
  lev->l.public_eligible  = NONESTAR_public_eligible;

  lev->budget = NIL; 
  lev->current = NIL;
  lev->flag = 0;
  lev->status = NONESTAR_IDLE;
  lev->server_Q = 0;
  lev->server_T = 0;

  lev->scheduling_level = master;

  for(i=0; i<MAX_PROC; i++)
    lev->new_level[i]      = -1;
  
  return l;

}

int NONESTAR_setbudget(LEVEL l, PID p, int budget)
{
                                                                                                                             
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
                                                                                                                             
  lev->budget = budget;
                                                                                                                             
  return 0;
                                                                                                                             
}
                                                                                                                             
int NONESTAR_getbudget(LEVEL l, PID p)
{
                                                                                                                             
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
                                                                                                                             
  return lev->budget;
                                                                                                                             
}

int NONESTAR_budget_has_thread(LEVEL l, int budget)
{                                                                                                                             
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
  
  if (lev->budget == budget) return 1;                                                                                                            
  return 0;

}

int NONESTAR_get_current(LEVEL l) {
  NONESTAR_level_des *lev = (NONESTAR_level_des *)(level_table[l]);
  return lev->current;
}

int NONESTAR_get_remain_capacity(int budget)
{
  return SERVER_get_remain_capacity(fsf_get_server_level(),budget);
}

int NONESTAR_get_last_reclaiming(int budget)
{
  return SERVER_get_last_reclaiming(fsf_get_server_level(),exec_shadow);
}


