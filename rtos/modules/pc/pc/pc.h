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
 CVS :        $Id: pc.h,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 This file contains the Priority Ceiling (PC) Protocol

 Title:
   PC (Priority Ceiling protocol)

 Resource Models Accepted:
   PC_RES_MODEL
     This model is used to tell a PC level the priority of a task.

 Description:
   This module implement the Priority Ceiling Protocol.
   The priority inheritance is made using the shadow field of the
   task descriptor. No difference is made upon the task model of the
   tasks that use PC mutexes.

   This module is directly derived from the PI one.

   A PC mutex is created passing the PC_mutexattr structure to mutex_init.

   When a task is created, a priority must be assigned to the task. This
   priority is specified using a PC_RES_MODEL resource model.

 Exceptions raised:
   XMUTEX_OWNER_KILLED
     This exception is raised when a task ends and it owns one or more
     mutexes

 Restrictions & special features:
   - This module is NOT Posix compliant
   - This module can manage any number of PC mutexes.
   - If a task ends (because it reaches the end of the body or because it
     is killed by someone) and it owns some mutex, an exception is raised.
   - if a mutex unlock is called on a mutex not previously
     locked or previously locked by another task an exception is raised
   - A PC mutex can't be statically allocated.

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



#ifndef __PC_H__
#define __PC_H__

#include <kernel/types.h>
#include <kernel/descr.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

RLEVEL PC_register_module(void);

/*+ This function gets the ceiling of a PC mutex, and it have to be called
    only by a task that owns the mutex.
    Returns -1 if the mutex is not a PC mutex, 0 otherwise +*/
int PC_get_mutex_ceiling(const mutex_t *mutex, DWORD *ceiling);

/*+ This function sets the ceiling of a PC mutex, and it have to be called
    only by a task that owns the mutex.
    Returns -1 if the mutex is not a PC mutex, 0 otherwise +*/
int PC_set_mutex_ceiling(mutex_t *mutex, DWORD ceiling, DWORD *old_ceiling);

/*+ This function sets the ceiling of a task +*/
void PC_set_task_ceiling(RLEVEL r, PID p, DWORD priority);

__END_DECLS
#endif
