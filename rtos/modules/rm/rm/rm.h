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
 *   Anton Cervin
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */


/**
 ------------
 CVS :        $Id: rm.h,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains the scheduling module RM (rate-/deadline-monotonic)

 Title:
   RM (rate-/deadline-monotonic)

 Task Models Accepted:
   HARD_TASK_MODEL - Hard Tasks (Periodic and Sporadic)
     wcet field and mit field must be != 0. They are used to set the wcet
       and period of the tasks.
     periodicity field can be either PERIODIC or APERIODIC
     drel field must be <= mit. NOTE 1: a drel of 0 is interpreted as mit.
       NOTE 2: The utilization of the task is computed as wcet/drel.
     offset field specifies a release offset relative to task_activate or
       group_activate.

 Guest Models Accepted:
   JOB_TASK_MODEL - a single guest task activation
     Identified by an absolute deadline and a period.
     period field is ignored

 Description:
   This module schedules periodic and sporadic tasks based on their
   relative deadlines. The task guarantee is based on a simple
   utilization approach. The utilization factor of a task is computed
   as wcet/drel. (By default, drel = mit.) A periodic task must only
   be activated once; subsequent activations are triggered by an
   internal timer. By contrast, an sporadic task must be explicitely
   activated for each instance. NO GUARANTEE is performed on guest
   tasks. The guarantee must be performed by the level that inserts
   guest tasks in the RM level.

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests of this type. When a guest
     operation is called, the exception is raised.

   The following exceptions may be raised by the module:
   XDEADLINE_MISS
     If a task misses its deadline and the RM_ENABLE_DL_EXCEPTION
     flag is set, this exception is raised.

   XWCET_VIOLATION 
     If a task executes longer than its declared wcet and the
     RM_ENABLE_WCET_EXCEPTION flag is set, this exception is raised.

   XACTIVATION
     If a sporadic task is activated more often than its declared mit
     and the RM_ENABLE_ACT_EXCEPTION flag is set, this exception is
     raised. This exception is also raised if a periodic task is
     activated while not in the SLEEP state.

 Restrictions & special features:

   - Relative deadlines drel <= mit may be specified.
   - An offset > 0 will delay the activation of the task by the same
     amount of time. To synchronize a group of tasks, assign suitable
     offsets and then use the group_activate function.
   - This level doesn't manage the main task.
   - The level uses the priority and timespec_priority fields.
   - The guest tasks don't provide the guest_endcycle function.
   - At init time, the user can specify the behavior in case of
     deadline and wcet overruns. The following flags are available:

     (No flags enabled)      - Deadline and wcet overruns are ignored.
                               Pending periodic jobs are queued and are
			       eventually scheduled with correct deadlines
			       according to their original arrival times.
			       Sporadic tasks that arrive to often are
			       simply dropped.
     RM_ENABLE_DL_CHECK     -  When a deadline overrun occurs, the 
                               dl_miss counter of the task is increased.
                               Same behavior for pending jobs as above.
     RM_ENABLE_WCET_CHECK   -  When a wcet overrun occurs, the 
                               wcet_miss counter of the task is increased.
                               Same behavior for pending jobs as above.
     RM_ENABLE_DL_EXCEPTION -  When a deadline overrun occurs, an
                               exception is raised.
     RM_ENABLE_WCET_EXCEPTION - When a wcet overrun occurs, an
                               exception is raised.
     RM_ENABLE_ACT_EXCEPTION   When a periodic or sporadic task is activated
                               too often, an exception is raised.

   - The functions RM_get_dl_miss, RM_get_wcet_miss, RM_get_act_miss,
     and RM_get_nact can be used to find out the number of missed
     deadlines, the number of wcet overruns, the number of skipped
     activations, and the number of currently queued periodic activations.

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


#ifndef __RM_H__
#define __RM_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* Level flags */
#define RM_DISABLE_ALL            0
#define RM_ENABLE_GUARANTEE       1  /* Task guarantee enabled             */
#define RM_ENABLE_WCET_CHECK      2  /* Wcet monitoring enabled            */
#define RM_ENABLE_DL_CHECK        4  /* Deadline monitoring enabled        */
#define RM_ENABLE_WCET_EXCEPTION  8  /* Wcet overrun exception enabled     */
#define RM_ENABLE_DL_EXCEPTION   16  /* Deadline overrun exception enabled */
#define RM_ENABLE_ACT_EXCEPTION  32  /* Activation exception enabled       */
#define RM_ENABLE_ALL            63  /* All flags enabled                  */

/* Registration function */
LEVEL RM_register_level(int flags);


/**** Public utility functions ****/

/* Get the bandwidth used by the level */
bandwidth_t RM_usedbandwidth(LEVEL l);

/* Get the number of missed deadlines for a task */
int RM_get_dl_miss(PID p);

/* Get the number of execution overruns for a task */
int RM_get_wcet_miss(PID p);

/* Get the number of skipped activations for a task */
int RM_get_act_miss(PID p);

/* Get the current number of queued activations for a task */
int RM_get_nact(PID p);


__END_DECLS
#endif
