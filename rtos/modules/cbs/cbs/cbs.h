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
 CVS :        $Id: cbs.h,v 1.1 2005/02/25 10:53:02 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:02 $
 ------------

 This file contains the aperiodic server CBS (Total Bandwidth Server)

 Title:
   CBS (Constant Bandwidth Server)

 Task Models Accepted:
   SOFT_TASK_MODEL - Soft Tasks
     wcet field is ignored
     met field must be != 0
     period field must be != 0
     periodicity field can be either PERIODIC or APERIODIC
     arrivals field can be either SAVE or SKIP

 Description:
   This module schedule his tasks following the CBS scheme.
   (see Luca Abeni and Giorgio Buttazzo,
        "Integrating Multimedia Applications in Hard Real-Time Systems"
        Proceedings of the IEEE Real-Time Systems Symposium, Madrid, Spain,
        December 1998)

   The tasks are inserted in an EDF level (or similar) with a JOB_TASK_MODEL,
   and the CBS level expects that the task is scheduled with the absolute
   deadline passed in the model.

   The task guarantee is based on the factor utilization approach.

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests. When a guest operation
     is called, the exception is raised.

   These exceptions are pclass-dependent...
   XDEADLINE_MISS
     If a task miss his deadline, the exception is raised.
     Normally, a CBS task can't cause the raise of such exception because
     if it really use more time than declared the deadline is postponed.

 Restrictions & special features:
   - This level doesn't manage the main task.
   - At init time we have to specify:
       . guarantee check
          (when all task are created the system will check that the task_set
          will not use more than the available bandwidth)
   - A function to return the used bandwidth of the level is provided.
   - A function to return the pending activations of the task.

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


#ifndef __CBS_H__
#define __CBS_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+ flags... +*/
#define CBS_DISABLE_ALL           0  /*+ Task Guarantee enabled +*/
#define CBS_ENABLE_GUARANTEE      1  /*+ Task Guarantee enabled +*/
#define CBS_ENABLE_ALL            1

/*+ Registration function:
    int flags     Options to be used in this level instance...
    LEVEL master  the level that must be used as master level for the
                  CBS tasks

    returns the level number at which the module has been registered.
+*/
LEVEL CBS_register_level(int flags, LEVEL master);

/*+ Returns the used bandwidth of a level +*/
bandwidth_t CBS_usedbandwidth(LEVEL l);

/*+ Returns the number of pending activations of a task.
    No control is done if the task is not a CBS task! +*/
int CBS_get_nact(LEVEL l, PID p);

__END_DECLS
#endif
