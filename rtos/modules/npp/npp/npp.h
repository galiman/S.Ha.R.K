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
 CVS :        $Id: npp.h,v 1.1 2005/02/25 10:45:36 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:45:36 $
 ------------

 This file contains the Non Preemptive Protocol (NPP)

 Title:
   NPP (Non Preemptive Protocol)

 Resource Models Accepted:
   None

 Description:
   This module implement the Non Preemptive Protocol.
   When a task aquire a critical section, it become non-preemptable.

   A NPP mutex is created passing the NPP_mutexattr structure to mutex_init.

 Exceptions raised:
   XMUTEX_OWNER_KILLED
     This exception is raised when a task ends and it owns one or more
     mutexes

 Restrictions & special features:
   - This module is NOT Posix compliant
   - This module can manage any number of NPP mutexes.
   - if a task uses a NPP mutex, it can use only this type of mutex.
   - If a task ends (because it reaches the end of the body or because it
     is killed by someone) and it owns some mutex, an exception is raised.
   - if a mutex unlock is called on a mutex not previously
     locked or previously locked by another task an error is returned
   - A NPP mutex can be statically allocated. To do this, the init function
     have to define a macro that puts this information in the mutex
     descriptor: mutexlevel = <NPP resource level>; opt = NULL;
     for example, if the NPP module is registered at level 1, the macro is
     like:
     #define MUTEX_INITIALIZER {1,(void *)NULL}

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



#ifndef __NPP_H__
#define __NPP_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

void NPP_register_module(void);

__END_DECLS
#endif
