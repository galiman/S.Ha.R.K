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
 CVS :        $Id: nop.c,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 Binary Semaphores. see nop.h for more details...

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


#include <nop/nop/nop.h>

#include <ll/ll.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

/* The NOP resource level descriptor */
typedef struct {
  mutex_resource_des m;   /*+ the mutex interface +*/
} NOP_mutex_resource_des;


/* this is the structure normally pointed by the opt field in the
   mutex_t structure */
typedef struct {
  PID owner;
  IQUEUE blocked;
} NOP_mutex_t;


/* Wait status for this library */
#define NOP_WAIT LIB_STATUS_BASE

static int NOP_res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  return -1;
}

static void NOP_res_detach(RLEVEL l, PID p)
{
}

static int NOP_init(RLEVEL l, mutex_t *m, const mutexattr_t *a)
{
  NOP_mutex_t *p;
  
  if (a->mclass != NOP_MCLASS)
    return -1;

  p = (NOP_mutex_t *) kern_alloc(sizeof(NOP_mutex_t));

  /* control if there is enough memory; no control on init on a
     non- destroyed mutex */

  if (!p)
    return (ENOMEM);

  p->owner = NIL;
  iq_init(&p->blocked, &freedesc, 0);

  m->mutexlevel = l;
  m->opt = (void *)p;

  return 0;
}


static int NOP_destroy(RLEVEL l, mutex_t *m)
{
//  NOP_mutex_resource_des *lev = (NOP_mutex_resource_des *)(resource_table[l]);
  SYS_FLAGS f;

  if ( ((NOP_mutex_t *)m->opt)->owner != NIL)
    return (EBUSY);

  f = kern_fsave();
  if (m->opt) {
    kern_free(m->opt,sizeof(NOP_mutex_t));
    m->opt = NULL;
  }
  kern_frestore(f);

  return 0;
}

static int NOP_lock(RLEVEL l, mutex_t *m)
{
  NOP_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (NOP_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, initialize it! */
    NOP_mutexattr_t a;
    NOP_mutexattr_default(a);
    NOP_init(l, m, &a);
  }

  if (p->owner == exec_shadow) {
    /* the task already owns the mutex */
    kern_frestore(f);
    return (EDEADLK);
  }

  if (p->owner != NIL)  {           /* We must block exec task   */
       LEVEL l;            /* for readableness only */
     
       proc_table[exec_shadow].context = kern_context_save();
       kern_epilogue_macro();
     
       l = proc_table[exec_shadow].task_level;
       level_table[l]->public_block(l,exec_shadow);

       /* we insert the task in the semaphore queue */
       proc_table[exec_shadow].status = NOP_WAIT;
       iq_insertlast(exec_shadow,&p->blocked);

       /* and finally we reschedule */
       exec = exec_shadow = -1;
       scheduler();
       kern_context_load(proc_table[exec_shadow].context);
  }
  else {
    /* the mutex is free, We can lock it! */
    p->owner = exec_shadow;
    kern_frestore(f);
  }

  return 0;
}

static int NOP_trylock(RLEVEL l, mutex_t *m)
{
  NOP_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (NOP_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, initialize it! */
    NOP_mutexattr_t a;
    NOP_mutexattr_default(a);
    NOP_init(l, m, &a);
  }

  if (p->owner != NIL)  {
    /* a task already owns the mutex */
    kern_frestore(f);
    return (EBUSY);
  }
  else {
    /* the mutex is free, We can lock it! */
    p->owner = exec_shadow;
    kern_frestore(f);
  }

  return 0;
}

static int NOP_unlock(RLEVEL l, mutex_t *m)
{
  NOP_mutex_t *p;

  p = (NOP_mutex_t *)m->opt;
  if (!p)
    return (EINVAL);

  if (p->owner != exec_shadow) {
    /* the mutex is owned by another task!!! */
    kern_sti();
    return (EPERM);
  }

  proc_table[exec_shadow].context = kern_context_save();

  /* the mutex is mine, pop the firsttask to extract */
  p->owner = iq_getfirst(&p->blocked);
  if (p->owner != NIL) {
    l = proc_table[p->owner].task_level;
    level_table[l]->public_unblock(l,p->owner);
  }

  scheduler();
  kern_context_load(proc_table[exec_shadow].context);

  return 0;
}

RLEVEL NOP_register_module(void)
{
  RLEVEL l;                  /* the level that we register */
  NOP_mutex_resource_des *m;  /* for readableness only */

  printk("NOP_register_module\n");

  /* request an entry in the level_table */
  l = resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m = (NOP_mutex_resource_des *)kern_alloc(sizeof(NOP_mutex_resource_des));

  /* update the level_table with the new entry */
  resource_table[l] = (resource_des *)m;

  /* fill the resource_des descriptor */
  m->m.r.rtype                       = MUTEX_RTYPE;
  m->m.r.res_register                = NOP_res_register;
  m->m.r.res_detach                  = NOP_res_detach;

  /* fill the mutex_resource_des descriptor */
  m->m.init                          = NOP_init;
  m->m.destroy                       = NOP_destroy;
  m->m.lock                          = NOP_lock;
  m->m.trylock                       = NOP_trylock;
  m->m.unlock                        = NOP_unlock;

  return l;
}

