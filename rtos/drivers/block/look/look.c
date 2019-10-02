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

int look_init(look_queue_t *q)
{
  __b_fastmutex_init(&q->mutex); 
  q->disk=NULL;
  q->queue[0]=NULL;
  q->queue[1]=NULL;
  q->dir=0;
  q->counter=0;
  return 0;
}

int look_numelements(look_queue_t *q)
{
  return q->counter;
}

/* 0 -> no starvation */
/* 1 -> possible starvation */
#define STARVATION 1

int look_insertrequest(look_queue_t *q, request_prologue_t *e)
{
  request_prologue_t *p,*o;
  
  __b_fastmutex_lock(&q->mutex);
  
  if (q->queue[q->dir]==NULL) q->dir^=1;

  if (q->queue[q->dir]==NULL) {
    q->queue[q->dir]=e;
    e->x.next=NULL;
  } else {

    if (e->cylinder==q->queue[q->dir]->cylinder) {
      if (q->dir==STARVATION) goto HI_INS;
      else                    goto LO_INS;
    }
    
    if (e->cylinder<q->queue[q->dir]->cylinder) {
    LO_INS:
      /* insert into low queue */
      o=NULL;
      p=q->queue[0];
      while (p!=NULL) {
	if (e->cylinder>p->cylinder) break;
	o=p;
	p=p->x.next;
      }
      if (o==NULL) q->queue[0]=e;
      else         o->x.next=e;
      e->x.next=p;
    } else {
    HI_INS:
      /* insert into high queue */
      o=NULL;
      p=q->queue[1];
      while (p!=NULL) {
	if (e->cylinder<p->cylinder) break;
	o=p;
	p=p->x.next;
      }
      if (o==NULL) q->queue[1]=e;
      else         o->x.next=e;
      e->x.next=p;
    }
    
  }
  
  q->counter++;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}

request_prologue_t *look_getrequest(look_queue_t *q)
{
  request_prologue_t *ret;
  __b_fastmutex_lock(&q->mutex);
  ret=q->queue[q->dir];
  if (ret==NULL) {
    q->dir^=1;
    ret=q->queue[q->dir];
  }
  __b_fastmutex_lock(&q->mutex);
  return ret;
}

int look_removerequest(look_queue_t *q)
{
  __b_fastmutex_lock(&q->mutex);
  assertk(q->queue[q->dir]!=NULL);
  q->queue[q->dir]=q->queue[q->dir]->x.next;
  q->counter--;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}
