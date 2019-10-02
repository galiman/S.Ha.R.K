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
 CVS :        $Id: srp.h,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 This file contains the Stack Resource Policy (SRP) Protocol

 Title:
   SRP (Stack Resource Policy)

 Resource Models Accepted:
   None

 Description:
   This module implement the Stack Resource policy using the shadow field
   of the task descriptor. No difference is made upon the task model of the
   tasks that use SRP mutexes.

   A SRP mutex is created passing the SRP_mutexattr structure to mutex_init.

   In effect, this module doesn't work correctly if it is
   used, for example, with a Round Robin Module; in this case we
   can have a task B that preempts A, but when B finishes his timeslice, A
   can be scheduled by the RR module, and the SRP can't block it, because
   the SRP module can not control preemptions (it works only using the shadow
   field at lock and unlock time!!!).
   Note that this problen not affect the EDF, RM & co. algorithms... because
   if B preempts A, A will never preempts on B...

   A task that want to use the SRP protocol MUST declare it using a
   SRP_RES_MODEL in the task_create. Only the first SRP_RES_MODEL is
   considered.

   A task that want to use a SRP mutex have to declare it with the
   SRP_usemutex function as last parameter of a task_create call, AFTER
   the specification of the preemption level.

 Exceptions raised:
   XMUTEX_OWNER_KILLED
     This exception is raised when a task ends and it owns one or more
     mutexes

   XSRP_UNVALID_LOCK
     This exception is raised when a task try to lock a srp mutex but
     it don't have the provilege.

 Restrictions & special features:
   - This module is NOT Posix compliant
   - This module can manage any number of SRP mutexes.
   - If a task ends (because it reaches the end of the body or because it
     is killed by someone) and it owns some mutex, an exception is raised.
   - if a mutex unlock is called on a mutex not previously
     locked or previously locked by another task an exception is raised
   - A SRP mutex can not be statically allocated
   - The module is incompatible with the primitive TASK_JOIN, so the tasks
     that uses SRP can NOT call task_join.
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



#ifndef __SRP_H__
#define __SRP_H__

#include <kernel/model.h>
#include <kernel/descr.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

RLEVEL SRP_register_module(void);

extern __inline__ RES_MODEL *SRP_usemutex(mutex_t *m) {
  return (RES_MODEL *)m->opt;
};

__END_DECLS
#endif
