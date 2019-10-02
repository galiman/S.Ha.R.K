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
 CVS :        $Id: iqueue.c,v 1.1 2002/11/11 08:34:08 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2002/11/11 08:34:08 $
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

#include <kernel/iqueue.h>
#include <kernel/mem.h>

void iq_init (IQUEUE *q, IQUEUE *share, int flags)
{
  q->first = NIL;
  q->last = NIL;

  if (share)
    q->s = share->s;
  else {
    q->s = (struct IQUEUE_shared *)kern_alloc(sizeof(struct IQUEUE_shared));

    if (!(flags & IQUEUE_NO_PRIORITY))
      q->s->priority = (DWORD *)kern_alloc(sizeof(DWORD) * MAX_PROC);
    if (!(flags & IQUEUE_NO_TIMESPEC))
      q->s->timespec_priority = (struct timespec *)
	kern_alloc(sizeof(struct timespec) * MAX_PROC);
  }
}

/*+
  This function insert the task with PID i in the queue que.
  The insertion is made respecting the priority field.
  (the first item in the queue have the less priority)
+*/
void iq_priority_insert (PID i, IQUEUE *que)
{
  DWORD prio;
  PID p,q;
  
  p = NIL;
  q = que->first;
  prio = que->s->priority[i];
  
  while ((q != NIL) && (prio >= que->s->priority[q])) {
    p = q;
    q = que->s->next[q];
  }
  
  if (p != NIL)
    que->s->next[p] = i;
  else
    que->first = i;
  
  if (q != NIL)
    que->s->prev[q] = i;
    else
      que->last = i;
  
  que->s->next[i] = q;
  que->s->prev[i] = p;
}


/*
  This function insert the task with PID i in the queue que.
  The insertion is made respecting the timespec priority field.
  (the first item in the queue have the less priority)
*/
void iq_timespec_insert(PID i, IQUEUE *que)
{
  struct timespec prio;
  PID p,q;

  p = NIL;
  q = que->first;

  TIMESPEC_ASSIGN(&prio, &que->s->timespec_priority[i]);
  
  while ((q != NIL) && 
	 !TIMESPEC_A_LT_B(&prio, &que->s->timespec_priority[q])) {
    p = q;
    q = que->s->next[q];
  }
  
  if (p != NIL)
    que->s->next[p] = i;
  else
    que->first = i;
  
  if (q != NIL)
    que->s->prev[q] = i;
    else
      que->last = i;
  
  que->s->next[i] = q;
  que->s->prev[i] = p;
}



void iq_insertfirst(PID p, IQUEUE *q)
{
  if (q->first != NIL) {
    q->s->next[p] = q->first;
    q->s->prev[q->first] = p;
  }
  else {
    q->last = p;
    q->s->next[p] = NIL;
  }
  q->s->prev[p] = NIL;
  q->first = p;
}


void iq_insertlast(PID p, IQUEUE *q)
{
  if (q->last != NIL) {
    q->s->prev[p] = q->last;
    q->s->next[q->last] = p;
  }
  else {
    q->first = p;
    q->s->prev[p] = NIL;
  }
  q->s->next[p] = NIL;
  q->last = p;
}


void iq_extract(PID i, IQUEUE *que)
{
  PID p,q;
  
  p = que->s->prev[i];
  q = que->s->next[i];
  
  if (p != NIL)
    que->s->next[p] = que->s->next[i];
  else
    que->first = q;
  
  if (q != NIL)
    que->s->prev[q] = que->s->prev[i];
  else
    que->last = p;
}

PID iq_getfirst(IQUEUE *q)
{
  PID p = q->first;
  
  if (p == NIL) 
    return NIL;

  q->first = q->s->next[q->first];

  if (q->first != NIL)
    q->s->prev[q->first] = NIL;
  else
    q->last = NIL;
  
  return p;
}

PID iq_getlast(IQUEUE *q)
{
  PID p = q->last;
  
  if (p == NIL) 
    return NIL;

  q->last = q->s->prev[q->last];

  if (q->last != NIL)
    q->s->next[q->last] = NIL;
  else
    q->first = NIL;
  
  return p;
}
