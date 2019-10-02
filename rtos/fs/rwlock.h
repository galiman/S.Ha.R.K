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
 * CVS :        $Id: rwlock.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/**** 
    This file contains the prototypes of an implementation of a  reader/writer's protocol.
****/

#ifndef __RWLOCK_H
#define __RWLOCK_H

#include <fs/mutex.h>
#include <fs/semaph.h>

typedef struct {
  __mutex_t  mutex;
  int        active_readers;
  int        active_writers;
  int        blocked_readers;
  int        blocked_writers;
  __sem_t    readers_semaph;
  __sem_t    writers_semaph;
} __rwlock_t;

void __rwlock_init(__rwlock_t *ptr);
void __rwlock_rdlock(__rwlock_t *ptr);
void __rwlock_wrlock(__rwlock_t *ptr);
int  __rwlock_tryrdlock(__rwlock_t *ptr);
int  __rwlock_trywrlock(__rwlock_t *ptr);
void __rwlock_rdunlock(__rwlock_t *ptr);
void __rwlock_wrunlock(__rwlock_t *ptr);

#endif
