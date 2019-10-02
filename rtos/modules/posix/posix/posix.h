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
 CVS :        $Id: posix.h,v 1.1 2005/02/25 10:46:36 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:46:36 $
 ------------

 This file contains the scheduling module compatible with POSIX
 specifications

 Title:
   POSIX version 1

 Task Models Accepted:
   NRT_TASK_MODEL - Non-Realtime Tasks
     weight field is ignored
     slice  field is used to set the slice of a task, if it is !=0
     policy field is ignored
     inherit field is ignored

 Description:
   This module schedule his tasks following the POSIX specifications...

   A task can be scheduled in a Round Robin way or in a FIFO way.
   The tasks have also a priority field.

   The slices can be different one task from one another.

   The module can SAVE or SKIP activations

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests. When a guest operation
     is called, the exception is raised.

 Restrictions & special features:
   - if specified, it creates at init time a task,
     called "Main", attached to the function __init__().
   - There must be only one module in the system that creates a task
     attached to the function __init__().
   - The level tries to guarantee that a RR task uses a "full" timeslice
     before going to the queue tail. "full" means that a task can execute
     a maximum time of slice+sys_tick due to the approx. done by
     the Virtual Machine. If a task execute more time than the slice,
     the next time it execute less...

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


#ifndef __POSIX_H__
#define __POSIX_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

extern TASK __init__(void *arg);



/*+ Const: +*/
#define POSIX_MINIMUM_SLICE    1000 /*+ Minimum Timeslice +*/
#define POSIX_MAXIMUM_SLICE  500000 /*+ Maximum Timeslice +*/

#define POSIX_MAIN_YES            1 /*+ The level creates the main +*/
#define POSIX_MAIN_NO             0 /*+ The level does'nt create the main +*/

/*+ Registration function:
    TIME slice                the slice for the Round Robin queue
    int createmain            1 if the level creates the main task 0 otherwise
    struct multiboot_info *mb used if createmain specified   

    returns the level number at which the module has been registered.
+*/
LEVEL POSIX_register_level(TIME slice,
                          int createmain,
                          struct multiboot_info *mb,
                          int prioritylevels);

/*+ this function forces the running task to go to his queue tail,
    then calls the scheduler and changes the context
    (it works only on the POSIX level) +*/
int POSIX_sched_yield(LEVEL l);

/* the following functions have to be called with interruptions DISABLED! */

/*+ this function returns the maximum level allowed for the POSIX level +*/
int POSIX_get_priority_max(LEVEL l);

/*+ this function returns the default timeslice for the POSIX level +*/
int POSIX_rr_get_interval(LEVEL l);

/*+ this functions returns some paramaters of a task;
    policy must be NRT_RR_POLICY or NRT_FIFO_POLICY;
    priority must be in the range [0..prioritylevels]
    returns ENOSYS or ESRCH if there are problems +*/
int POSIX_getschedparam(LEVEL l, PID p, int *policy, int *priority);

/*+ this functions sets paramaters of a task +*/
int POSIX_setschedparam(LEVEL l, PID p, int policy, int priority);

__END_DECLS
#endif

/*
MANCANO
13.3.6 GETPRIORITYMin da mettere a 0
*/
