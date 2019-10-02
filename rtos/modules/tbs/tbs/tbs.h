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
 CVS :        $Id: tbs.h,v 1.1 2005/02/25 10:53:02 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:02 $
 ------------

 This file contains the aperiodic server TBS (Total Bandwidth Server)

 Title:
   TBS (Total Bandwidth Server)

 Task Models Accepted:
   SOFT_TASK_MODEL - Aperiodic Tasks
     wcet field must be != 0
     met field is ignored
     period field is ignored
     periodicity must be APERIODIC
     arrivals can be either SAVE or SKIP

 Description:
   This module schedule his tasks following the TBS scheme.
   Each task has a deadline assigned with the TBS scheme,

                       wcet
   d = max(r , d   ) + ----
    k       k   k-1     Us

   The tasks are inserted in an EDF level (or similar) with a JOB_TASK_MODEL,
   and the TBS level expects that the task is scheduled with the absolute
   deadline passed in the model.

   The task guarantee is based on the factor utilization approach.
   The theory guarantees that the task set is schedulable if
    Up + Us <= 1
   so it is sufficient to add the Us to the bandwidth used by the upper
   levels (we suppose that the levels with level number < of the current
   can guarantee their task sets basing on the same formula...

   All the tasks are put in a FIFO (FCFS) queue and at a time only the first
   task in the queue is put in the upper level.

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests. When a guest operation
     is called, the exception is raised.

   These exceptions are pclass-dependent...
   XDEADLINE_MISS
     If a task miss his deadline, the exception is raised.
     Normally, a TBS task can't cause the raise of such exception because
     if it really use more time than declared a XWCET_VIOLATION is raised
     instead.

   XWCET_VIOLATION
     If a task doesn't end the current cycle before if consume the wcet,
     an exception is raised, and the task is put in the TBS_WCET_VIOLATED
     state. To reactivate it, use TBS_task_activate via task_activate or
     manage directly the TBS data structure. Note that if the exception
     is not handled properly, an XDEADLINE_MISS exception will also be
     raised at the absolute deadline...

 Restrictions & special features:
   - This level doesn't manage the main task.
   - At init time we have to specify:
     . The bandwidth used by the server
     . some flags
       . wcet check activated
          (If a task require more time than declared, it is stopped and put in
          the state TBS_WCET_VIOLATED; a XWCET_VIOLATION exception is raised)
       . guarantee check
          (when all task are created the system will check that the task_set
          will not use more than the available bandwidth)
   - The level don't use the priority field.
   - A function to return the used bandwidth of the level is provided.

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


#ifndef __TBS_H__
#define __TBS_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+ flags... +*/
#define TBS_DISABLE_ALL           0
#define TBS_ENABLE_WCET_CHECK     1  /*+ Wcet check enabled     +*/
#define TBS_ENABLE_GUARANTEE      2  /*+ Task Guarantee enabled +*/
#define TBS_ENABLE_ALL            3  /*+ All flags enabled      +*/


/*+ Registration function:
    bandwidth_t b Max bandwidth used by the TBS
    int flags     Options to be used in this level instance...
    LEVEL master  the level that must be used as master level for the
                  TBS tasks
    int num,den   used to compute the TBS bandwidth                      +*/
void TBS_register_level(int flags, LEVEL master, int num, int den);

/*+ Returns the used bandwidth of a level +*/
bandwidth_t TBS_usedbandwidth(LEVEL l);

/*+ Returns the number of pending activations of a task, or -1 if the level
    is wrong.
    No control is done if the task is not a TBS task! +*/
int TBS_get_nact(LEVEL l, PID p);

__END_DECLS
#endif
