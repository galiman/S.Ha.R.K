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

/**
 ------------
 CVS :        $Id: pi.c,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 Priority Inhertitance protocol. see pi.h for more details...

**/

/*
 * Copyright (C) 2000 Paolo Gai
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


#include <pi/pi/pi.h>

#include <ll/ll.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

/* The PI resource level descriptor */
typedef struct {
  mutex_resource_des m;   /*+ the mutex interface +*/

  int nlocked[MAX_PROC];  /*+ how many mutex a task currently locks +*/

  PID blocked[MAX_PROC];  /*+ blocked queue ... +*/
} PI_mutex_resource_des;


/* this is the structure normally pointed by the opt field in the
   mutex_t structure */
typedef struct {
  PID owner;
  int nblocked;
  PID firstblocked;
} PI_mutex_t;



#if 0
/*+ print resource protocol statistics...+*/
static void PI_resource_status(RLEVEL r)
{
  PI_mutex_resource_des *m = (PI_mutex_resource_des *)(resource_table[r]);
  PID i;

  kern_printf("Resources owned by the tasks:\n");
  for (i=0; i<MAX_PROC; i++) {
     kern_printf("%-4d", m->nlocked[i]);
  }
}
#endif

static int PI_res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  /* priority inheritance works with all tasks without Resource parameters */
  return -1;
}

static void PI_res_detach(RLEVEL l, PID p)
{
  PI_mutex_resource_des *m = (PI_mutex_resource_des *)(resource_table[l]);

  if (m->nlocked[p])
    kern_raise(XMUTEX_OWNER_KILLED, p);
}

static int PI_init(RLEVEL l, mutex_t *m, const mutexattr_t *a)
{
  PI_mutex_t *p;

  if (a->mclass != PI_MCLASS)
    return -1;

  p = (PI_mutex_t *) kern_alloc(sizeof(PI_mutex_t));

  /* control if there is enough memory; no control on init on a
     non- destroyed mutex */

  if (!p)
    return (ENOMEM);

  p->owner        = NIL;
  p->nblocked     = 0;
  p->firstblocked = NIL;

  m->mutexlevel   = l;
  m->opt          = (void *)p;

  return 0;
}


static int PI_destroy(RLEVEL l, mutex_t *m)
{
//  PI_mutex_resource_des *lev = (PI_mutex_resource_des *)(resource_table[l]);
  SYS_FLAGS f;
 
  if ( ((PI_mutex_t *)m->opt)->nblocked)
    return (EBUSY);

  f = kern_fsave();
  if (m->opt) {
    kern_free(m->opt,sizeof(PI_mutex_t));
    m->opt = NULL;
  }
  kern_frestore(f);

  return 0;
}

/* Note that in this approach, when unlocking we can't wake up only
   one thread, but we have to wake up all the blocked threads, because there
   is not a concept of priority between the task... Each woken thread have
   to retest he condition.
   Normally, they retest it only one time, because if many threads are
   unblocked, they are scheduled basing on their priority (unkown in this
   module!)... and if the slice is greather than the critical sections,
   they never block!
   */
static int PI_lock(RLEVEL l, mutex_t *m)
{
  PI_mutex_resource_des *lev = (PI_mutex_resource_des *)(resource_table[l]);
  PI_mutex_t *p;
  SYS_FLAGS f;
//  return 0;

  f =  kern_fsave();

  p = (PI_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, initialize it! */
    PI_mutexattr_t a;
    PI_mutexattr_default(a);
    PI_init(l, m, &a);
  }


  if (p->owner == exec_shadow) {
    /* the task already owns the mutex */
    kern_frestore(f);
    return (EDEADLK);
  }

  while (p->owner != NIL) {
    /* the mutex is locked by someone, "block" the task ...*/
    proc_table[exec_shadow].shadow = p->owner;
    lev->blocked[exec_shadow] = p->firstblocked;
    p->firstblocked = exec_shadow;
    p->nblocked++;
//    kern_printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    /* ... call the scheduler... */
    scheduler();
    TRACER_LOGEVENT(FTrace_EVT_inheritance,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)proc_table[exec].context);
    kern_context_load(proc_table[exec_shadow].context);

    /* ... and reaquire the cli() before the test... */
    kern_cli();
  }

  /* the mutex is free, We can lock it! */
  lev->nlocked[exec_shadow]++;

  p->owner = exec_shadow;

  kern_frestore(f);

  return 0;
}

static int PI_trylock(RLEVEL l, mutex_t *m)
{
  PI_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (PI_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, initialize it! */
    PI_mutexattr_t a;
    PI_mutexattr_default(a);
    PI_init(l, m, &a);
  }

  if (p->owner != NIL) {
    /* a task already owns the mutex */
    kern_frestore(f);
    return (EBUSY);
  }
  else {
    /* the mutex is free */
    PI_mutex_resource_des *lev = (PI_mutex_resource_des *)(resource_table[l]);
    lev->nlocked[exec_shadow]++;

    p->owner = exec_shadow;

    kern_frestore(f);
    return 0;
  }
}

static int PI_unlock(RLEVEL l, mutex_t *m)
{
  PI_mutex_resource_des *lev;
  PI_mutex_t *p;
  int i, j;

//  return 0;
  p = (PI_mutex_t *)m->opt;
  if (!p)
    return (EINVAL);

  if (p->owner != exec_shadow) {
    /* the mutex is owned by another task!!! */
    kern_sti();
    return (EPERM);
  }

  proc_table[exec_shadow].context = kern_context_save();

  /* the mutex is mine */
  lev = (PI_mutex_resource_des *)(resource_table[l]);
  lev->nlocked[exec_shadow]--;

  p->owner = NIL;

  /* we unblock all the waiting tasks... */
  i = p->firstblocked;
  p->firstblocked = NIL;

  while (i != NIL) {
//    kern_printf("<<%d>>", i);
    proc_table[i].shadow = j = i;
    i = lev->blocked[i];
    lev->blocked[j] = NIL;
  }
  p->nblocked = 0;

/*  {
   int xxx;
   kern_printf("(PI_unlock owner=%d ",p->owner);
   for (xxx = 0; xxx<5; xxx++) kern_printf("p%d s%d|",xxx, proc_table[xxx].shadow);
   kern_printf(")\n");
  }*/

  scheduler();
  TRACER_LOGEVENT(FTrace_EVT_inheritance,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)proc_table[exec].context);
  kern_context_load(proc_table[exec_shadow].context);

  return 0;
}

RLEVEL PI_register_module(void)
{
  RLEVEL l;                  /* the level that we register */
  PI_mutex_resource_des *m;  /* for readableness only */
  PID i;                     /* a counter */

  printk("PI_register_module\n");

  /* request an entry in the level_table */
  l = resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m = (PI_mutex_resource_des *)kern_alloc(sizeof(PI_mutex_resource_des));

  /* update the level_table with the new entry */
  resource_table[l] = (resource_des *)m;

  /* fill the resource_des descriptor */
  m->m.r.rtype                       = MUTEX_RTYPE;
  m->m.r.res_register                = PI_res_register;
  m->m.r.res_detach                  = PI_res_detach;

  /* fill the mutex_resource_des descriptor */
  m->m.init                          = PI_init;
  m->m.destroy                       = PI_destroy;
  m->m.lock                          = PI_lock;
  m->m.trylock                       = PI_trylock;
  m->m.unlock                        = PI_unlock;

  /* fille the PI_mutex_resource_des descriptor */
  for (i=0; i<MAX_PROC; i++) {
    m->nlocked[i] = 0;
    m->blocked[i] = NIL;
  }
  
  return l;
}

