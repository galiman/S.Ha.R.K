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
 CVS :        $Id: pistar.h,v 1.4 2004/10/25 14:38:30 trimarchi Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2004/10/25 14:38:30 $
 ------------

 This file contains the Priority Inheritance (PI) Protocol

 Title:
   PI (Priority Inheritance protocol)

 Resource Models Accepted:
   None

 Description:
   This module implement the Priority Inheritance Protocol.
   The priority inheritance is made using the shadow field of the
   task descriptor. No difference is made upon the task model of the
   tasks that use PISTAR mutexes.

   A PISTAR mutex is created passing the PISTAR_mutexattr structure to mutex_init.

 Exceptions raised:
   XMUTEX_OWNER_KILLED
     This exception is raised when a task ends and it owns one or more
     mutexes

 Restrictions & special features:
   - This module is NOT Posix compliant
   - This module can manage any number of PISTAR mutexes.
   - If a task ends (because it reaches the end of the body or because it
     is killed by someone) and it owns some mutex, an exception is raised.
   - if a mutex unlock is called on a mutex not previously
     locked or previously locked by another task an exception is raised
   - A PISTAR mutex can be statically allocated. To do this, the init function
     have to define a macro that puts this information in the mutex
     descriptor: mutexlevel = <PISTAR resource level>; opt = NULL;
     for example, if the PISTAR module is registered at level 1, the macro is
     like:
     #define MUTEX_INITIALIZER {1,NULL}

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



#ifndef __PISTAR_H__
#define __PISTAR_H__

#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

RLEVEL PISTAR_register_module(void);

//int PISTAR_lock(RLEVEL l, mutex_t *m, TIME wcet);


__END_DECLS
#endif
