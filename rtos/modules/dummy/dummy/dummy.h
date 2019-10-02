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
 CVS :        $Id: dummy.h,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains the scheduling module RR (Round Robin)

 Title:
   DUMMY

 Task Models Accepted:
   DUMMY_TASK_MODEL - Dummy process (not usable)

 Description:
   This module creates the dummy task, witch is a special task that
   do nothing.

 Exceptions raised:
   XUNVALID_GUEST
     This level doesn't support guests. When a guest operation
     is called, the exception is raised.
   XUNVALID_DUMMY_OP
     The dummy task can't be created, or activated, and so on...

 Restrictions & special features:
   - the task model DUMMY_TASK_MODEL can be used only at init time
     to register the dummy process into the system.
   - if __HLT_WORKS__ defined in this header file, the dummy task can
     perform a hlt istruction to save power...

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


#ifndef __DUMMY_H__
#define __DUMMY_H__

#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+
 On upper Intel CPUs it is possible to avoid CPU power consumption
 when the system is idle issuing the hlt instruction.
 This is often available on many 32 bit CPUs...
 If it is, simply define the following!!!
+*/
#define  __HLT_WORKS__

/*+ Registration function 

    returns the level number at which the module has been registered.
+*/
LEVEL dummy_register_level();

__END_DECLS
#endif
