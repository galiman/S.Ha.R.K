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
 CVS :        $Id: ss.h,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains the aperiodic server SS (Sporadic Server)

 Title:
   SS (Sporadic Server)

 Task Models Accepted:
   SOFT_TASK_MODEL - Soft Tasks
     wcet field is ignored
     met field is ignored
     period field is ignored
     periodicity field can be only APERIODIC
     arrivals field can be either SAVE or SKIP

 Description:
   This module schedule his tasks following the Sporadic Server scheme.
   The scheme is slightly modified respect to classic SS. The server
   becomes active at arrival time of a task must be served.

   All the tasks are put in a FIFO (FCFS) queue and at a time only the first
   task in the queue is put in the upper level.

   The module remembers pending activations when calling task_sleep...

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests. When a guest operation
     is called, the exception is raised.
   XUNVALID_SS_REPLENISH
     Indicates an anomalous replenishment situation:
     . replenish time fires and no amounts are set
     . replenish amount posted when server is not active
     . no more space to post a replenish amount

 Restrictions & special features:
   - This level doesn't manage the main task.
   - At init time we have to specify:
     . The Capacity and the period used by the server
   - The level don't use the priority field.
   - if an aperiodic task calls a task_delay when owning a mutex implemented
     with shadows, the delay may have no effect, so don't use delay when
     using a mutex!!!
   - On calling task_delay and task_sleep the replenish amount is posted.
     This because replenish time may fires when task is sleeping (server
     is not active).
   - A function to return the used bandwidth of the level is provided.
   - A function to return the avail server capacity is provided.

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


#ifndef __SS_H__
#define __SS_H__

#include <kernel/config.h>
#include <kernel/types.h>
#include <sys/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+ 1 - ln(2) +*/
#ifndef RM_MINFREEBANDWIDTH
#define RM_MINFREEBANDWIDTH 1317922825
#endif

/*+ Max size of replenish queue +*/
#define SS_MAX_REPLENISH MAX_EVENT

/*+ flags... +*/
#define SS_DISABLE_ALL           0
#define SS_ENABLE_BACKGROUND     1  /*+ Background scheduling enabled +*/
#define SS_ENABLE_GUARANTEE_EDF  2  /*+ Task Guarantee enabled +*/
#define SS_ENABLE_ALL_EDF        3  /*+ guarantee+background enabled +*/

#define SS_ENABLE_GUARANTEE_RM   4  /*+ Task Guarantee enabled +*/
#define SS_ENABLE_ALL_RM         5  /*+ guarantee+background enabled +*/

/*+ internal flags +*/
#define SS_BACKGROUND            8  /*+ this flag is set when scheduling
                                        in background +*/
#define SS_BACKGROUND_BLOCK     16  /*+ this flag is set when we want to
                                        blocks the background scheduling +*/

/*+ internal switches +*/
typedef enum {
	SS_SERVER_NOTACTIVE,   /*+ SS is not active +*/
	SS_SERVER_ACTIVE       /*+ SS is active +*/
} ss_status;

/*+ internal functions +*/

/*+ Used to store replenishment events +*/
/* Now we use static allocated array. In this way, no more then
   SS_MAX_REPLENISH replenishments can be posted.
typedef struct {
	struct timespec rtime;
	int ramount;
	replenishq *next;
} replenishq;
*/

/*+ Registration function:
    bandwidth_t b Max bandwidth used by the SS
    int flags     Options to be used in this level instance...
    LEVEL master  The level that must be used as master level
    int Cs        Server capacity
    int per       Server period           

    returns the level number at which the module has been registered.
+*/
LEVEL SS_register_level(int flags, LEVEL master, int Cs, int per);

/*+ Returns the used bandwidth of a level +*/
bandwidth_t SS_usedbandwidth(LEVEL l);

/*+ Returns tha available capacity +*/
int SS_availCs(LEVEL l);

__END_DECLS
#endif
