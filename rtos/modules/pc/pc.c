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
 CVS :        $Id: pc.c,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 Priority Ceiling protocol. see pc.h for more details...

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


#include <pc/pc/pc.h>

#include <ll/ll.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

typedef struct PC_mutexstruct_t PC_mutex_t;

/* The PC resource level descriptor */
typedef struct {
  mutex_resource_des m;     /*+ the mutex interface +*/

  int nlocked[MAX_PROC];    /*+ how many mutex a task currently locks +*/

  PC_mutex_t *mlist;        /*+ the list of the busy mutexes +*/
  DWORD priority[MAX_PROC]; /*+ the PC priority of the tasks in the system +*/

  PID blocked[MAX_PROC];

} PC_mutex_resource_des;

/* this is the structure normally pointed by the opt field in the
   mutex_t structure */
struct PC_mutexstruct_t {
  PID owner;
  int nblocked;
  PID firstblocked;

  DWORD ceiling;
  PC_mutex_t *next;
  PC_mutex_t *prev;
};

/* This is the test done when a task try to lock a mutex.
   It checks if the system ceiling is less than the process priority
   It returns 1 if the task can lock the mutex, 0 otherwise */
static int PC_accept(PC_mutex_resource_des *lev, DWORD prio)
{
  PC_mutex_t *l = lev->mlist;

  while (l) {
    if (l->owner != exec_shadow)
      /* l points to a mutex owned by another task. Its ceiling is the
         system ceiling... */
      return prio < l->ceiling;

    l = l->next;
  }

  /* no busy mutexes other than mine!!! */
  return 1;
}

/* this function inserts a mutex in the mutex list.
   the code is similar to q_insert of queue.c */
static void PC_insert(PC_mutex_resource_des *lev, PC_mutex_t * m)
{
    DWORD prio;
    PC_mutex_t *p, *q;

    p = NULL;
    q = lev->mlist;
    prio = m->ceiling;

    while ((q != NULL) && (prio >= q->ceiling)) {
	p = q;
	q = q->next;
    }

    if (p != NULL)
      p->next = m;
    else
      lev->mlist = m;

    if (q != NULL) q->prev = m;

    m->next = q;
    m->prev = p;
}

/* this function extracts a mutex in the mutex list.
   the code is similar to q_extract of queue.c */
static void PC_extract(PC_mutex_resource_des *lev, PC_mutex_t * m)
{
    PC_mutex_t *p, *q;

    //kern_printf("extract: prev=%d next = %d\n",m->prev, m->next);
    p = m->prev;
    q = m->next;

    if (p == NULL) lev->mlist = q;
    else p->next = m->next;

    if (q != NULL) q->prev = m->prev;
}


#if 0
/*+ print resource protocol statistics...+*/
static void PC_resource_status(RLEVEL r)
{
  PC_mutex_resource_des *m = (PC_mutex_resource_des *)(resource_table[r]);
  PID i;

  kern_printf("Resources owned by the tasks:\n");
  for (i=0; i<MAX_PROC; i++) {
     kern_printf("%-4d", m->nlocked[i]);
  }

  kern_printf("\nPC priority of the tasks:\n");
  for (i=0; i<MAX_PROC; i++) {
     kern_printf("%-4ld", m->priority[i]);
  }
  // in the future: print the status of the blocked semaphores!

}
#endif

static int PC_res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  PC_mutex_resource_des *m = (PC_mutex_resource_des *)(resource_table[l]);
  PC_RES_MODEL *pc;

  if (r->rclass != PC_RCLASS)
    return -1;
  if (r->level && r->level !=l)
    return -1;

  pc = (PC_RES_MODEL *)r;

  m->priority[p] = pc->priority;
  m->nlocked[p] = 0;

  return 0;
}

static void PC_res_detach(RLEVEL l, PID p)
{
  PC_mutex_resource_des *m = (PC_mutex_resource_des *)(resource_table[l]);

  if (m->nlocked[p])
    kern_raise(XMUTEX_OWNER_KILLED, p);
  else
    m->nlocked[p] = 0;

  m->priority[p] = MAX_DWORD;
}

static int PC_init(RLEVEL l, mutex_t *m, const mutexattr_t *a)
{
  PC_mutex_t *p;

  if (a->mclass != PC_MCLASS)
    return -1;

  p = (PC_mutex_t *) kern_alloc(sizeof(PC_mutex_t));

  /* control if there is enough memory; no control on init on a
     non- destroyed mutex */

  if (!p)
    return (ENOMEM);

  p->owner = NIL;
  p->nblocked = 0;
  p->firstblocked = NIL;

  p->ceiling = ((PC_mutexattr_t *)a)->ceiling;
  p->next = 0;


  m->mutexlevel = l;
  m->opt = (void *)p;

  return 0;
}


static int PC_destroy(RLEVEL l, mutex_t *m)
{
//  PC_mutex_resource_des *lev = (PC_mutex_resource_des *)(resource_table[l]);
  SYS_FLAGS f;

  if ( ((PC_mutex_t *)m->opt)->nblocked)
    return (EBUSY);

  f = kern_fsave();
  if (m->opt) {
    kern_free(m->opt,sizeof(PC_mutex_t));
    m->opt = NULL;
  }
  kern_frestore(f);

  return 0;
}

/* see pi.c for informations on the blocking algorithm used */
static int PC_lock(RLEVEL l, mutex_t *m)
{
  PC_mutex_resource_des *lev = (PC_mutex_resource_des *)(resource_table[l]);
  PC_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (PC_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, return an error! */
    kern_frestore(f);
    return (EINVAL);
  }

  if (p->owner == exec_shadow) {
    /* the task already owns the mutex */
    kern_frestore(f);
    return (EDEADLK);
  }

  if (p->ceiling > lev->priority[exec_shadow]) {
    /* see POSIX standard p. 258 */
    kern_frestore(f);
    return (EINVAL);
  }

  while (!PC_accept(lev, lev->priority[exec_shadow])) {
    /* the mutex is locked by someone,
       or another mutex with greater ceiling is busy,
       "block" the task on the busy mutex with the highest ceiling
       (pointed by lev->mlist)...*/

    //kern_printf("Blocking on %d, owner=%d, exec_shadow=%d\n",lev->mlist,lev->mlist->owner,exec_shadow);
    proc_table[exec_shadow].shadow = lev->mlist->owner;
    lev->blocked[exec_shadow] = lev->mlist->firstblocked;
    lev->mlist->firstblocked = exec_shadow;
    lev->mlist->nblocked++;

    /* ... call the scheduler... */
    scheduler();
    //kern_printf("schedule: exec=%d, exec_shadow=%d\n",exec,exec_shadow);
    TRACER_LOGEVENT(FTrace_EVT_inheritance,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)proc_table[exec].context);
    kern_context_load(proc_table[exec_shadow].context);

    /* ... and reaquire the cli() before the test... */
    kern_cli();
  }

  /* the mutex is free, We can lock it! */
  lev = (PC_mutex_resource_des *)(resource_table[l]);
  lev->nlocked[exec_shadow]++;

  p->owner = exec_shadow;

  PC_insert(lev, p);

  kern_frestore(f);

  return 0;
}

static int PC_trylock(RLEVEL l, mutex_t *m)
{
  PC_mutex_resource_des *lev = (PC_mutex_resource_des *)(resource_table[l]);
  PC_mutex_t *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = (PC_mutex_t *)m->opt;
  if (!p) {
    /* if the mutex is not initialized, return an error! */
    kern_frestore(f);
    return (EINVAL);
  }

  if (p->owner == exec_shadow) {
    /* the task already owns the mutex */
    kern_frestore(f);
    return (EDEADLK);
  }

  if (p->ceiling < lev->priority[exec_shadow]) {
    /* see POSIX standard p. 258 */
    kern_frestore(f);
    return (EINVAL);
  }

  while (!PC_accept(lev, lev->priority[exec_shadow])) {
    /* a task already owns the mutex */
    kern_frestore(f);
    return (EBUSY);
  }

  /* the mutex is free, We can lock it! */
  lev = (PC_mutex_resource_des *)(resource_table[l]);
  lev->nlocked[exec_shadow]++;

  p->owner = exec_shadow;

  PC_insert(lev, p);

  kern_frestore(f);

  return 0;
}

static int PC_unlock(RLEVEL l, mutex_t *m)
{
  PC_mutex_resource_des *lev;
  PC_mutex_t *p;
  int i, j;

  p = (PC_mutex_t *)m->opt;
  if (!p)
    return (EINVAL);

  if (p->owner != exec_shadow) {
    /* the mutex is owned by another task!!! */
    kern_sti();
    return (EPERM);
  }

  proc_table[exec_shadow].context = kern_context_save();

  /* the mutex is mine */
  lev = (PC_mutex_resource_des *)(resource_table[l]);
  lev->nlocked[exec_shadow]--;

  p->owner = NIL;

  /* we unblock all the waiting tasks... */
  i = p->firstblocked;
  p->firstblocked = NIL;

  while (i != NIL) {
    proc_table[i].shadow = j = i;
    i = lev->blocked[i];
    lev->blocked[j] = NIL;
  }
  p->nblocked = 0;

  PC_extract(lev, p);

/*  {
   int xxx;
   kern_printf("(PC_unlock owner=%d ",p->owner);
   for (xxx = 0; xxx<5; xxx++) kern_printf("p%d s%d|",xxx, proc_table[xxx].shadow);
   kern_printf(")\n");
  }*/

  scheduler();
  TRACER_LOGEVENT(FTrace_EVT_inheritance,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)proc_table[exec].context);
  kern_context_load(proc_table[exec_shadow].context);

  return 0;
}

RLEVEL PC_register_module(void)
{
  RLEVEL l;                  /* the level that we register */
  PC_mutex_resource_des *m;  /* for readableness only */
  PID i;                     /* a counter */

  printk("PC_register_module\n");

  /* request an entry in the level_table */
  l = resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m = (PC_mutex_resource_des *)kern_alloc(sizeof(PC_mutex_resource_des));

  /* update the level_table with the new entry */
  resource_table[l] = (resource_des *)m;

  /* fill the resource_des descriptor */
  m->m.r.rtype                       = MUTEX_RTYPE;
  m->m.r.res_register                = PC_res_register;
  m->m.r.res_detach                  = PC_res_detach;

  /* fill the mutex_resource_des descriptor */
  m->m.init                          = PC_init;
  m->m.destroy                       = PC_destroy;
  m->m.lock                          = PC_lock;
  m->m.trylock                       = PC_trylock;
  m->m.unlock                        = PC_unlock;

  /* fill the PC_mutex_resource_des descriptor */
  for (i=0; i<MAX_PROC; i++)
    m->nlocked[i] = 0, m->priority[i] = MAX_DWORD, m->blocked[i] = NIL;

  m->mlist = NULL;

  return l;

}

/*+ This function gets the ceiling of a PC mutex, and it have to be called
    only by a task that owns the mutex.
    Returns -1 if the mutex is not a PC mutex, 0 otherwise +*/
int PC_get_mutex_ceiling(const mutex_t *mutex, DWORD *ceiling)
{
  resource_des *r;

  if (!mutex)
    return -1;

  r = resource_table[mutex->mutexlevel];

  if (ceiling)
    *ceiling = ((PC_mutex_t *)mutex->opt)->ceiling;
  else
    return -1;

  return 0;
}

/*+ This function sets the ceiling of a PC mutex, and it have to be called
    only by a task that owns the mutex.
    Returns -1 if the mutex is not a PC mutex, 0 otherwise +*/
int PC_set_mutex_ceiling(mutex_t *mutex, DWORD ceiling, DWORD *old_ceiling)
{
  resource_des *r;

  if (!mutex)
    return -1;

  r = resource_table[mutex->mutexlevel];

  if (old_ceiling)
    *old_ceiling = ((PC_mutex_t *)mutex->opt)->ceiling;

  ((PC_mutex_t *)mutex->opt)->ceiling = ceiling;
  return 0;
}

void PC_set_task_ceiling(RLEVEL r, PID p, DWORD priority)
{
  PC_mutex_resource_des *m = (PC_mutex_resource_des *)(resource_table[r]);
  m->priority[p] = priority;
}

