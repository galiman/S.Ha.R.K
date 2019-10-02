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

/* this file implement a fixed queue, that is simply an array that
is filled with the events until it is full. After that, all the other 
events are discarded. */




typedef struct TAGfixed_queue_t {
  int         size;
  int         index;
  char        *filename;
  int         uniq;
  
  trc_event_t table[0]; 
  /* Yes, 0!... the elements are allocated 
     in a dirty way into the kern_alloc into fixed_create */
} fixed_queue_t;

/* This function simply return an event to fill (only if the fixed table
is not yet full) */
static trc_event_t *fixed_get(fixed_queue_t *queue)
{
  if (queue->index>=queue->size) return NULL;
  return &queue->table[queue->index++];
}

/* since get returns the correct event address,
the post function does nothing... */
static int fixed_post(fixed_queue_t *queue)
{
  return 0;
}

static TRC_FIXED_PARMS defaultargs;
static int once=0;

static int fixed_create(trc_queue_t *queue, TRC_FIXED_PARMS *args)
{
  fixed_queue_t *ptr;

  /* initialize the default arguments for the fixed queue */ 
  if (!once) {
    /* well... this func is called when the system is not running! */
    once=1;
    trc_fixed_default_parms(defaultargs);
  }
  if (args==NULL) args=&defaultargs;
  
  /* allocate the fixed queue data structure plus the array of events */
  ptr=(fixed_queue_t*)kern_alloc(sizeof(fixed_queue_t)+
				 sizeof(trc_event_t)*(args->size+1));
  if (ptr==NULL) return -1;

  /* set the current queue pointers and data */
  queue->get=(trc_event_t*(*)(void*))fixed_get;
  queue->post=(int(*)(void*))fixed_post;
  queue->data=ptr;

  ptr->size=args->size;
  ptr->index=0;
  ptr->filename=args->filename;
  return 0;
}

static TASK fixed_shutdown(fixed_queue_t *queue)
{
  char pathname[PATH_MAX];
  int h;
  
  printk(KERN_DEBUG "<fixed queuesize:%i>",queue->index);

  if (queue->filename==NULL) trc_create_name("fix",queue->uniq,pathname);
  else trc_create_name(queue->filename,0,pathname);

  h=open("/TEMP/FIX1",O_CREAT|O_TRUNC|O_WRONLY);
  if (h!=-1) {
    write(h,queue->table,queue->index*sizeof(trc_event_t));
    close(h);
    printk(KERN_NOTICE "tracer file %s created!",pathname);
  }  else {
    printk(KERN_NOTICE "tracer file %s not created!",pathname);
  }
  
  resume_fs_shutdown();
  return NULL;
}

static int fixed_activate(fixed_queue_t *queue, int uniq)
{
  queue->uniq=uniq;
  return 0;
}

static int fixed_terminate(fixed_queue_t *queue)
{
  SOFT_TASK_MODEL model;
  PID pid;

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
  
  pid=task_create("ShutTrcFix",fixed_shutdown,&model,NULL);
  if (pid==-1) {
    printk(KERN_ERR "can't start tracer shutdown task (fixed)");
    return -1;
  } else 
    task_activate(pid);

  return 0;
}

int trc_register_fixed_queue(void)
{
  int res;
  
  res=trc_register_queuetype(TRC_FIXED_QUEUE,
			     (int(*)(trc_queue_t*,void*))fixed_create,
			     (int(*)(void*,int))fixed_activate,
			     (int(*)(void*))fixed_terminate
			     );
  
  if (res!=0) printk(KERN_WARNING "can't register tracer fixed queue");
  return res;
}
