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

int clook_init(clook_queue_t *q)
{
  __b_fastmutex_init(&q->mutex); 
  q->disk=NULL;
  q->queue[0]=NULL;
  q->queue[1]=NULL;
  q->act=0;
  q->counter=0;
  return 0;
}

int clook_numelements(clook_queue_t *q)
{
  return q->counter;
}

/* 0 -> no starvation */
/* 1 -> possible starvation */
#define STARVATION 0

int clook_insertrequest(clook_queue_t *q, request_prologue_t *e)
{
  request_prologue_t *p,*o;
  
  __b_fastmutex_lock(&q->mutex);

  if (q->queue[q->act]==NULL) {
    q->queue[q->act]=e;
    e->x.next=NULL;
  } else if (e->cylinder>q->queue[q->act]->cylinder-STARVATION) {
    o=q->queue[q->act];
    p=o->x.next;
    while (p!=NULL) {
      if (e->cylinder<p->cylinder) break;
      o=p;
      p=p->x.next;
    }
    o->x.next=e;
    e->x.next=p;    
  } else {
    o=NULL;
    p=q->queue[q->act^1];
    while (p!=NULL) {
      if (e->cylinder<p->cylinder) break;
      o=p;
      p=p->x.next;
    }
    if (o==NULL) q->queue[q->act^1]=e;
    else         o->x.next=e;
    e->x.next=p;
  }
  
  q->counter++;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}

request_prologue_t *clook_getrequest(clook_queue_t *q)
{
  request_prologue_t *ret;
  __b_fastmutex_lock(&q->mutex);
  ret=q->queue[q->act];
  if (ret==NULL) {
    q->act^=1;
    ret=q->queue[q->act];
  }
  __b_fastmutex_unlock(&q->mutex);
  return ret;
}

int clook_removerequest(clook_queue_t *q)
{
  __b_fastmutex_lock(&q->mutex);
  assertk(q->queue[q->act]!=NULL);
  q->queue[q->act]=q->queue[q->act]->x.next;
  q->counter--;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}
