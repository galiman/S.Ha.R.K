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
 CVS :        $Id: types.h,v 1.5 2004/01/28 11:54:36 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2004/01/28 11:54:36 $
 ------------

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

#ifndef __KERNEL_TYPES_H__
#define __KERNEL_TYPES_H__
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define TASK void *

/*+ ... a task index +*/
typedef int PID;

/*+ ... a scheduling level index +*/
typedef int LEVEL;

/*+ ... a resource level index +*/
typedef int RLEVEL;

/*+ the type used in the on-line guarantee +*/
typedef unsigned int bandwidth_t;
typedef signed long long ext_bandwidth_t;

/*+ keys for task specific data */
typedef int task_key_t;

__END_DECLS
#endif
