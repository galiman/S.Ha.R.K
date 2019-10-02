/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
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
 CVS :        $Id: activate.c,v 1.11 2006/06/27 08:56:57 tullio Exp $

 File:        $File$
 Revision:    $Revision: 1.11 $
 Last update: $Date: 2006/06/27 08:56:57 $
 ------------

 task_activate & group_activate

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

#include <stdarg.h>
#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <tracer.h>

/*+
  Activates a single task now
+*/
int task_activate(PID p)
{
  struct timespec t;
  kern_gettime(&t);
  return task_activate_at(p, &t);
}

/*+
  Activates a single task at time t
+*/
int task_activate_at(PID p, struct timespec *t)
{
    LEVEL l;  /* the level of the task p */

    /* some controls on the task p */
    if (p<0 || p>=MAX_PROC) {
	errno = EINVALID_TASK_ID;
	return -1;
    }
    if (proc_table[p].status == FREE) {
	errno = EINVALID_TASK_ID;
	return -1;
    }

    /*+ if we are calling the runlevel functions the system is
        into the global_context... we only have to call
        the task_activate of the level +*/
    if (calling_runlevel_func) {
	SYS_FLAGS f;
	f=kern_fsave();
        if (proc_table[p].control & FREEZE_ACTIVATION)
          proc_table[p].frozen_activations++;
        else {
          l = proc_table[p].task_level;
          level_table[l]->public_activate(l,p,t);
        }
	kern_frestore(f);
	return 0;
    }


    /* Begin activate */
    if (ll_ActiveInt()) {
      SYS_FLAGS f;
      f = kern_fsave();
      if (proc_table[p].control & FREEZE_ACTIVATION)
        proc_table[p].frozen_activations++;
      else {
        l = proc_table[p].task_level;
	TRACER_LOGEVENT(FTrace_EVT_task_activate,(unsigned short int)proc_table[p].context,0);
        level_table[l]->public_activate(l,p,t);
        event_need_reschedule();
      }
      kern_frestore(f);
    }
    else {
      proc_table[exec_shadow].context = kern_context_save();

      if (proc_table[p].control & FREEZE_ACTIVATION)
        proc_table[p].frozen_activations++;
      else {
	/* tracer stuff */
    	TRACER_LOGEVENT(FTrace_EVT_task_activate,(unsigned short int)proc_table[p].context,0);
        l = proc_table[p].task_level;	
        level_table[l]->public_activate(l,p,t);
    
        /* Preempt if necessary */
        scheduler();
      }
      kern_context_load(proc_table[exec_shadow].context);
    }

    return 0;
}


/*+
  Activate a group of tasks identified by the group g now.
  It returns -1 if the group is not valid
+*/
int group_activate(WORD g)
{
  struct timespec t;
  kern_gettime(&t);
  return group_activate_at(g, &t);
}

/*+
  Activate a group of tasks identified by the group g at time t.
  It returns -1 if the group is not valid
+*/
int group_activate_at(WORD g, struct timespec *t)
{
  PID i;              /* a counter */
  register LEVEL l;   /* a level value */

  if (g == 0) {
    errno = EINVALID_GROUP;
    return -1;
  }

  /*+ if we are calling the runlevel functions the system is
      into the global_context... we only have to call
      the task_activate of the level +*/
  if (calling_runlevel_func) {
    SYS_FLAGS f;
    f = kern_fsave();

    for (i = 0; i < MAX_PROC; i++) {
      if (proc_table[i].status != FREE) {
        if (proc_table[i].group == g) {
          if (proc_table[i].control & FREEZE_ACTIVATION) {
            proc_table[i].frozen_activations++;
            continue;
          }
          /* tracer stuff */
          TRACER_LOGEVENT(FTrace_EVT_task_activate, (unsigned short int)proc_table[i].context, 0);        
          l = proc_table[i].task_level;
          level_table[l]->public_activate(l,i,t);
        }
      }
    }

    kern_frestore(f);
    return 0;
  }

  if (ll_ActiveInt()) {
    SYS_FLAGS f;
    f = kern_fsave();
    for (i = 0; i < MAX_PROC; i++) {
      if (proc_table[i].status != FREE) {
        if (proc_table[i].group == g) {
          if (proc_table[i].control & FREEZE_ACTIVATION) {
            proc_table[i].frozen_activations++;
            continue;
          }
          /* tracer stuff */
          TRACER_LOGEVENT(FTrace_EVT_task_activate, (unsigned short int)proc_table[i].context, 0);        
          l = proc_table[i].task_level;
          level_table[l]->public_activate(l,i,t);
          event_need_reschedule();
        }
      }
    }
    kern_frestore(f);
  }
  else {
    proc_table[exec_shadow].context = kern_context_save();

    for (i = 0 ; i < MAX_PROC; i++) {
      if (proc_table[i].status != FREE) {
        if (proc_table[i].group == g) {
          if (proc_table[i].control & FREEZE_ACTIVATION) {
            proc_table[i].frozen_activations++;
            continue;
          }
          l = proc_table[i].task_level;
          level_table[l]->public_activate(l,i,t);
          /* tracer stuff */
          TRACER_LOGEVENT(FTrace_EVT_task_activate,(unsigned short int)proc_table[i].context,0);
        }
      }
    }
  
    scheduler();
    kern_context_load(proc_table[exec_shadow].context);
  }
  return 0;
}
