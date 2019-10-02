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
 CVS :        $Id: edfstar.h,v 1.3 2004/09/01 08:44:55 trimarchi Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2004/09/01 08:44:55 $
 ------------

 Title:
   EDFSTAR

 Task Models Accepted:
   HARD_TASK_MODEL - Hard Tasks (only Periodic)
     wcet field and mit field must be != 0. They are used to set the wcet
       and period of the tasks.
     periodicity field can be only PERIODIC
     drel field is ignored
    
 Guest Models Accepted:
   JOB_TASK_MODEL - a single guest task activation
     Identified by an absolute deadline and a period.
     period field is ignored

 Description:

   This module schedule his tasks following the classic EDF
   scheme. This module is derived from the EDFACT Scheduling Module.

   This module can not stay alone: when it have to schedule a task, it
   simply inserts it into another master module using a
   BUDGET_TASK_MODEL.

   No Task guarantee is performed at all.
   The tasks scheduled are only periodic.
   All the task are put in a queue and the scheduling is based on the
   deadline value.
   If a task miss a deadline a counter is incremented.
   If a task exausts the wcet a counter is incremented
   No ZOMBIE support!!!!!!

 Exceptions raised:
   XUNVALID_GUEST XUNVALID_TASK
     some primitives are not implemented:
     task_sleep, task_delay, guest_endcycle, guest_sleep, guest_delay

   XACTIVATION
     If a task is actiated through task_activate or guest_activate more than
     one time
     
 Restrictions & special features:
   - This level doesn't manage the main task.
   - Functions to return and reset the nact, wcet and dline miss
     counters are provided

**/

/*
 * Copyright (C) 2001 Paolo Gai
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


#ifndef __EDFSTAR_H__
#define __EDFSTAR_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>


typedef struct {
  int command;
  void *param;
} EDFSTAR_command_message;

/* flags... */
#define EDFSTAR_ENABLE_GUARANTEE      1  /* Task Guarantee enabled */
#define EDFSTAR_ENABLE_ALL            1

#define EDFSTAR_FAILED_GUARANTEE      8  /* used in the module, unsettabl
                                         in EDF_register_level... */



#define EDFSTAR_LEVELNAME        "EDFSTAR base"
#define EDFSTAR_LEVEL_CODE       166
#define EDFSTAR_LEVEL_VERSION    1


/* Registration function:
   int budget      The budget used by this module (see CBSSTAR.h)
   int master      The master module used by EDFSTAR
*/
LEVEL EDFSTAR_register_level(int master);

/* returns respectively the number of dline, wcet or nact; -1 if error */
int EDFSTAR_get_dline_miss(PID p);
int EDFSTAR_get_wcet_miss(PID p);
int EDFSTAR_get_nact(PID p);

/* resets respectively the number of dline, wcet miss; -1 if error */
int EDFSTAR_reset_dline_miss(PID p);
int EDFSTAR_reset_wcet_miss(PID p);

int EDFSTAR_getbudget(LEVEL l, PID p);
int EDFSTAR_setbudget(LEVEL l, PID p, int budget);
int EDFSTAR_budget_has_thread(LEVEL l, int budget);

void EDFSTAR_set_nopreemtive_current(LEVEL l);
void EDFSTAR_unset_nopreemtive_current(LEVEL l);

#endif

