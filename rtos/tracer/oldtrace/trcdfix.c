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

#include <ll/i386/x-dos.h>

/* this file implement a fixed queue, that is simply an array that
is filled with the events until it is full. After that, all the other 
events are discarded. It uses the DOSFS Filesystem to write all the data 

This file is derived from the trcfixed.c file; I used a different file
because including trcfixed.c in the executable would have implied the
linking of all the filesystem...
*/




typedef struct TAGfixed_queue_t {
  int         size;
  int         index;
  char        *filename;
  int         uniq;
  
  trc_event_t table[0]; 
  /* Yes, 0!... the elements are allocated 
     in a dirty way into the kern_alloc into fixed_create */
} dosfs_fixed_queue_t;

/* This function simply return an event to fill (only if the fixed table
is not yet full) */
static trc_event_t *dosfs_fixed_get(dosfs_fixed_queue_t *queue)
{
  if (queue->index>=queue->size) return NULL;
  return &queue->table[queue->index++];
}

/* since get returns the correct event address,
the post function does nothing... */
static int dosfs_fixed_post(dosfs_fixed_queue_t *queue)
{
  return 0;
}

static TRC_FIXED_PARMS defaultargs;
static int once=0;

static void dosfs_fixed_flush(void *arg);

static int dosfs_fixed_create(trc_queue_t *queue, TRC_FIXED_PARMS *args)
{
  dosfs_fixed_queue_t *ptr;

  /* initialize the default arguments for the fixed queue */ 
  if (!once) {
    /* well... this func is called when the system is not running! */
    once=1;
    trc_fixed_default_parms(defaultargs);
  }
  if (args==NULL) args=&defaultargs;
  
  /* allocate the fixed queue data structure plus the array of events */
  ptr=(dosfs_fixed_queue_t*)kern_alloc(sizeof(dosfs_fixed_queue_t)+
				 sizeof(trc_event_t)*(args->size+1));
  if (ptr==NULL) return -1;

  /* set the current queue pointers and data */
  queue->get=(trc_event_t*(*)(void*))dosfs_fixed_get;
  queue->post=(int(*)(void*))dosfs_fixed_post;
  queue->data=ptr;

  ptr->size=args->size;
  ptr->index=0;
  ptr->filename=args->filename;

  /* prepare for shutdown ;-) */
  sys_atrunlevel(dosfs_fixed_flush, (void *)ptr, RUNLEVEL_AFTER_EXIT);

  return 0;
}

static void dosfs_fixed_flush(void *arg)
{
  DOS_FILE *f;
  dosfs_fixed_queue_t *queue = (dosfs_fixed_queue_t *)arg;
  
  char pathname[100]; /* it should be PATH_MAX, but we do not use the
                         filesystem, so the symbol is not defined */

  if (queue->filename==NULL) trc_create_name("fix",queue->uniq,pathname);
  else trc_create_name(queue->filename,0,pathname);

  printk(KERN_DEBUG "tracer flush index= %d pathname=%s\n", 
	 queue->index, pathname);

  f = DOS_fopen(pathname,"w");

  DOS_fwrite(queue->table,1,queue->index*sizeof(trc_event_t),f);

  DOS_fclose(f);

}

static int dosfs_fixed_activate(dosfs_fixed_queue_t *queue, int uniq)
{
  queue->uniq=uniq;
  return 0;
}

static int dosfs_fixed_terminate(dosfs_fixed_queue_t *queue)
{
  return 0;
}

int trc_register_dosfs_fixed_queue(void)
{
  int res;
  
  res=trc_register_queuetype(TRC_DOSFS_FIXED_QUEUE,
			     (int(*)(trc_queue_t*,void*))dosfs_fixed_create,
			     (int(*)(void*,int))dosfs_fixed_activate,
			     (int(*)(void*))dosfs_fixed_terminate
			     );
  
  if (res!=0) printk(KERN_WARNING "can't register tracer DOSFS fixed queue");
  return res;
}
