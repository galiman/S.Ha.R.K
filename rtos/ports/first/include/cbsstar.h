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


/*
 ------------
 CVS :        $Id: cbsstar.h,v 1.6 2004/09/09 10:36:08 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.6 $
 Last update: $Date: 2004/09/09 10:36:08 $
 ------------

 This file contains the budget support for the multiapplication
 scheduling algorithm proposed in the framework of the FIRST Project

 Title:
   CBSSTAR

 Task Models Accepted:
   None!

 Guest Models Accepted:
   BUDGET_TASK_MODEL - A task that is attached to a budget
     int b; --> the number of the budget which the task is attached to

 Description:
   This module schedule its tasks following the CBS scheme.
   Every task is inserted using the guest calls.
   
   The module defines a limited set of budgets that the application
   can use. Every guest task will use a particular budget; FIFO
   scheduling is used inside a budget to schedule more than one ready
   task attached to the same budget.

   The tasks are inserted in an EDF level (or similar) with a JOB_TASK_MODEL,
   and the CBS level expects that the task is scheduled with the absolute
   deadline passed in the model.

   This module tries to implement a simplified version of the guest
   task interface: 
   - To insert a guest task, use guest_create
   - When a task is dispatched, use guest_dispatch
   - When a task have to be suspended, you have to use:
     -> preemption: use guest_epilogue
     -> synchronization, end: use guest_end
   Remember: no check is done on the budget number passed with the model!!!

 Exceptions raised:
   XUNVALID_TASK
     This level doesn't support normal tasks, but just guest tasks.
     When a task operation is called, an exception is raised.

 Restrictions & special features:
   - This level doesn't manage the main task.
   - At init time we have to specify:
       . guarantee check
          (when all task are created the system will check that the task_set
          will not use more than the available bandwidth)
   - A function to return the used bandwidth of the level is provided.

   - A function is provided to allocate a buffer.
*/

/*
 * Copyright (C) 2002 Paolo Gai
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


#ifndef __CBSSTAR_H__
#define __CBSSTAR_H__

#include <kernel/kern.h>

//#include <ll/ll.h>
//#include <kernel/config.h>
//#include <sys/types.h>
//#include <kernel/types.h>
//#include <modules/codes.h>

/* -----------------------------------------------------------------------
   BUDGET_TASK_MODEL: a model for guest tasks
   ----------------------------------------------------------------------- */

#define BUDGET_PCLASS 0x0600				
			
typedef struct {
  TASK_MODEL t;
  int b;
} BUDGET_TASK_MODEL;

#define budget_task_default_model(m,buf)                          \
                        task_default_model((m).t, BUDGET_PCLASS), \
                        (m).b    = (buf);



/* some constants for registering the Module in the right place */
#define CBSSTAR_LEVELNAME       "CBSSTAR"
#define CBSSTAR_LEVEL_CODE       106
#define CBSSTAR_LEVEL_VERSION    1

typedef struct {
  int command;
  void *param;
} CBSSTAR_command_message;

typedef struct {
  int budget;
  TIME T,Q;
} CBSSTAR_mod_budget;

/* Registration function:
    int N         Maximum number of budgets allocated for the applications
    LEVEL master  the level that must be used as master level for the
                  CBS tasks
*/
LEVEL CBSSTAR_register_level(int n, LEVEL master);

/* Allocates a budget to be used for an application.
   Input parameters:
     Q The budget
     T The period of the budget
   Return value:
     0..N The ID of the budget
     -1   no more free budgets
     -2   The budgets allocated locally to this module have bandwidth > 1
     -3   wrong LEVEL id 
*/
int CBSSTAR_setbudget(LEVEL l, TIME Q, TIME T, TIME D, LEVEL local_scheduler_level, int scheduler_id);

int CBSSTAR_removebudget(LEVEL l, int budget);

int CBSSTAR_adjust_budget(LEVEL l, TIME Q, TIME T, TIME D, int budget);

int CBSSTAR_getbudgetinfo(LEVEL l, TIME *Q, TIME *T, TIME *D, int budget);

int CBSSTAR_was_budget_overran(LEVEL l, int budget);

int CBSSTAR_is_active(LEVEL l, int budget);

int CBSSTAR_get_local_scheduler_level_from_budget(LEVEL l, int budget);

int CBSSTAR_get_local_scheduler_level_from_pid(LEVEL l, PID p);

int CBSSTAR_get_local_scheduler_id_from_budget(LEVEL l, int budget);

int CBSSTAR_get_local_scheduler_id_from_pid(LEVEL l, PID p);

int CBSSTAR_get_last_reclaiming(LEVEL l, PID p);

int CBSSTAR_get_remain_capacity(LEVEL l, int budget);

void CBSSTAR_disable_server(LEVEL l, int budget);

#endif
