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
 CVS :        $Id: sem.h,v 1.1 2005/02/25 10:50:43 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:50:43 $
 ------------

 This file contains the semaphoric primitives

 Title:
   HARTSEM (Hartik Semaphores)

 Resource Models Accepted:
   None

 Description:
   This module contains a semaphore library compatible with Posix, Plus
   an extension to permit post and wait with counters > 1

 Exceptions raised:
   None

 Restrictions & special features:
   - a function isBlocked is provided
   - the named semaphores are NOT implemented with a filesystem
   - the system supports up to _POSIX_SEM_NSEMS_MAX defined in limits.h

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

#ifndef __MODULES_SEM_H__
#define __MODULES_SEM_H__

#include <kernel/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define SEM_FAILED NULL
typedef int sem_t;

/*+ This function must be inserted in the __hartik_register_levels__ +*/
void SEM_register_module(void);

int sem_close(sem_t *sem);
int sem_destroy(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_init(sem_t *sem, int pshared, unsigned int value);
sem_t *sem_open(const char *name, int oflag, ...);
int sem_post(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_unlink(const char *name);
int sem_wait(sem_t *sem);



int sem_xpost(sem_t *sem, int n);
int sem_xwait(sem_t *sem, int n, int wait);


int isBlocked(PID i);

__END_DECLS
#endif
