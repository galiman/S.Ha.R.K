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


/***************************************

  CVS :        $Id: idereq.c,v 1.1.1.1 2002/03/29 14:12:49 pj Exp $
 
  Revision:    $Revision: 1.1.1.1 $

  Last update: $Date: 2002/03/29 14:12:49 $

  This module manage the ide request structure: it is implemented
  a policy for the queueing of the requests made by tasks.

***************************************/

/*
 * Copyright (C) 1999 Massimiliano Giorgi
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

#include <fs/assert.h>

#include "glue.h"

#include "ide.h"
#include "idereq.h"

/*
 * if defined:
 * use trc_logevent with event TRC_USER0 to trace head movement
 */   
#define TRACEMOVEMENT
#undef  TRACEMOVEMENT

#ifdef TRACEMOVEMENT
#include <trace/types.h>
#include <kernel/trace.h>
#endif

/*+ a mutex for the  +*/
//static __b_fastmutex_t mutex;

/*+ a mutex for the "free request" queue manipulation +*/
static __b_fastmutex_t mutexreq;

/*+ the "free request" queue +*/
static int freereq=NIL;

/*+ a pool for the ide request (actually all the ide request structure
  are statically allocated)
  +*/
idereq_t idereq[MAXIDEREQUEST];

/*+ is the queue not blocked? +*/
int notblockedqueue[MAXIDEINTERFACES];

/*++++++++++++++++++++++++++++++++++++++
 
  Initialize this module; must be called prior of other call.
  
  ++++++++++++++++++++++++++++++++++++++*/

void init_idereq(void)
{
  int i;
  //__b_fastmutex_init(&mutex);
  __b_fastmutex_init(&mutexreq);
  freereq=0;
  for (i=0;i<MAXIDEREQUEST;i++) {
    idereq[i].next=((i==MAXIDEREQUEST-1)?NIL:i+1);
    __b_sem_init(&idereq[i].wait,0);
  }
  for (i=0;i<MAXIDEINTERFACES;i++)
    notblockedqueue[i]=1;
}

/*++++++++++++++++++++++++++++++++++++++

  Find and return an ide request structure from the free queue if possible.

  int get_idereq
    return an index into idereq[] or NIL
  ++++++++++++++++++++++++++++++++++++++*/

int get_idereq(void)
{
  int ret;
  __b_fastmutex_lock(&mutexreq);
  if (freereq==NIL) 
    ret=NIL;
  else {
    ret=freereq;
    freereq=idereq[ret].next;
  }
  __b_fastmutex_unlock(&mutexreq);
  return ret;
}

/*++++++++++++++++++++++++++++++++++++++

  Insert an ide request structure into the free queue.

  int req
    index into idereq[] to insert into the free queue
  ++++++++++++++++++++++++++++++++++++++*/

void free_idereq(int req)
{
  __b_fastmutex_lock(&mutexreq);
  idereq[req].next=freereq;
  freereq=req;
  __b_fastmutex_unlock(&mutexreq);
}

/*
 * The following functions can be modificated to implement other policy
 * of queueing of the request made by task.
 */

static __inline__ int other_request(int ideif)
{
  return bqueue_numelements(&ide[ideif].queue[0])+
    bqueue_numelements(&ide[ideif].queue[1])?1:0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Insert an ide request structure into an ide interface struct for
  processing a request.
  It is inserted into the tail.

  int insert_idereq
    return 1 if this is the first request of the queue, 0 if there are
    already other requests into the queue; if 1 is returned the calling
    thread should wake up the server

  int ideif
    interface queue to use (an index into ide[] structure)

  int req
    index into idereq[] of the request struct to insert
  ++++++++++++++++++++++++++++++++++++++*/

int insert_idereq(int ideif, int drive, int req)
{
  int ret;
  bqueue_insertrequest(&ide[ideif].queue[drive],
		       (request_prologue_t *)&idereq[req]
		       );
  
  ret=bqueue_numelements(&ide[ideif].queue[drive]);
  if (ret==1)
    /* means: wake up the server if the queue is not blocked */    
    ret=notblockedqueue[ideif];
  else
    /* means: not wake up the server (it is already running) */
    ret=0;

  return ret;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Release a blocked queue.

  int releasequeue_idereq
    return 1 if there are request in queue, 0 if not

  int ideif
    interface queue to use (an index into ide[] structure)
  ++++++++++++++++++++++++++++++++++++++*/

int releasequeue_idereq(int ideif)
{
  notblockedqueue[ideif]=1;
  return other_request(ideif);
}

int first_idereq(int ideif)
{
  request_prologue_t *ptr;
  
  assertk(ide[ideif].actreq==-1);
  
  //__b_fastmutex_lock(&mutex);
  ptr=bqueue_getrequest(&ide[ideif].queue[0]);
  
  //cprintf("°%p,%p,%li°",ptr,idereq,((idereq_t*)ptr)-idereq);
      
  if (ptr==NULL) {
    ptr=bqueue_getrequest(&ide[ideif].queue[1]);
    if (ptr==NULL) {
      //__b_fastmutex_unlock(&mutex);
      return NIL;
    } else
      ide[ideif].actdrv=1;      
  } else
    ide[ideif].actdrv=0;
  ide[ideif].actreq=((idereq_t*)ptr)-idereq;
  //__b_fastmutex_unlock(&mutex);

#ifdef TRACEMOVEMENT
  {
    long cylinder;
    cylinder=ptr->cylinder;
    trc_logevent(TRC_USER0,&cylinder);
  }
#endif
  
  return ide[ideif].actreq;
}

int actual_idereq(ideif)
{
  assertk(ide[ideif].actreq!=-1);
  return ide[ideif].actreq;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Remove the first request from the queue of an ide interface.

  int remove_idereq
    return 1 if there are other request in queue, 0 if not

  int ideif
    interface queue to use (an index into ide[] structure)

  int req
    index into idereq[] of the request struct to insert
  ++++++++++++++++++++++++++++++++++++++*/

int remove_idereq(int ideif)
{
  assertk(ide[ideif].actreq!=-1);
  bqueue_removerequest(&ide[ideif].queue[ide[ideif].actdrv]);
  ide[ideif].actreq=-1;
  return other_request(ideif)?1:0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Remove the first request from the queue of an ide interface
  blocking the queue.

  int remove_idereq
    return 1 if there are other request in queue, 0 if not

  int ideif
    interface queue to use (an index into ide[] structure)

  int req
    index into idereq[] of the request struct to insert
  ++++++++++++++++++++++++++++++++++++++*/

int remove_idereq_blocking(int ideif)
{
  assertk(ide[ideif].actreq!=-1);
  bqueue_removerequest(&ide[ideif].queue[ide[ideif].actdrv]); 
  notblockedqueue[ideif]=0;
  ide[ideif].actreq=-1;
  return other_request(ideif);
}
