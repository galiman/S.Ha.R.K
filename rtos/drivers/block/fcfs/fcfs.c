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

/*
 * Copyright (C) 2000 Massimiliano Giorgi
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

#include "bqueue.h"
#include "glue.h"

int fcfs_init(fcfs_queue_t *q)
{
  __b_fastmutex_init(&q->mutex); 
  q->disk=NULL;
  q->head=NULL;
  q->tail=NULL;
  q->counter=0;
  return 0;
}

int fcfs_numelements(fcfs_queue_t *q)
{
  return q->counter;
}

int fcfs_insertrequest(fcfs_queue_t *q, request_prologue_t *e)
{

  //cprintf("Û%p into %pÛ",e,q);

  __b_fastmutex_lock(&q->mutex);  
  if (q->head==NULL) q->head=e;
  else               q->tail->x.next=e;
  q->tail=e;
  e->x.next=NULL;
  q->counter++;
  __b_fastmutex_unlock(&q->mutex);
  
  return 0;
}

request_prologue_t *fcfs_getrequest(fcfs_queue_t *q)
{
  request_prologue_t *ret;

  //cprintf("Ûfrom %p have %pÛ",q,q->head);

  __b_fastmutex_lock(&q->mutex);
  ret=q->head;
  __b_fastmutex_unlock(&q->mutex);
  
  return ret;
}

int fcfs_removerequest(fcfs_queue_t *q)
{
  //cprintf("Ûremoved %p from %pÛ",q->head,q);
  
  __b_fastmutex_lock(&q->mutex);
  assertk(q->head!=NULL);
  q->head=q->head->x.next;
  q->counter--;
  __b_fastmutex_unlock(&q->mutex);
  
  return 0;
}
