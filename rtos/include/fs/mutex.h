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

/***************************************

Mutexs for the kernel

***************************************/

/*
 * CVS :        $Id: mutex.h,v 1.3 2006/03/09 16:29:17 tullio Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.3 $
 * Last update: $Date: 2006/03/09 16:29:17 $
 */

/*
 * Copyright (C) 1999,2000 Massimiliano Giorgi
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

#ifndef _FS_MUTEX_H
#define _FS_MUTEX_H

/*
 * standard mutex     
 */
     
#include <kernel/int_sem.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+ a semaphore object +*/
typedef internal_sem_t __mutex_t;

/*
  The following macros can be used to synchronize events; all
  require a pointer to a semaphore object and return nothing;
  a semaphore can have "val" resource free.
 */

/*+ Initialize a semaphore (to integer val) +*/
#define __mutex_init(ptr,attr) internal_sem_init((ptr),1)

/*+ Wait for a semaphore +*/
#define __mutex_lock(ptr)     (internal_sem_wait(ptr))

/*+ Try to wait for a semaphore (return 0 on success) +*/
#define __mutex_trylock(ptr)  (internal_sem_wait(ptr))

/*+ Signal a semaphore +*/
#define __mutex_unlock(ptr)   (internal_sem_post(ptr))

/*
 * fast mutex     
 */

#include <kernel/func.h>

typedef SYS_FLAGS __fastmutex_t;

#define __fastmutex_init(ptr)
#define __fastmutex_lock(ptr)   *ptr=kern_fsave()
#define __fastmutex_unlock(ptr) kern_frestore(*ptr)
 
__END_DECLS
#endif

