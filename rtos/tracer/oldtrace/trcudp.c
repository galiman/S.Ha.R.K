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
 * CVS :        $Id: trcudp.c,v 1.1 2003/12/10 16:48:48 giacomo Exp $
 */

#include <ll/sys/types.h>
#include <ll/stdlib.h>

#include <kernel/func.h>
#include <kernel/mem.h>
#include <kernel/log.h>

#include <types.h>
#include <trace.h>
#include <queues.h>

//#define DEBUG_TRCUDP

#define TRCUDP_MAXEVENTS (1500/sizeof(trc_event_t))
//#define TRCUDP_MAXEVENTS 10

/* Well... this file is very similar to trccirc.c! */

typedef struct TAGtrcudp_queue_t {
  /*+ size of the queue +*/
  int         size;
  /*+ index of the next insertion into the queue +*/
  int         index;
  /*+ index of the next item to write (if online_tracer activated) +*/
  int         windex;
  /*+ number of events lost (if online_tracer activated) +*/
  long        hoops;
  /*+ local and remote IP numbers +*/
  UDP_ADDR local, remote;
  /*+ unique number that identify the queue +*/
  int         uniq;
  /*+ =1 when the system shuts down +*/
  int mustgodown;
  TASK_MODEL *m;
  /*+ dummy, needed for creating a valid packet (dirty trick ;-) +*/
  short int   dummy;
  /*+ events table +*/
  trc_event_t table[0];  
} trcudp_queue_t;

static TASK online_tracer(trcudp_queue_t *queue)
{
  int s; /* the socket */
  int newwindex; /* new write index after sending the packet */
  int n; /* number of packets to send */
  short int *pkt;


  s = udp_bind(&queue->local, NULL);
  
  for (;;) {
    if (queue->index<queue->windex) {
      if (queue->windex+TRCUDP_MAXEVENTS < queue->size) {
	newwindex = queue->windex+TRCUDP_MAXEVENTS;
	n = TRCUDP_MAXEVENTS;
      } else {
	newwindex = 0;
	n = queue->size-queue->windex;
      }
    } else {
      if (queue->windex+TRCUDP_MAXEVENTS < queue->index) {
	newwindex = queue->windex+TRCUDP_MAXEVENTS;
	n = TRCUDP_MAXEVENTS;
      } else {
	newwindex = queue->index;
	n = queue->index-queue->windex;
      }
    }
    
    if (n) {
      /* set the number of events into the UDP packet.  It works
	 because the event entry before windex is always empty, or
	 because we use the dummy field into the struct */
      pkt = ((short int *)(queue->table+queue->windex))-1;
      *pkt = (short int)n;
      udp_sendto(s,(char *)pkt,
		 n*sizeof(trc_event_t)+2,&queue->remote);
#ifdef DEBUG_TRCUDP
      printk(KERN_DEBUG "UDP: SEND %d events,"
	     " index %d windex %d new %d!!!\n",n,
	     queue->index, queue->windex, newwindex);
#endif
      queue->windex = newwindex;
    }
    
    if (queue->mustgodown) {
      if (queue->windex == queue->index)
	break;
    }
    else
      task_endcycle();
  }

  return NULL;
}


static trc_event_t *trcudp_get(trcudp_queue_t *queue)
{
  if (queue->mustgodown)
    return NULL;

  if (queue->index==queue->size-1) {
    if (queue->windex==0) {
      queue->hoops++;
      return NULL;
    }
    queue->index=0;
    return &queue->table[queue->size-1];
  }  
  if (queue->index+1==queue->windex) {
    queue->hoops++;
    return NULL;
  }
  return &queue->table[queue->index++];
}

static int trcudp_post(trcudp_queue_t *queue)
{
  return 0;
}

static void trcudp_shutdown(trcudp_queue_t *queue);

static int trcudp_create(trc_queue_t *p, TRC_UDP_PARMS *args)
{
  trcudp_queue_t *queue;

  if (args==NULL) {
    printk(KERN_ERR "trcudp_create: you must specify a non-NULL parameter!");
    return -1;
  }
  
  queue=(trcudp_queue_t*)kern_alloc(sizeof(trcudp_queue_t)+
				    sizeof(trc_event_t)*args->size);
  if (queue==NULL) {
    printk(KERN_ERR "trcudp_create: error during memory allocation!");
    return -1;
  }

  p->get=(trc_event_t*(*)(void*))trcudp_get;
  p->post=(int(*)(void*))trcudp_post;
  p->data=queue;
  
  queue->size=args->size;
  queue->windex=queue->index=0;
  queue->hoops=0;
  queue->local=args->local;
  queue->remote=args->remote;
  /* uniq initialized in trcudp_activate */
  queue->mustgodown=0;
  queue->m = args->model;
  /* dummy unused */
  
  /* AFTER exit because in that way we can hope to be back in text mode... */
  sys_atrunlevel((void (*)(void *))trcudp_shutdown, (void *)queue, RUNLEVEL_AFTER_EXIT);
    
  return 0;
}

static int trcudp_activate(trcudp_queue_t *queue, int uniq)
{
  SOFT_TASK_MODEL model;
  TASK_MODEL *m;
  PID pid;


  queue->uniq=uniq;

  if (!queue->m) {
    soft_task_default_model(model);
    soft_task_def_system(model);
    /* soft_task_def_notrace(model); Should we trace the tracer? */
    soft_task_def_periodic(model);
    soft_task_def_period(model,250000);
    soft_task_def_met(model,10000);
    soft_task_def_wcet(model,10000);
    /* soft_task_def_nokill(model); NOOOOOOO!!!! */
    soft_task_def_arg(model,queue);
    m = (TASK_MODEL *)&model;
  }
  else {
    m = queue->m;
    task_def_arg(*m,queue);
  }

  pid=task_create("trcUDP",online_tracer,m,NULL);
  if (pid==-1) {
    printk(KERN_ERR "can't start tracer online trcudp trace task");
  } else 
    task_activate(pid);

  return 0;
}

static int trcudp_terminate(trcudp_queue_t *queue)
{
  queue->mustgodown = 1;

  return 0;
}

static void trcudp_shutdown(trcudp_queue_t *queue)
{
  printk(KERN_NOTICE "tracer: %li events lost into UDP queue %d",
	 queue->hoops, queue->uniq);
}

int trc_register_udp_queue(void)
{
  int res;
  
  res=trc_register_queuetype(TRC_UDP_QUEUE,
			     (int(*)(trc_queue_t*,void*))trcudp_create,
			     (int(*)(void*,int))trcudp_activate,
			     (int(*)(void*))trcudp_terminate
			     );
  
  if (res!=0) printk(KERN_WARNING "can't register tracer trcudp queue");
  return res;
}
