/*
 * Project: HARTIK (HA-rd R-eal TI-me K-ernel)
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *               Gerardo Lamastra <gerardo@sssup.it>
 *
 * Authors     : Massimiliano Giorgi <massy@hartik.sssup.it>
 * (see authors.txt for full list of hartik's authors)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it 
 */

/*
 * Copyright (C) 1999 Massimiliano Giorgi
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

/*
 * CVS :        $Id: mutex.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/**** 
    This file binds the functions used internally for mutex regions with the real functions;
    two kind of mutex are used: normal mutex and fast mutex.
****/

#ifndef __MUTEX_H
#define __MUTEX_H

#include <fs/mutex.h>

extern void *fsmutexattr;

#define __fs_mutex_t __mutex_t

#define __fs_mutex_init(ptr)    __mutex_init(ptr,fsmutexattr);
#define __fs_mutex_lock(ptr)    __mutex_lock(ptr)
#define __fs_mutex_trylock(ptr) __mutex_trylock(ptr)
#define __fs_mutex_unlock(ptr)  __mutex_unlock(ptr)

#define __fs_fastmutex_t __fastmutex_t

#define __fs_fastmutex_init(ptr)   __fastmutex_init(ptr)  
#define __fs_fastmutex_lock(ptr)   __fastmutex_lock(ptr)
#define __fs_fastmutex_unlock(ptr) __fastmutex_unlock(ptr)

#endif
