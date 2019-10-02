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
 CVS :        $Id: int_sem.h,v 1.3 2003/03/13 13:36:28 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2003/03/13 13:36:28 $
 ------------

 Internal semaphores.

 They are different from the Posix semaphores and the mutexes because:
 - internal_sem_wait is not a cancellation point
 - there are no limits on the semaphores that can be created
   (they works like a mutex_t...)
 - the queuing policy is FIFO
 - Be Careful!
   they are made to be fast... so not so many controls are done!!!

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

#ifndef __INT_SEM_H__
#define __INT_SEM_H__

#include <kernel/types.h>
#include <kernel/iqueue.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* this is the structure normally pointed by the opt field in the
   mutex_t structure */
typedef struct {
  int count;
  IQUEUE blocked;
} internal_sem_t;


/* this function initializes an internal semaphore */
void internal_sem_init(internal_sem_t *s, int value);

/* blocking wait (decrement by 1) */
void internal_sem_wait(internal_sem_t *s);

/* return 0 if the counter is decremented, -1 if not */
int internal_sem_trywait(internal_sem_t *s);

/* post (increment by 1) */
void internal_sem_post(internal_sem_t *s);

/* getvalue (>= 0 if there is noone blocked on it, -1 otherwise) */
int internal_sem_getvalue(internal_sem_t *s);

__END_DECLS
#endif
