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

/*
 * CVS :        $Id: trccirc.c,v 1.1 2003/12/10 16:48:48 giacomo Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1 $
 * Last update: $Date: 2003/12/10 16:48:48 $
 */

#include <ll/sys/types.h>
#include <ll/stdlib.h>

#include <kernel/func.h>
#include <kernel/mem.h>
#include <kernel/log.h>

#include <types.h>
#include <trace.h>
#include <queues.h>

#include <fs/fs.h>

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

/* Well... this file is very similar to trcfixed.c! */

typedef struct TAGcircular_queue_t {
  /*+ size of the queue +*/
  int         size;
  /*+ index of the next insertion into the queue +*/
  int         index;
  /*+ index of the next item to write (if online_tracer activated) +*/
  int         windex;
  /*+ number of events lost (if online_tracer activated) +*/
  long        hoops;
  /*+ filename of the trace file +*/
  char        *filename;
  /*+ flags from flags field of the initialization struct +*/
  int         flags;
  /*+ some internal models have needed of extra parameters +*/
  void        *dummy;
  /*+ unique number that identify the queue +*/
  int         uniq;
  
  /*+ events table +*/
  trc_event_t table[0];  
} circular_queue_t;

static int mustgodown=0;

static TASK online_tracer(circular_queue_t *queue)
{
  char pathname[PATH_MAX];
  int handle;
  int index;

  if (queue->filename==NULL) trc_create_name("cir",queue->uniq,pathname);
  else trc_create_name(queue->filename,0,pathname);

  if (wait_for_fs_initialization()) {
    printk(KERN_NOTICE "tracer online task not running");
    goto BADEND;
  }
    
  handle=open(pathname,O_CREAT|O_TRUNC|O_WRONLY);
  if (handle==-1) {
    printk(KERN_NOTICE "tracer file %s not created!",pathname);
    goto BADEND;
  }
  
  for (;;) {
    while (queue->index!=queue->windex) {
      index=queue->index;
      if (index<queue->windex) {
	write(handle,
	      queue->table+queue->windex,
	      (queue->size-queue->windex)*sizeof(trc_event_t)
	      );
	queue->windex=0;
	continue;
      }
      write(handle,
	    queue->table+queue->windex,
	    (index-queue->windex)*sizeof(trc_event_t)
	    );
      queue->windex=index;
    }
    if (mustgodown) break;
    
    task_endcycle();
  }
  
  close(handle);
  printk(KERN_NOTICE "tracer file %s created!",pathname);

  if (queue->hoops) 
    printk(KERN_NOTICE "tracer: %li event lost into %s",queue->hoops,pathname);

  resume_fs_shutdown();
  return NULL;

BADEND:
  resume_fs_shutdown();
  
  /*Why this? for a little bug on the task_makefree() routine */
  for (;;) {
    if (mustgodown) break;
    task_endcycle();
  }
  
  return NULL;  
}

static trc_event_t *circular_get2(circular_queue_t *queue)
{
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

static trc_event_t *circular_get1(circular_queue_t *queue)
{
  if (queue->index==queue->size-1) {
    queue->index=0;
    return &queue->table[queue->size-1];
  }
  return &queue->table[queue->index++];
}

static int circular_post(circular_queue_t *queue)
{
  return 0;
}

struct create_args {
  long             period;
  long             slice;
  circular_queue_t *queue;
};


/* to remove!!! */
/*
static void circular_create_postponed(void *foo)
{
  struct create_args *ptr=(struct create_args *)foo;
  SOFT_TASK_MODEL model;
  PID pid;

  printk(KERN_DEBUG "postponed create: START");
  
  soft_task_default_model(model);
  soft_task_def_system(model);
  soft_task_def_notrace(model);
  soft_task_def_periodic(model);
  soft_task_def_period(model,ptr->period);
  soft_task_def_met(model,ptr->slice);
  soft_task_def_wcet(model,ptr->slice);
  soft_task_def_arg(model,ptr->queue);

  printk(KERN_DEBUG "postponed create: A");
  
  kern_free(foo,sizeof(struct create_args));

  printk(KERN_DEBUG "postponed create: B");
	    
  pid=task_create("trcCirc",online_tracer,&model,NULL);
  if (pid==-1) {
    printk(KERN_ERR "can't start tracer online circular trace task");
  } else {
    printk(KERN_DEBUG "postponed create: C1");
    task_activate(pid);
    printk(KERN_DEBUG "postponed create: C2");
    suspend_fs_shutdown();
    printk(KERN_DEBUG "postponed create: C3");
  }

  printk(KERN_DEBUG "postponed create: END");
}
*/

static int circular_create(trc_queue_t *queue, TRC_CIRCULAR_PARMS *args)
{
  TRC_CIRCULAR_PARMS defaultargs;
  circular_queue_t *ptr;

  if (args==NULL) {
    trc_circular_default_parms(defaultargs);
    args=&defaultargs;
  }
  
  ptr=(circular_queue_t*)kern_alloc(sizeof(circular_queue_t)+
				    sizeof(trc_event_t)*args->size);
  if (ptr==NULL) return -1;

  queue->get=(trc_event_t*(*)(void*))circular_get1;
  queue->post=(int(*)(void*))circular_post;
  queue->data=ptr;
  
  ptr->size=args->size;
  ptr->windex=ptr->index=0;
  ptr->hoops=0;
  ptr->filename=args->filename;
  ptr->flags=args->flags;
  ptr->dummy=NULL;
  
  if (args->flags&TRC_CIRCULAR_ONLINETASK) {
    struct create_args *p;
    p=kern_alloc(sizeof(struct create_args));
    if (p==NULL) {
      printk(KERN_ERR "can't create tracer online circular trace task");
      return -1;
    }
    queue->get=(trc_event_t*(*)(void*))circular_get2;
    ptr->dummy=p;
    
    p->period=args->period;
    p->slice=args->slice;
    p->queue=ptr;
    //sys_atrunlevel(circular_create_postponed,(void*)p,RUNLEVEL_INIT);    
  }
  
  return 0;
}

static int circular_activate(circular_queue_t *queue, int uniq)
{
  queue->uniq=uniq;

  if (queue->flags&TRC_CIRCULAR_ONLINETASK) {
    
    struct create_args *ptr=(struct create_args *)queue->dummy;
    SOFT_TASK_MODEL model;
    PID pid;

    printk(KERN_DEBUG "postponed create: START");
    
    soft_task_default_model(model);
    soft_task_def_system(model);
    soft_task_def_notrace(model);
    soft_task_def_periodic(model);
    soft_task_def_period(model,ptr->period);
    soft_task_def_met(model,ptr->slice);
    soft_task_def_wcet(model,ptr->slice);
    soft_task_def_arg(model,ptr->queue);

    printk(KERN_DEBUG "postponed create: A");
  
    kern_free(queue->dummy,sizeof(struct create_args));

    printk(KERN_DEBUG "postponed create: B");
	    
    pid=task_create("trcCirc",online_tracer,&model,NULL);
    if (pid==-1) {
      printk(KERN_ERR "can't start tracer online circular trace task");
    } else {
      printk(KERN_DEBUG "postponed create: C1");
      suspend_fs_shutdown();
      printk(KERN_DEBUG "postponed create: C2");      
      task_activate(pid);
      printk(KERN_DEBUG "postponed create: C3");
    }

    printk(KERN_DEBUG "postponed create: END");    
  }
  
  return 0;
}

static TASK circular_shutdown(circular_queue_t *queue)
{
  char pathname[PATH_MAX];
  int h;
  
  if (queue->filename==NULL) trc_create_name("cir",queue->uniq,pathname);
  else trc_create_name(queue->filename,0,pathname);
  
  h=open(pathname,O_CREAT|O_TRUNC|O_WRONLY);
  if (h!=-1) {
    if (queue->index!=queue->size-1)
      write(h,
	    queue->table+queue->index+1,
	    (queue->size-queue->index-1)*sizeof(trc_event_t)
	    );
    write(h,
	  queue->table,
	  queue->index*sizeof(trc_event_t)
	  );
    close(h);
    printk(KERN_NOTICE "tracer file %s created!",pathname);
  }  else
    printk(KERN_NOTICE "tracer file %s NOT created!",pathname);
  
  resume_fs_shutdown();
  return NULL;
}

static int circular_terminate(circular_queue_t *queue)
{
  SOFT_TASK_MODEL model;
  PID pid;

  mustgodown=1;
  if (queue->flags&TRC_CIRCULAR_ONLINETASK) return 0;

  suspend_fs_shutdown();
  
  //nrt_task_default_model(model);
  //nrt_task_def_system(model);
  //nrt_task_def_arg(model,queue);

  soft_task_default_model(model);
  soft_task_def_system(model);
  soft_task_def_notrace(model);
  soft_task_def_periodic(model);
  soft_task_def_period(model,50000);
  soft_task_def_met(model,10000);
  soft_task_def_wcet(model,10000);
  soft_task_def_arg(model,queue);
  
  pid=task_create("ShutTrcCir",circular_shutdown,&model,NULL);
  if (pid==-1) {
    printk(KERN_ERR "can't start tracer shutdown task (circular)");
    return -1;
  } else 
    task_activate(pid);

  return 0;
}

int trc_register_circular_queue(void)
{
  int res;
  
  res=trc_register_queuetype(TRC_CIRCULAR_QUEUE,
			     (int(*)(trc_queue_t*,void*))circular_create,
			     (int(*)(void*,int))circular_activate,
			     (int(*)(void*))circular_terminate
			     );
  
  if (res!=0) printk(KERN_WARNING "can't register tracer circular queue");
  return res;
}
