/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 ------------
 CVS :        $Id: iqueue.h,v 1.2 2003/03/13 13:36:28 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:36:28 $
 ------------

*/

/*
 * Copyright (C) 2002 Paolo Gai
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
 IQUEUEs

 This file contains functions that helps to manage task queues.

 These functions are different from the functions that manages the
 QUEUE and QQUEUE types. In particular, these functions no more relies
 on the prev & next fields of the task descriptor. In that way, tasks
 can be inserted in more than one queue at a time.

 Basically, an IQUEUE has an "I"nternal prev/next structure, that may
 be shared between one or more queue. Of course, the user MUST
 guarantee that the same task will not be inserted in two IQUEUEs that
 share the same prev/next buffer.

 The queue insertion is made by the following functions:
 iq_insert          -> insertion based on the priority field.
 iq_timespec_insert -> same as above but use the timespec_priority field
 iq_insertfirst     -> insert in the first position of the queue
*/

#include <ll/ll.h>
#include <kernel/const.h>
#include <kernel/types.h>

#ifndef __KERNEL_IQUEUE_H__
#define __KERNEL_IQUEUE_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define IQUEUE_NO_PRIORITY 1
#define IQUEUE_NO_TIMESPEC 2

struct IQUEUE_shared {
  PID prev[MAX_PROC];
  PID next[MAX_PROC];
  struct timespec *timespec_priority;
  DWORD *priority;
};

typedef struct {
  PID first;
  PID last;
  struct IQUEUE_shared *s;
} IQUEUE;



/* Internal queue initialization:

   share = &x -> the internal data structure of the IQUEUE x is used
                 to enqueue the tasks.  

   share = NULL -> an internal data structure to handle prev/next
		   pairs is dynamically allocated (The amount of
		   memory that is allocated can be reduced using the
		   flags).

   flags can be used to reduce the memory usage of an IQUEUE when share=NULL:
   IQUEUE_NO_PRIORITY -> the iqueue do not provide internally a priority field
   IQUEUE_NO_TIMESPEC -> the iqueue do not provide internally a timespec field

   - note that, if these flags are used, the corresponding insert
     functions will not work!
   - the default value for the flags is, of course, 0 
*/
void iq_init (IQUEUE *q, IQUEUE *share, int flags);

/* Queue insert functions: 

   - inserts a p into the q. p must not be already inserted into q.
   - four versions of the function; 
     - iq_priority_insert -> ordered insertion using the priority field
     - iq_timespec_insert -> ordered insertion using the timespec field
     - iq_insertfirst     -> insert at the first position of the queue
     - iq_insertlast      -> insert at the last position of the queue
*/
void iq_priority_insert (PID p, IQUEUE *q);
void iq_timespec_insert (PID p, IQUEUE *q);
void iq_insertfirst     (PID p, IQUEUE *q);
void iq_insertlast      (PID p, IQUEUE *q);

/* Queue extract functions: 

   - extracts a task p from the queue q.
   - three versions of the function; 
     - iq_extract -> extracts given a task p 
                     (that must be inserted in the queue)

     - iq_getfirst -> extracts the first task in the queue, 
                      NIL if the queue is empty
     
     - iq_getlast -> extracts the last task in the queue,
                     NIL if the queue is empty

*/
void iq_extract         (PID p, IQUEUE *q);
PID  iq_getfirst        (       IQUEUE *q);
PID  iq_getlast         (       IQUEUE *q);


/* Queue query functions:

   The first two functions return the first and the last task in the queue,
   NIL if the queue is empty.

   The second two functions can be used to get/set the priority or the
   timespec field used when queuing.
*/
static __inline__ PID iq_query_first(IQUEUE *q)
{
  return q->first;
}

static __inline__ PID iq_query_last(IQUEUE *q)
{
  return q->last;
}

static __inline__ struct timespec *iq_query_timespec(PID p, IQUEUE *q)
{
  return &q->s->timespec_priority[p]; 
}

static __inline__ DWORD *iq_query_priority (PID p, IQUEUE *q)
{
  return &q->s->priority[p];
}

/* Queue iterators */

/* sometimes it is useful to go through the list. For that reason
   You can use the following two functions... */
static __inline__ PID iq_query_next (PID p, IQUEUE *q)
{
  return q->s->next[p];
}

static __inline__ PID iq_query_prev (PID p, IQUEUE *q)
{
  return q->s->prev[p];
}

/* Queue test functions */
static __inline__ int iq_isempty (IQUEUE *q)
{
  return q->first == NIL;
}

__END_DECLS
#endif
