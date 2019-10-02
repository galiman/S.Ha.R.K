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
 CVS :        $Id: nop.h,v 1.1 2005/02/25 10:53:02 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:02 $
 ------------

 This file contains the No Protocol (NOP) implementation of mutexes

 Title:
   NOP (Binary Semaphores)

 Resource Models Accepted:
   None

 Description:
   This module implement a mutex interface using extraction and insertion
   into scheduling queues.

 Exceptions raised:
   none

 Restrictions & special features:
   - This module is NOT Posix compliant
   - This module can manage any number of NOP mutexes.
   - A NOP mutex can be statically allocated. To do this, the init function
     have to define a macro that puts this information in the mutex
     descriptor: mutexlevel = <NOP resource level>; opt = NULL;
     for example, if the NOP module is registered at level 1, the macro is
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



#ifndef __NOP_H__
#define __NOP_H__

#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

RLEVEL NOP_register_module(void);

__END_DECLS
#endif
