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
#include "mpegstar.h"
#include "fsf_contract.h"
#include "fsf_server.h"

//#define MPEGSTAR_DEBUG

/*+ Status used in the level +*/
#define MPEGSTAR_READY   MODULE_STATUS_BASE

#define FRAME_IDLE 0
#define FRAME_DECODING 1
#define FRAME_SKIPPED 2

typedef struct {
  level_des l;          /*+ the standard level descriptor          +*/

  int server_Q;
  int server_T;

  int budget;
  int current;
  int status;
  
  int scheduling_level;

} MPEGSTAR_level_des;

static int MPEGSTAR_public_eligible(LEVEL l, PID p)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
  return level_table[ lev->scheduling_level ]->
      private_eligible(lev->scheduling_level,p);
  
  return 0;
}

static int MPEGSTAR_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

  #ifdef MPEGSTAR_DEBUG
    kern_printf("(MP:Crt:%d)",p);
  #endif

  if (m->pclass != HARD_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  if (lev->current != NIL) return -1; 

  lev->current = p;
  lev->status = FRAME_IDLE;
 
  proc_table[p].avail_time = lev->server_Q;
  proc_table[p].wcet       = lev->server_Q;                                                                                    
  proc_table[p].control = (proc_table[p].control & ~CONTROL_CAP);                  
  
  return 0; /* OK */

}

static void MPEGSTAR_public_dispatch(LEVEL l, PID p, int nostop)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

  #ifdef MPEGSTAR_DEBUG
     kern_printf("(MS:Dsp:%d)",p);
  #endif

  if (!nostop) {
    level_table[ lev->scheduling_level ] -> 
      private_dispatch(lev->scheduling_level, p, nostop);
  }

}

static void MPEGSTAR_public_epilogue(LEVEL l, PID p)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
  int r;

  #ifdef MPEGSTAR_DEBUG
     kern_printf("(MS:Epi:%d)",p);
  #endif

  level_table[ lev->scheduling_level ] -> 
    private_epilogue(lev->scheduling_level,p);

  r = SERVER_get_remain_capacity(lev->scheduling_level,lev->budget);
  if (r < 0) {

    #ifdef MPEGSTAR_DEBUG
      kern_printf("(MS:FRAME SKIP %d)",p);
    #endif

    lev->status = FRAME_SKIPPED;

    level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level, p);

  }  

}

static void MPEGSTAR_public_activate(LEVEL l, PID p)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
  BUDGET_TASK_MODEL b;

  budget_task_default_model(b, lev->budget);
     
  #ifdef MPEGSTAR_DEBUG
    kern_printf("(MP:Act:%d:%d)",p,lev->budget);
  #endif

  lev->status = FRAME_DECODING;

  level_table[ lev->scheduling_level ]->
    private_insert(lev->scheduling_level, p, (TASK_MODEL *)&b);

}

static void MPEGSTAR_public_unblock(LEVEL l, PID p)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

}

static void MPEGSTAR_public_block(LEVEL l, PID p)
{  
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

}

static int MPEGSTAR_public_message(LEVEL l, PID p, void *m)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

  switch ((long)(m)) {

    /* Task EndCycle */
    case (long)(NULL):

      #ifdef MPEGSTAR_DEBUG
        kern_printf("(MS:EndCycle:%d)",p);
      #endif

      lev->status = FRAME_IDLE;

      level_table[ lev->scheduling_level ]->
        private_extract(lev->scheduling_level, p);

      level_table[ lev->scheduling_level ]->
        public_message(lev->scheduling_level, p, NULL);

      break;

    /* Task Disable */
    case (long)(1):

      break; 

    default:

      break;
         
  }

  return 0;

}

static void MPEGSTAR_public_end(LEVEL l, PID p)
{
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
 
  #ifdef MPEGSTAR_DEBUG
    kern_printf("(MS:End:%d)", p);
  #endif

  lev->current = NIL;
  lev->status = FRAME_IDLE;
  proc_table[p].status = FREE;

}

/* Registration functions */

/*+ Registration function:
    TIME slice                the slice for the Round Robin queue +*/
LEVEL MPEGSTAR_register_level(int master)
{
  LEVEL l;            /* the level that we register */
  MPEGSTAR_level_des *lev;  /* for readableness only */
 
  l = level_alloc_descriptor(sizeof(MPEGSTAR_level_des));

  lev = (MPEGSTAR_level_des *)level_table[l];

  lev->l.public_guarantee = NULL;
  lev->l.public_create    = MPEGSTAR_public_create;
  lev->l.public_end       = MPEGSTAR_public_end;
  lev->l.public_dispatch  = MPEGSTAR_public_dispatch;
  lev->l.public_epilogue  = MPEGSTAR_public_epilogue;
  lev->l.public_activate  = MPEGSTAR_public_activate;
  lev->l.public_unblock   = MPEGSTAR_public_unblock;
  lev->l.public_block     = MPEGSTAR_public_block;
  lev->l.public_message   = MPEGSTAR_public_message;
  lev->l.public_eligible  = MPEGSTAR_public_eligible;

  lev->budget = NIL; 
  lev->current = NIL;
  lev->status = FRAME_IDLE;
  lev->server_Q = 0;
  lev->server_T = 0;

  lev->scheduling_level = master;

  return l;

}

int MPEGSTAR_setbudget(LEVEL l, PID p, int budget)
{
                                                                                                                             
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
                                                                                                                             
  lev->budget = budget;
                                                                                                                             
  return 0;
                                                                                                                             
}
                                                                                                                             
int MPEGSTAR_getbudget(LEVEL l, PID p)
{
                                                                                                                             
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
                                                                                                                             
  return lev->budget;
                                                                                                                             
}

int MPEGSTAR_budget_has_thread(LEVEL l, int budget)
{                                                                                                                             
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);
  
  if (lev->budget == budget) return 1;                                                                                                            
  return 0;

}

int MPEGSTAR_rescale(int budget, TIME Q, TIME T)
{
  LEVEL l = SERVER_get_local_scheduler_level_from_budget(fsf_get_server_level(),budget);
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

  SERVER_adjust_budget(lev->scheduling_level,Q,T,T,lev->budget);
  lev->server_Q = Q;
  lev->server_T = T;
  if (lev->current != NIL) {
    proc_table[lev->current].avail_time = Q;
    proc_table[lev->current].wcet = Q;
  }

  return 0;
}

int MPEGSTAR_get_remain_capacity(int budget)
{
  return SERVER_get_remain_capacity(fsf_get_server_level(),budget);
}

int MPEGSTAR_get_last_reclaiming(int budget)
{
  return SERVER_get_last_reclaiming(fsf_get_server_level(),exec_shadow);
}

int MPEGSTAR_is_frame_skipped(int budget)
{
  LEVEL l = SERVER_get_local_scheduler_level_from_budget(fsf_get_server_level(),budget);
  MPEGSTAR_level_des *lev = (MPEGSTAR_level_des *)(level_table[l]);

  if (lev->status == FRAME_SKIPPED)
    return 1;
  else
    return 0;

}
