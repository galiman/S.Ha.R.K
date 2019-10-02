/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
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

#include <ll/sys/types.h>

#include <kernel/types.h>
#include <kernel/var.h>

#include <ll/stdlib.h>
#include <ll/string.h>

#include <kernel/func.h>

#include <types.h>
#include <trace.h>
#include <queues.h>

#include <bits/limits.h>

#include <tracer.h>

static void dummy_logevent(int event, void *ptr)
{
  return;
}

void (*trc_logevent)
     (int event, void *ptr)=dummy_logevent;
     
static int dummy_suspendresume(void)
{
  return 0;
}

int (*trc_suspend)(void)=dummy_suspendresume;
int (*trc_resume)(void)=dummy_suspendresume;

/* maximum number of different queues where we want to log our events */
#define TRC_MAXQUEUES 5

/*
 *
 */

/* this is the base path that is used as a prologue for all the
filenames that are passed to the tracer */
static char basepath[PATH_MAX];

/* used to create the name for a tracer file */
void trc_create_name(char *basename, int uniq, char *pathname)
{
  if (uniq) sprintf(pathname,"%s/%s%i",basepath,basename,uniq);
  else      sprintf(pathname,"%s/%s",basepath,basename);
}

/*
 *
 */

/* the flag used to discriminate if an event have to be traced or not */
#define FLAG_NOTRACE 0x01

typedef struct TAGtrc_evtinfo_t {
  trc_queue_t *queue;  /* the queue responsible for the logging of an event */
  unsigned    flags;   /* if = FLAG_NOTRACE the event must not be logged */
} trc_evtinfo_t;

/* -- */

/* one entry for each event; this array says for each event the queue to use 
and if it must be logged */
trc_evtinfo_t eventstable[TRC_NUMEVENTS];

/* For each kind of queue (see include/tracer/queues.h) there is a set of
pointers to the functions that a queue should implement */
int (*createqueue[TRC_QUEUETYPESNUMBER])(trc_queue_t *, void *);
int (*activatequeue[TRC_QUEUETYPESNUMBER])(void *,int);
int (*terminatequeue[TRC_QUEUETYPESNUMBER])(void *);

/* for each queue registered in the system, 
   the functions used to get/post an event 
   The elements of this table are initialized with calls to createqueue[type]()
   (see include/trace/queues.h) */
trc_queue_t queuetable[TRC_MAXQUEUES];

/* initialized as a dummy queue, the default value of all the queues */
trc_queue_t queuesink;

/* number of registered queues in the system */
int numqueues;

/* -- */

/* The Dummy queue */

static trc_event_t *dummy_get(void *foo)
{  
  return NULL;
}

static int dummy_post(void *foo)
{
  return 0;
}

static int dummy_createqueue(trc_queue_t *queue, void *unused)
{
  queue->get=dummy_get;
  queue->post=dummy_post;
  queue->data=NULL;
  return 0;
}

static int dummy_terminatequeue(void *unused)
{
  return 0;
}

/*
static int dummy_activatequeue(void *unused, int unused2)
{
  return 0;
}
*/

/* -- */

/* this function simply register the functions that are used to 
   handle a queue */
int trc_register_queuetype(int queuetype,
			   int(*creat)(trc_queue_t *, void *),
			   int(*activate)(void *,int),
			   int(*term)(void *))
{
  if (queuetype<0||queuetype>=TRC_QUEUETYPESNUMBER) return -1;
  createqueue[queuetype]=creat;
  activatequeue[queuetype]=activate;
  terminatequeue[queuetype]=term;
  return 0;
}

/* this function register a queue in the system.
   It uses the type to access to the queue handling functions registered 
   with the previous function (trc_register_queuetype) 
   numqueue is incremented!
   */
int trc_create_queue(int queuetype, void *args)
{
  int res;

  if (createqueue[queuetype]==dummy_createqueue) return -1;
  if (numqueues==TRC_MAXQUEUES) return -1;
  res=createqueue[queuetype](&queuetable[numqueues],args);
  if (res) return -1;
  queuetable[numqueues].type=queuetype;
  numqueues++;
  return numqueues-1;
}

/* -- */

static void (*old_logevent)(int event, void *ptr)=NULL;
static void internal_trc_logevent(int, void *ptr);

static void trc_end(void *unused)
{
  int i;
  
  printk(KERN_INFO "tracer shutdown...");
  
  /* suspend event logging */
  trc_suspend();

  /* for safety: send all events to the sink queue */
  for (i=0;i<TRC_NUMEVENTS;i++) {
    eventstable[i].queue=&queuesink;
    eventstable[i].flags|=FLAG_NOTRACE;
  }
  
  /* terminate all queues */
  for (i=0;i<numqueues;i++)
    terminatequeue[queuetable[i].type](queuetable[i].data);
}

static int internal_trc_resume(void);
static int internal_trc_suspend(void);

int TRC_init_phase1(TRC_PARMS *parms)
{ 
  int i;
  
  printk(KERN_INFO "initializing tracer...");
  
  /* all the queues are initialized to the dummy queue (sink!) */
  for (i=0;i<TRC_QUEUETYPESNUMBER;i++) {
    createqueue[i]=dummy_createqueue;
    terminatequeue[i]=dummy_terminatequeue;
  }
  
  /* the sink queue is initialized */
  dummy_createqueue(&queuesink,NULL);
  
  /* no queues registered yet */
  numqueues=0;
  
  /* all the events are initialized to put to the sink queue */
  for (i=0;i<TRC_NUMEVENTS;i++) {
    eventstable[i].queue=&queuesink;
    eventstable[i].flags=FLAG_NOTRACE;
  }
  
  /* this will end the tracer at shutdown */
  i=sys_atrunlevel(trc_end,NULL,RUNLEVEL_SHUTDOWN);

  /* initialize the parameters if not initialized */
  {
    TRC_PARMS m;
    trc_default_parms(m);
    if (parms==NULL) parms=&m;

    strncpy(basepath,parms->path,sizeof(basepath));
    basepath[sizeof(basepath)-1]='\0';
  }

  trc_suspend=internal_trc_suspend;
  trc_resume=internal_trc_resume;
   
  /* start the tracer */
  trc_resume();
  return 0;
}

/* this function simply activates all the registered queues.
   This is usually called into the init() tasks!!! */
int TRC_init_phase2(void)
{
  int i;
  for (i=0;i<numqueues;i++)
    activatequeue[queuetable[i].type](queuetable[i].data,i+1);
  return 0;
}

/* saves the current logevent function and set it as 
   the internal_trc_logevent */
static int internal_trc_resume(void)
{
  SYS_FLAGS f;
  int ret=-1;
  f=kern_fsave();
  if (old_logevent==NULL) {  
    old_logevent=trc_logevent;
    trc_logevent=internal_trc_logevent;
    ret=0;
  }
  kern_frestore(f);
  return ret;
}

/* restores the saved logevent function (initially, the logevent function is
   a dummy function) */
static int internal_trc_suspend(void)
{
  SYS_FLAGS f;
  int ret=-1;
  f=kern_fsave();
  if (old_logevent!=NULL) {    
    trc_logevent=old_logevent;
    old_logevent=NULL;
    ret=0;
  }
  kern_frestore(f);
  return 0;
}

static void internal_trc_logevent(int event, void *ptr)
{
  trc_event_t   *evt;
  trc_queue_t   *queue;
  SYS_FLAGS f;

  /* disables interrupts (this function can be called also into a task */
  f=kern_fsave();

  /* check if the event has to be logged */
  if (eventstable[event].flags&FLAG_NOTRACE) {
    kern_frestore(f);
    return;
  } 
  queue=eventstable[event].queue;
  
  /* gets a free event descriptor, fills it and post it */
  evt=queue->get(queue->data);
  if (evt!=NULL) {
    evt->event=event;
    evt->time=kern_gettime(NULL);
    memcpy(&evt->x,ptr,sizeof(trc_allevents_t));
    queue->post(queue->data);
  }
  
  kern_frestore(f);
}

/*
 *
 *
 *
 */

/* these set of functions can be used to trace or not single event and classes.
   They make use of the classtable structure, that is used to discriminate
   the indexes occupied by every class */

int classtable[TRC_NUMCLASSES+1]={
  TRC_F_TRACER,
  TRC_F_SYSTEM,
  TRC_F_USER,
  TRC_F_LL,
  TRC_F_SEM,
  TRC_F_LAST
};

#define checkevent(x)   if ((x)<0||(x)>=TRC_NUMEVENTS) return -1
#define checkqueue(x)   if ((x)<0||(x)>=numqueues) return -1
#define checkclass(x)   if ((x)<0||(x)>=TRC_NUMCLASSES) return -1

int trc_assign_event_to_queue(int event, int queue)
{
  checkevent(event);
  checkqueue(queue);
  eventstable[event].queue=&queuetable[queue];
  return 0;
}

int trc_assign_class_to_queue(int class, int queue)
{
  int i;
  checkqueue(queue);
  checkclass(class);
  for (i=classtable[class];i<classtable[class+1];i++)
    eventstable[i].queue=&queuetable[queue];
  return 0;
}

int trc_notrace_event(int event)
{
  checkevent(event);
  eventstable[event].flags|=FLAG_NOTRACE;
  return 0;
}

int trc_trace_event(int event)
{
  checkevent(event);
  eventstable[event].flags&=~FLAG_NOTRACE;
  return 0;
}

int trc_notrace_class(int class)
{
  int i;
  checkclass(class);
  for (i=classtable[class];i<classtable[class+1];i++)
    eventstable[i].flags|=FLAG_NOTRACE;
  return 0;
}

int trc_trace_class(int class)
{
  int i;
  checkclass(class);
  for (i=classtable[class];i<classtable[class+1];i++)
    eventstable[i].flags&=~FLAG_NOTRACE;
  return 0;
}


/* -- */

int TRC_init_phase1_standard(void)
{
  int qf,qc;
  int res;

  /* initialize the trace */  
  res=TRC_init_phase1(NULL);
  if (res) return res;

  /* register two kinds of queues, fixed and circular */
  res=trc_register_circular_queue();
  if (res) return res;
  res=trc_register_fixed_queue();
  if (res) return res;

  /* creates two queues:
      a circular queue for the system events, 
      a fixed queue 
      */
  qc=trc_create_queue(TRC_CIRCULAR_QUEUE,NULL);
  qf=trc_create_queue(TRC_FIXED_QUEUE,NULL);
  if (qc==-1||qf==-1) return -97;

  /* We want to trace all the system events */
  res=trc_trace_class(TRC_CLASS_SYSTEM);
  if (res) return res;
  /* All the system events must be traced into the circular queue */
  res=trc_assign_class_to_queue(TRC_CLASS_SYSTEM,qc);
  if (res) return res;

  return 0;
}

int TRC_init_phase2_standard(void)
{
  return TRC_init_phase2();
}

