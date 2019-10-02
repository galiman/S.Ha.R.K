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

/**/

#include <modules/bd_pscan.h>

static __inline__ int get_priority(void)
{
  int x;
  x=bd_pscan_getpriority();
  if (x<0||x>=NUMPRIORITY) return NUMPRIORITY-1;
  return x;
}

/**/

int pscan_init(pscan_queue_t *q)
{
  int i;
  __b_fastmutex_init(&q->mutex);
  q->disk=NULL;
  q->counter=0;
  q->actprior=0;
  for (i=0;i<NUMPRIORITY;i++) {
    q->pri[i].queue[0]=NULL;
    q->pri[i].queue[1]=NULL;
    q->pri[i].dir=0;
  }
  return 0;
}

int pscan_numelements(pscan_queue_t *q)
{
  return q->counter;
}

/* 0 -> no starvation (low performance) */
/* 1 -> possible starvation (high performance) */
#define STARVATION 1

int pscan_insertrequest(pscan_queue_t *q, request_prologue_t *e)
{
  request_prologue_t *p,*o;
  struct pscan_queues *ptr;
  int i;
  
  __b_fastmutex_lock(&q->mutex);

  i=get_priority();
  ptr=&q->pri[i];
    
  if (ptr->queue[ptr->dir]==NULL) ptr->dir^=1;

  if (ptr->queue[ptr->dir]==NULL) {
    ptr->queue[ptr->dir]=e;
    e->x.next=NULL;
  } else {

    if (e->cylinder==ptr->queue[ptr->dir]->cylinder) {
      if (ptr->dir==STARVATION) goto HI_INS;
      else                      goto LO_INS;
    }
    
    if (e->cylinder<ptr->queue[ptr->dir]->cylinder) {
    LO_INS:
      /* insert into low queue */
      o=NULL;
      p=ptr->queue[0];
      while (p!=NULL) {
	if (e->cylinder>p->cylinder) break;
	o=p;
	p=p->x.next;
      }
      if (o==NULL) ptr->queue[0]=e;
      else         o->x.next=e;
      e->x.next=p;
    } else {
    HI_INS:
      /* insert into high queue */
      o=NULL;
      p=ptr->queue[1];
      while (p!=NULL) {
	if (e->cylinder<p->cylinder) break;
	o=p;
	p=p->x.next;
      }
      if (o==NULL) ptr->queue[1]=e;
      else         o->x.next=e;
      e->x.next=p;
    }
    
  }
  
  q->counter++;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}

request_prologue_t *pscan_getrequest(pscan_queue_t *q)
{
  request_prologue_t *ret;
  int i;
  __b_fastmutex_lock(&q->mutex);
  for (i=0;i<NUMPRIORITY;i++) {
    q->actprior=i;
    ret=q->pri[i].queue[q->pri[i].dir];
    if (ret!=NULL) break;
    else {
      q->pri[i].dir^=1;
      ret=q->pri[i].queue[q->pri[i].dir];
      if (ret!=NULL) break;
    }
  }
  //cprintf("{%i}",i);  
  __b_fastmutex_unlock(&q->mutex);
  return ret;
}

int pscan_removerequest(pscan_queue_t *q)
{
  struct pscan_queues *ptr;
  __b_fastmutex_lock(&q->mutex);
  ptr=&q->pri[q->actprior];
  assertk(ptr->queue[ptr->dir]!=NULL);
  ptr->queue[ptr->dir]=ptr->queue[ptr->dir]->x.next;
  q->counter--;
  __b_fastmutex_unlock(&q->mutex);  
  return 0;
}
