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
 CVS :        $Id: nopm.c,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 See modules/nopm.h.
 This code is a copy of nop.c with minor modifications.
**/

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


#include <nopm/nopm/nopm.h>

#include <ll/ll.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

/* The NOPM resource level descriptor */
typedef struct {
  mutex_resource_des m;   /*+ the mutex interface +*/
} NOPM_mutex_resource_des;


/* this is the structure normally pointed by the opt field in the
   mutex_t structure */
typedef struct {
  PID owner;
  IQUEUE blocked;
  int counter;
} NOPM_mutex_t;









#define MAXTABLE 4096
static mutex_t *table[MAXTABLE];
static int index=0;

static int register_nopm(mutex_t *p)
{
  if (index>=MAXTABLE) return -1;
  table[index++]=p;
  return 0;
}

void dump_nopm_table(void)
{
  NOPM_mutex_t *ptr;
  SYS_FLAGS f;
  PID j;
  int i;

  f=kern_fsave();
  kern_printf("nopm_mutex module TABLE\n");
  kern_printf("----------------------\n");
  for(i=0;i<index;i++) {
    ptr=table[i]->opt;
    if (!iq_isempty(&ptr->blocked)) {
      kern_printf("%i blocks on 0x%p: ",ptr->owner,table[i]);
      j=iq_query_first(&ptr->blocked);
      while (j!=NIL) {
	kern_printf("%i ",(int)j);
	j=iq_query_next(j, &ptr->blocked);
      }
      kern_printf("\n");
    } else {
      //kern_printf("0x%p no block\n",table[i]);
    }      
  }
  kern_frestore(f);

}











/* Wait status for this library */
#define NOPM_WAIT LIB_STATUS_BASE



static int NOPM_res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  return -1;
}

static void NOPM_res_detach(RLEVEL l, PID p)
{
}

static int NOPM_init(RLEVEL l, mutex_t *m, const mutexattr_t *a)
{
  NOPM_mutex_t *p;

  if (a->mclass != NOPM_MCLASS)
    return -1;

  p = (NOPM_mutex_t *) kern_alloc(sizeof(NOPM_mutex_t));

  /* control if there is enough memory; no control on init on a
     non- destroyed mutex */

  if (!p)
    return (ENOMEM);

  p->owner = NIL;
  iq_init(&p->blocked, &freedesc, 0);
  p->counter=0;
  
  m->mutexlevel = l;
  m->opt = (void *)p;
 
  /* MG */
  register_nopm(m);
  
  return 0;
}


static int NOPM_destroy(RLEVEL l, mutex_t *m)
{
//  NOPM_mutex_resource_des *lev = (NOPM_mutex_resource_des *)(resource_table[l]);
  SYS_FLAGS f;

  if ( ((NOPM_mutex_t *)m->opt)->owner != NIL)
    return (EBUSY);

  f = kern_fsave();
  if (m->opt) {
    kern_free(m->opt,sizeof(NOPM_mutex_t));
    m->opt = NULL;
  }
  kern_frestore(f);

  return 0;
}

static int NOPM_lock(RLEVEL l, mutex_t *m)
{
  NOPM_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (NOPM_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, initialize it! */
    NOPM_mutexattr_t a;
    NOPM_mutexattr_default(a);
    NOPM_init(l, m, &a);
  }

  if (p->owner == exec_shadow) {
    /* the task already owns the mutex */
    p->counter++;
    kern_frestore(f);
    return 0;
  }

  if (p->owner != NIL)  {           /* We must block exec task   */
       LEVEL l;            /* for readableness only */
     
       proc_table[exec_shadow].context = kern_context_save();
       kern_epilogue_macro();
     
       l = proc_table[exec_shadow].task_level;
       level_table[l]->public_block(l,exec_shadow);

       /* we insert the task in the semaphore queue */
       proc_table[exec_shadow].status = NOPM_WAIT;
       iq_insertlast(exec_shadow,&p->blocked);

       /* and finally we reschedule */
       exec = exec_shadow = -1;
       scheduler();
       kern_context_load(proc_table[exec_shadow].context);            
  }
  else {
    /* the mutex is free, We can lock it! */
    p->owner = exec_shadow;
    p->counter++;
    kern_frestore(f);
  }

  return 0;
}

static int NOPM_trylock(RLEVEL l, mutex_t *m)
{
  NOPM_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (NOPM_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, initialize it! */
    NOPM_mutexattr_t a;
    NOPM_mutexattr_default(a);
    NOPM_init(l, m, &a);
  }

  if (p->owner != NIL)  {
    /* a task already owns the mutex */
    kern_frestore(f);
    return (EBUSY);
  }
  else {
    /* the mutex is free, We can lock it! */
    p->owner = exec_shadow;
    p->counter++;
    kern_frestore(f);
  }

  return 0;
}

static int NOPM_unlock(RLEVEL l, mutex_t *m)
{
  NOPM_mutex_t *p;
  PID e;

  p = (NOPM_mutex_t *)m->opt;
  if (!p)
    return (EINVAL);

  if (p->owner != exec_shadow) {
    /* the mutex is owned by another task!!! */
    kern_printf("wrongunlock<owner=%i,unlocker=%i>",p->owner,exec_shadow);
    kern_sti(); 
    return (EPERM);
  }

  p->counter--;
  if (p->counter!=0) {
    /* we have multiple lock on this mutex */
    kern_sti();
    return 0;
  }
  
  proc_table[exec_shadow].context = kern_context_save();

  /* the mutex is mine, pop the firsttask to extract */
  for (;;) {
    e = iq_getfirst(&p->blocked);
    if (e == NIL) {
      p->owner = NIL;
      break;
    } else if (proc_table[e].status == NOPM_WAIT) {
      l = proc_table[e].task_level;
      level_table[l]->public_unblock(l,e);
      p->counter++;
      break;
    }
  }

  /* MG!!! */
  p->owner = e;

  scheduler();
  kern_context_load(proc_table[exec_shadow].context);

  return 0;
}

RLEVEL NOPM_register_module(void)
{
  RLEVEL l;                  /* the level that we register */
  NOPM_mutex_resource_des *m;  /* for readableness only */

  printk("NOPM_register_module\n");

  /* request an entry in the level_table */
  l = resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m = (NOPM_mutex_resource_des *)kern_alloc(sizeof(NOPM_mutex_resource_des));

  /* update the level_table with the new entry */
  resource_table[l] = (resource_des *)m;

  /* fill the resource_des descriptor */
  m->m.r.rtype                       = MUTEX_RTYPE;
  m->m.r.res_register                = NOPM_res_register;
  m->m.r.res_detach                  = NOPM_res_detach;

  /* fill the mutex_resource_des descriptor */
  m->m.init                          = NOPM_init;
  m->m.destroy                       = NOPM_destroy;
  m->m.lock                          = NOPM_lock;
  m->m.trylock                       = NOPM_trylock;
  m->m.unlock                        = NOPM_unlock;

  return l;
}

