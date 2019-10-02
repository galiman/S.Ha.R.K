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
 CVS :        $Id: rr.h,v 1.1 2005/02/25 10:45:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:45:58 $
 ------------

 This file contains the scheduling module RR (Round Robin)

 Title:
   RR (Round Robin)

 Task Models Accepted:
   NRT_TASK_MODEL - Non-Realtime Tasks
     weight field is ignored
     slice  field is used to set the slice of a task, if it is !=0
     activation field is ignored
     policy field is ignored
     inherit field is ignored

 Description:
   This module schedule his tasks following the classic round-robin
   scheme. The default timeslice is given at registration time and is a
   a per-task specification. The default timeslice is used if the slice
   field in the NRT_TASK_MODEL is 0.

   The module always SKIP instances, either if SAVE_ARRIVALS is set!!!
   There is another module, RR2, thar remember activations...

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests. When a guest operation
     is called, the exception is raised.

 Restrictions & special features:
   - if specified, it creates at init time a task,
     called "Main", attached to the function __init__().
   - There must be only one module in the system that creates a task
     attached to the function __init__().
   - The level tries to guarantee that a task uses a "full" timeslice
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


#ifndef __RR_H__
#define __RR_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

extern TASK __init__(void *arg);



/*+ Const: +*/
#define RR_MINIMUM_SLICE      1000 /*+ Minimum Timeslice +*/
#define RR_MAXIMUM_SLICE    500000 /*+ Maximum Timeslice +*/

#define RR_MAIN_YES              1 /*+ The level creates the main +*/
#define RR_MAIN_NO               0 /*+ The level does'nt create the main +*/

/*+ Registration function:
    TIME slice                the slice for the Round Robin queue
    int createmain            1 if the level creates the main task 0 otherwise
    struct multiboot_info *mb used if createmain specified   

    returns the level number at which the module has been registered.
+*/
LEVEL RR_register_level(TIME slice,
                       int createmain,
                       struct multiboot_info *mb);
__END_DECLS
#endif
