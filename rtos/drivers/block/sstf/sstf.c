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

int sstf_init(sstf_queue_t *q)
{
  __b_fastmutex_init(&q->mutex); 
  q->disk=NULL;
  q->lqueue=NULL;
  q->hqueue=NULL;
  q->actual=NULL;
  q->counter=0;
  return 0;
}

int sstf_numelements(sstf_queue_t *q)
{
  return q->counter;
}

int sstf_insertrequest(sstf_queue_t *q, request_prologue_t *e)
{
  request_prologue_t *p,*o;
  
  __b_fastmutex_lock(&q->mutex);
  if (q->actual==NULL)
    q->actual=e;
  else {
    if (e->cylinder<=q->actual->cylinder) {
      /* insert into low queue */
      o=NULL;
      p=q->lqueue;
      while (p!=NULL) {
	if (e->cylinder>p->cylinder) break;
	o=p;
	p=p->x.next;
      }
      if (o==NULL) q->lqueue=e;
      else         o->x.next=e;
      e->x.next=p;
    } else {
      /* insert into high queue */
      o=NULL;
      p=q->hqueue;
      while (p!=NULL) {
	if (e->cylinder<p->cylinder) break;
	o=p;
	p=p->x.next;
      }
      if (o==NULL) q->hqueue=e;
      else         o->x.next=e;
      e->x.next=p;
    }
  }
  q->counter++;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}

request_prologue_t *sstf_getrequest(sstf_queue_t *q)
{
  request_prologue_t *ret;
  __b_fastmutex_lock(&q->mutex);
  ret=q->actual;
  __b_fastmutex_unlock(&q->mutex);
  return ret;
}

int sstf_removerequest(sstf_queue_t *q)
{
  __b_fastmutex_lock(&q->mutex);
  assertk(q->actual!=NULL);
  if (q->lqueue==NULL) {
    if (q->hqueue==NULL) {
      
      /* lqueue==NULL && hqueue==NULL */
      
      q->actual=NULL;
      
    } else {
      
      /* lqueue==NULL && hqueue!=NULL */
      
      q->actual=q->hqueue;
      q->hqueue=q->hqueue->x.next;
      
    }
  } else {
    if (q->hqueue==NULL) {
      
      /* lqueue!=NULL && hqueue==NULL */
      
      q->actual=q->lqueue;
      q->lqueue=q->lqueue->x.next;
      
    } else {
      
      /* lqueue!=NULL && hqueue!=NULL */
      
      int dl,dh;
      dh=q->hqueue->cylinder-q->actual->cylinder;
      dl=q->actual->cylinder-q->lqueue->cylinder;
      if (dh<dl) {
	q->actual=q->hqueue;
	q->hqueue=q->hqueue->x.next;
      } else {
	q->actual=q->lqueue;
	q->lqueue=q->lqueue->x.next;    
      }
      
    }
  }
  
  q->counter--;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}
