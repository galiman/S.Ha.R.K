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

/*
 *
 */

#include <modules/bd_edf.h>

static __inline__ void get_dl(request_prologue_t *e)
{
  e->x.dl=bd_edf_getdl();
}

/*
 *
 */

int bd_edf_init(bd_edf_queue_t *q)
{
  __b_fastmutex_init(&q->mutex); 
  q->disk=NULL;
  q->queue=NULL;
  q->counter=0;
  q->inservice=0;
  return 0;
}

int bd_edf_numelements(bd_edf_queue_t *q)
{
  return q->counter;
}

int bd_edf_insertrequest(bd_edf_queue_t *q, request_prologue_t *e)
{
  request_prologue_t *p,*o;

  __b_fastmutex_lock(&q->mutex);

  get_dl(e);

  if (q->queue==NULL) {
    q->queue=e;
    e->x.next=NULL;
  } else {
    o=NULL;
    p=q->queue;
    if (q->inservice) {
      /* no preemption possible! */
      o=p;
      p=p->x.next;
    }
    while (p!=NULL) {
      if (e->x.dl<p->x.dl) break;
      o=p;
      p=p->x.next;
    }
    o->x.next=e;
    e->x.next=p;    
  }

  q->counter++;
  __b_fastmutex_unlock(&q->mutex);
  
  return 0;
}

request_prologue_t *bd_edf_getrequest(bd_edf_queue_t *q)
{
  request_prologue_t *ret;

  __b_fastmutex_lock(&q->mutex);
  ret=q->queue;
  if (ret!=NULL) q->inservice=1;
  __b_fastmutex_unlock(&q->mutex);
  
  return ret;
}

int bd_edf_removerequest(bd_edf_queue_t *q)
{
  __b_fastmutex_lock(&q->mutex);
  assertk(q->queue!=NULL);
  q->queue=q->queue->x.next;
  q->inservice=0;
  q->counter--;
  __b_fastmutex_unlock(&q->mutex);
  
  return 0;
}



