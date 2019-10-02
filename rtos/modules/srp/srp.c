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
 CVS :        $Id: srp.c,v 1.1 2005/02/25 10:40:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:40:58 $
 ------------

 Stack Resource Policy. see srp.h for general details...


 HOW the shadows are managed in this module
 ------------------------------------------

 All the task that use SRP are inserted in an ordered list, called tasklist.

 when a task lock a mutex and change the system ceiling, all the shadows
 of the tasks with preemption level <= are set to the locking task, and
 viceversa when a mutex is unlocked.

 The real algorithm is slightly different: for example consider a task set
 of 8 tasks. We represent each task here as (PID, shadow, preemption level).

 There is also a field, current, used to scan the tasklist.

 When the system starts, the situation is as follows:

 system ceiling = 0, current = NIL
 (a,a,1) (b,b,2) (c,c,2) (d,d,2) (e,e,3) (f,f,4) (g,g,4) (h,h,5)

 for example, task a is scheduled, and lock a mutex that cause the system
 ceiling to become 2. The situation will be the following:

 system ceiling = 2, current = d
 (a,a,1) (b,a,2) (c,a,2) (d,a,2) (e,e,3) (f,f,4) (g,g,4) (h,h,5)

 Now suppose that task f preempts on task a. (no change to the shadows)

 Then the task f locks a mutex and the system ceiling become 4. The shadows
 will be set as follows:

 system ceiling = 4, current = g
 (a,f,1) (b,a,2) (c,a,2) (d,a,2) (e,f,3) (f,f,4) (g,f,4) (h,h,5)

 The system maintains a stack of the locked mutexes. each mutex has in the
 descriptor the space for implementing a stack, useful in the unlock()
 function to undo the modify done whith the last lock()...

 This approach minimizes the number of shadows to be set, so minimizes
 the complexity of the lock/unlock operations.

 Unfortunately, it creates a tree in the shadows (i.e., when sys_ceiling=4,
 task c points to task a that points to task f, and so on....). This may
 cause a performance a little worse with respect to a one-jump shadow set.
 This is not a big problem because when a task is preempted it is very
 difficult (if not impossible!) that it may be rescheduled before the end
 of another high priority task.

 Dynamic creation and termination of tasks
 -----------------------------------------
 This module allows dynamic creation and termination of tasks.

 To be correct the system have to really activate the task only when the
 system ceiling is 0.

 To implement this there is a list, the lobbylist, that contains that tasks.

 When a task is created and the system ceiling is > 0, the task is inserted
 on the top of the list, and his activation are frozen via a call to
 task_block_activations.

 When the system_ceiling returns to 0, the lobby list is purged and for each
 task in that list the task_unblock_activations is called. if the function
 return a number >0, a task call task_activate is done on the task.

 the tasks are inserted into the lobby list using only the next field.



 When a mutex is destryed or a task is created or killed, the ceiling
 have to be recalculated. The recalc is made when the system ceiling go down
 to 0. to know whitch are the mutexes that need the operation they are
 inserted into the srp_recalc list.


 The SRP_usemutex function (see srp.h) is used to declare the used mutexes
 of a task. Why this and how it works?
 In this way, a task can insert directly the list of the mutexes that it uses
 without allocating others resource models, but using directly the mutexes
 that MUST be (in any case) initialized before the task creation...
 This is done in a simple way, inheriting the SRP_mutex_t from the RES_MODEL.
 When a task registers a mutex, the SRP module receive the pointer to that
 mutex, so it can do all the stuffs with the needed data structures.

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


#include <srp/srp/srp.h>

#include <ll/ll.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

#include <tracer.h>

typedef struct SRP_mutexstruct_t SRP_mutex_t;

/* The SRP resource level descriptor */
typedef struct {
  mutex_resource_des m;   /*+ the mutex interface +*/

  int nlocked[MAX_PROC];  /*+ how many mutex a task currently locks +*/

  struct {
    DWORD preempt;
    PID next;
    PID prev;
  } proc_preempt[MAX_PROC]; /*+ the preemption level of each task in the
                                system; if a task don't use SRP its value
                                is 0; if a task use SRP the field preempt
                                is != 0 and the item is enqueued in the
                                ordered list tasklist +*/

  PID tasklist;             /*+ A list of all the task that can use SRP,
                                ordered by the preemption level of each
                                task.                  +*/
  PID current;              /*+ A pointer used to set shadows +*/

  PID lobbylist;            /*+ A list for all the new tasks created when
                                the system ceiling is != 0. These tasks
                                will be inserted into tasklist when the
                                ceiling return to 0. +*/
  SRP_mutex_t *srpstack;    /*+ this is the stack where we store the system
                                ceiling +*/

  SRP_mutex_t *srprecalc;   /*+ the list of all mutexes that need a ceiling
                                recalc +*/

  SRP_mutex_t *srplist;     /*+ an unordered list of all created SRP
                                mutexes +*/

} SRP_mutex_resource_des;


/* this is the structure normally pointed by the opt field in the
   mutex_t structure */
struct SRP_mutexstruct_t {
  RES_MODEL r;  /*+ This little trick make possible the use of
                    SRP_usemutex                                +*/

  /* because the number of mutexes that can be created is not limited,
     the stack normally used to store the system ceiling is implemented
     through these two fields in the mutex descriptor. Note that the mutex
     are mono-resource, so when we alloc space for a mutex descriptor we
     alloc also the needed space for the stack... */
  DWORD sysceiling; /*+ The system ceiling; this field contains
                        - a meaningless value if the struct is not inserted
                          into the srpstack
                        - the system ceiling if the struct is on the top of
                          the srpstack
                        - a "frozen" system ceiling if the struct is not on
                          the top of the srpstack.
                        when a mutex is locked, it is inserted into srpstack
                        updating the system ceiling automatically
                        +*/
  SRP_mutex_t *srpstack_next; /*+ the next entry on the srpstack +*/



  BYTE use[MAX_PROC]; /*+ use[p]==1 if the task p declared that it uses the
                          mutex +*/

  DWORD ceiling;      /*+ max premption level of the tasks that use the mutex +*/

  PID owner;          /*+ the task that owns the mutex, NIL otherwise +*/

  int in_recalc_list; /*+ a flag: 1 if the mutex is in the recalc list +*/
  SRP_mutex_t *srprecalc_next; /*+ the next item in the recalc list +*/
  SRP_mutex_t *srprecalc_prev; /*+ the prev item; useful in extractions +*/

  SRP_mutex_t *srplist_next; /*+ the next item in the srplist list +*/
  SRP_mutex_t *srplist_prev; /*+ the prev item; useful in extractions+*/
};











/* -----------------------------------------------------------------------
   LISTS HANDLING
   ----------------------------------------------------------------------- */

/*+ this function inserts a task into the tasklist ordered list +*/
static void SRP_insert_tasklist(SRP_mutex_resource_des *m, PID t)
{
   PID p,q;

   p = NIL;
   q = m->tasklist;

   while ((q != NIL) &&
          (m->proc_preempt[t].preempt >= m->proc_preempt[q].preempt)) {
  	p = q;
  	q = m->proc_preempt[q].next;
   }

   if (p != NIL)
     m->proc_preempt[p].next = t;
   else
     m->tasklist = t;

   if (q != NIL) m->proc_preempt[q].prev = t;

   m->proc_preempt[t].next = q;
   m->proc_preempt[t].prev = p;
}

/*+ this function extracts a task from the tasklist +*/
static void SRP_extract_tasklist(SRP_mutex_resource_des *m, PID i)
{
    PID p,q;

    p = m->proc_preempt[i].prev;
    q = m->proc_preempt[i].next;

    if (p == NIL) m->tasklist = q;
    else m->proc_preempt[p].next = m->proc_preempt[i].next;

    if (q != NIL) m->proc_preempt[q].prev = m->proc_preempt[i].prev;
}


/*+ this function inserts a task into the lobbylist (in an unordered way) +*/
static void SRP_insertfirst_lobbylist(SRP_mutex_resource_des *m, PID p)
{
  m->proc_preempt[p].next = m->lobbylist;
  m->proc_preempt[p].prev = NIL;

  m->proc_preempt[m->lobbylist].prev = p;
  m->lobbylist = p;
}

/*+ this function extract the first task from the lobbylist
    the lobbylist must be not-empty!!!! +*/
static __inline__ PID SRP_extractfirst_lobbylist(SRP_mutex_resource_des *m)
{
  PID lobby = m->lobbylist;
  m->lobbylist = m->proc_preempt[m->lobbylist].next;
  return lobby;
}



/*+ This function insert a mutex into the recalc list ONLY if the mutex
    isn't already in that list... +*/
static void SRP_insertfirst_recalclist(SRP_mutex_resource_des *m,
                                      SRP_mutex_t *mut)
{
  if (!mut->in_recalc_list) {
    mut->srprecalc_next = m->srprecalc;
    mut->srprecalc_prev = NULL;
    if (m->srprecalc) m->srprecalc->srprecalc_prev = mut;
    m->srprecalc = mut;

    mut->in_recalc_list = 1;
  }
}

/*+ this function extracts mut from the list l. +*/
static void SRP_extract_recalclist(SRP_mutex_resource_des *m,
                                   SRP_mutex_t *mut)
{
  SRP_mutex_t *p, *q;

  p = mut->srprecalc_prev;
  q = mut->srprecalc_next;

  if (p)
    p->srprecalc_next = mut->srprecalc_next;
  else
    m->srprecalc = q;

  if (q) q->srprecalc_prev = mut->srprecalc_prev;
}

/*+ this function extracts mut from the list l. +*/
static void SRP_extract_srplist(SRP_mutex_resource_des *m,
                                SRP_mutex_t *mut)
{
  SRP_mutex_t *p, *q;

  p = mut->srplist_prev;
  q = mut->srplist_next;

  if (p)
    p->srplist_next = mut->srplist_next;
  else
    m->srplist = q;

  if (q) q->srplist_prev = mut->srplist_prev;
}



/* -----------------------------------------------------------------------
   End of LISTS HANDLING
   ----------------------------------------------------------------------- */




/*+ This funcyion returns the actual system ceiling +*/
static __inline__ DWORD sysceiling(SRP_mutex_resource_des *m)
{
  if (m->srpstack)
    return m->srpstack->sysceiling;
  else
    return 0;
}

/*+ this function recalc the mutex ceiling basing on the preemption levels
    stored in the mevel m +*/
static void SRP_recalc_ceiling_value(SRP_mutex_resource_des *m,
                                     SRP_mutex_t *mut)
{
  PID p;
  int ceiling;

  ceiling = 0;
  for (p = 0; p < MAX_PROC; p++)
    if (mut->use[p] && ceiling < m->proc_preempt[p].preempt)
      ceiling = m->proc_preempt[p].preempt;

  mut->ceiling = ceiling;
}


static int SRP_res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  SRP_mutex_resource_des *m = (SRP_mutex_resource_des *)(resource_table[l]);

  if (r->level && r->level !=l)
    return -1;

  if (r->rclass == SRP_RCLASS) {
    /* SRP_RES_MODEL resource model */
//  kern_printf("!%d %d",((SRP_RES_MODEL *)r)->preempt,p);

    if (m->proc_preempt[p].preempt == 0) {
      /* only the first SRP_RES_MODEL is considered */
      SRP_RES_MODEL *srp = (SRP_RES_MODEL *)r;

      m->proc_preempt[p].preempt = srp->preempt;
//      kern_printf("res_register: preempt=%d, p=%d\n",srp->preempt,p);

      /* insert the new task in the ordered list tasklist or in the lobby
         list */
      if (m->srpstack) {
        SRP_insertfirst_lobbylist(m,p);
        /* we have also to freeze the activations... */
        task_block_activation(p);
//        kern_printf("LOBBY!!!");
      }
      else
        SRP_insert_tasklist(m,p);
    }

    m->nlocked[p] = 0;
    return 0;
  }
  else if (r->rclass == SRP2_RCLASS) {
    /* a mutex passed via SRP_useres() */
    SRP_mutex_t *mut = (SRP_mutex_t *)r;

    if (mut->use[p])
      /* the mutex is already registered, do nothing! */
      return -1;

    /* register the mutex for the task */
    mut->use[p] = 1;

    if (m->srpstack)
      SRP_insertfirst_recalclist(m,mut);
    else {
      /* we recalc the mutex ceiling */
      if (mut->ceiling < m->proc_preempt[p].preempt)
        mut->ceiling = m->proc_preempt[p].preempt;

    }
    return 0;
  }
  else 
    return -1;
}

static void SRP_res_detach(RLEVEL l, PID p)
{
  SRP_mutex_resource_des *m = (SRP_mutex_resource_des *)(resource_table[l]);
  SRP_mutex_t *mut;

  if (m->proc_preempt[p].preempt == 0)
    return;

  if (m->nlocked[p])
    kern_raise(XMUTEX_OWNER_KILLED, p);
  else
    m->nlocked[p] = 0;

  for (mut = m->srplist; mut; mut = mut->srplist_next)
  {
    if (!mut->use[p])
      /* the mutex is not registered, do nothing! */
      continue;

    /* unregister the mutex for the task */
    mut->use[p] = 0;

    if (m->srpstack)
      SRP_insertfirst_recalclist(m,mut);
    else
      SRP_recalc_ceiling_value(m,mut);
  }

  /* check if current points to the task being killed */
  if (m->current == p)
    m->current = m->proc_preempt[m->current].prev;

  /* remove the task from the tasklist */
  SRP_extract_tasklist(m, p);
}

static int SRP_init(RLEVEL l, mutex_t *m, const mutexattr_t *a)
{
  SRP_mutex_resource_des *lev = (SRP_mutex_resource_des *)(resource_table[l]);
  SRP_mutex_t *p;
  PID x;

  if (a->mclass != SRP_MCLASS)
    return -1;

  p = (SRP_mutex_t *) kern_alloc(sizeof(SRP_mutex_t));

  /* control if there is enough memory; no control on init on a
     non- destroyed mutex */

  if (!p)
    return (ENOMEM);

  res_default_model(p->r, SRP2_RCLASS);
  p->sysceiling    = 0;     /* dummy value :-) */
  p->srpstack_next = NULL;  /* dummy value :-) */

  for (x = 0; x < MAX_PROC; x++)
    p->use[x] = 0;

  p->ceiling = 0;
  p->owner = NIL;

  p->in_recalc_list = 0;
  p->srprecalc_next = NULL; /* dummy value :-) */
  p->srprecalc_prev = NULL; /* dummy value :-) */

  p->srplist_next = lev->srplist;
  p->srplist_prev = NULL;
  if (lev->srplist) lev->srplist->srplist_prev = p;
  lev->srplist = p;

  m->mutexlevel = l;
  m->opt = (void *)p;

  return 0;
}


static int SRP_destroy(RLEVEL l, mutex_t *m)
{
  SRP_mutex_resource_des *lev = (SRP_mutex_resource_des *)(resource_table[l]);
  SRP_mutex_t *mut;
  SYS_FLAGS f;

  mut = m->opt;

  if (mut->owner != NIL)
    return (EBUSY);

  f = kern_fsave();

  /* the mutex isn't in the srpstack, because it is not busy */

  /* check srprecalc list */
  if (mut->in_recalc_list)
    SRP_extract_recalclist(lev, mut);

  /* extract from srplist */
  SRP_extract_srplist(lev, mut);

  if (m->opt) {
    kern_free(m->opt,sizeof(SRP_mutex_t));
    m->opt = NULL;
  }
  kern_frestore(f);

  return 0;
}

static int SRP_lock(RLEVEL l, mutex_t *m)
{
  SRP_mutex_resource_des *lev = (SRP_mutex_resource_des *)(resource_table[l]);
  SRP_mutex_t *mut;
  DWORD oldsysceiling;
  SYS_FLAGS f;

  f = kern_fsave();

  mut = (SRP_mutex_t *)m->opt;
  if (!mut) {
    /* if the mutex is not initialized */
    kern_frestore(f);
    return (EINVAL);
  }

  if (mut->owner == exec_shadow) {
    /* the task already owns the mutex */
    kern_frestore(f);
    return (EDEADLK);
  }

  if (!mut->use[exec_shadow] ||
      lev->proc_preempt[exec_shadow].preempt == 0 ||
      mut->owner != NIL)
  {
//    kern_printf("SRP:lev =%d owner=%d use=%d preempt=%d exec_shadow=%d\n",
//    lev, mut->owner,
//    mut->use[exec_shadow],
//    lev->proc_preempt[exec_shadow].preempt,exec_shadow);
    kern_raise(XSRP_INVALID_LOCK, exec_shadow);
    kern_frestore(f);
    return (EINVAL);
  }

  /* we know that:
     - the task use the SRP protocol and the mutex that it wants to lock
     - the mutex is free
     => the task can lock now the mutex
  */

  lev->nlocked[exec_shadow]++;
  mut->owner = exec_shadow;

  oldsysceiling = sysceiling(lev);

  /* update the system ceiling */
  mut->sysceiling = (oldsysceiling>mut->ceiling) ?
                    oldsysceiling : mut->ceiling;

  /* update the srpstack */
  mut->srpstack_next = lev->srpstack;
  lev->srpstack = mut;

  /* if the system ceiling is changed we have to change the shadows
     Note that mut->sysceiling is the NEW sysceiling */
  if (oldsysceiling != mut->sysceiling) {
    /* we set the shadow of the last task that did a lock */
    if (mut->srpstack_next)
      proc_table[mut->srpstack_next->owner].shadow = exec_shadow;

    /* now we set the shadow field of the remainig tasks */

    /* first, get the first task to manage */
    if (lev->current == NIL)
      lev->current = lev->tasklist;
    else
      /* Note that because the sysceiling is increased by the lock, currrent
         can't be at the end of the tasklist, so the operation is legal */
      lev->current = lev->proc_preempt[lev->current].next;

    for (;;) {
      PID x;  /* for readablenesss only :-) */

      proc_table[lev->current].shadow = exec_shadow;

      /* test if we have to touch the next task in the tasklist */
      x = lev->proc_preempt[lev->current].next;
      if (x == NIL ||
          lev->proc_preempt[x].preempt > mut->sysceiling)
        break;

      /* look at the next task ! */
      lev->current = lev->proc_preempt[lev->current].next;
    }
  }

  kern_frestore(f);

  return 0;
}

/* SRP_trylock is equal to SRP_lock because the SRP_lock don't block !!! */

static int SRP_unlock(RLEVEL l, mutex_t *m)
{
  SRP_mutex_resource_des *lev;
  SRP_mutex_t *mut;
  DWORD newsysceiling;

  lev = (SRP_mutex_resource_des *)(resource_table[l]);
  mut = (SRP_mutex_t *)m->opt;

  if (!mut)
    return (EINVAL);

  if (mut->owner != exec_shadow) {
    /* the mutex is owned by another task!!! */
    kern_sti();
    return (EPERM);
  }

  if (!lev->srpstack || lev->srpstack != mut) {
    /* the mutex is not the top of the stack!!! (erroneous nesting!) */
    kern_sti();
    return (EINVAL);
  }

  proc_table[exec_shadow].context = kern_context_save();

  /* the mutex is mine and it is at the top of the stack */
  lev->nlocked[exec_shadow]--;

  mut->owner = NIL;
//  kern_printf("Ûnlocked=%dÛ",lev->nlocked[exec_shadow]);

  /* extract the top of the stack */
  lev->srpstack = lev->srpstack->srpstack_next;

  /* if the sysceiling decreases, we update the shadows */
  newsysceiling = sysceiling(lev);
  if (newsysceiling < mut->sysceiling) {
    do {
      proc_table[lev->current].shadow = lev->current;
      lev->current = lev->proc_preempt[lev->current].prev;
    } while (lev->current != NIL &&
             lev->proc_preempt[lev->current].preempt > newsysceiling);

    if (lev->srpstack)
      /* this is the stack that owns the mutex with the current sysceiling*/
      proc_table[lev->srpstack->owner].shadow = lev->srpstack->owner;
  }

  /* if it is the last mutex in the stack, handle lobbylist and srprecalc */
  if (!lev->srpstack) {
//    kern_printf("UNLOBBY:");
    while (lev->lobbylist != NIL) {
      PID x = SRP_extractfirst_lobbylist(lev);
//      kern_printf("x=%d - ",x);
      SRP_insert_tasklist(lev, x);

      /* activate the task if it was activated while in lobby list! */
      if (task_unblock_activation(x)) {
	struct timespec t;
        LEVEL sl = proc_table[x].task_level;
	kern_gettime(&t);
        level_table[sl]->public_activate(sl,x,&t);
//        kern_printf("activate it!!!");
      }
    }

    while (lev->srprecalc) {
      SRP_recalc_ceiling_value(lev, lev->srprecalc);
      SRP_extract_recalclist(lev, lev->srprecalc);
    }
  }

  scheduler();
  TRACER_LOGEVENT(FTrace_EVT_inheritance,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)proc_table[exec].context);
  kern_context_load(proc_table[exec_shadow].context);

  return 0;
}

RLEVEL SRP_register_module(void)
{
  RLEVEL l;                  /* the level that we register */
  SRP_mutex_resource_des *m;  /* for readableness only */
  PID i;                     /* a counter */

  printk("SRP_register_module\n");

  /* request an entry in the level_table */
  l = resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m = (SRP_mutex_resource_des *)kern_alloc(sizeof(SRP_mutex_resource_des));

  /* update the level_table with the new entry */
  resource_table[l] = (resource_des *)m;

  /* fill the resource_des descriptor */
  m->m.r.rtype                       = MUTEX_RTYPE;
  m->m.r.res_register                = SRP_res_register;
  m->m.r.res_detach                  = SRP_res_detach;

  /* fill the mutex_resource_des descriptor */
  m->m.init                          = SRP_init;
  m->m.destroy                       = SRP_destroy;
  m->m.lock                          = SRP_lock;
  m->m.trylock                       = SRP_lock;  /* equal!!! */
  m->m.unlock                        = SRP_unlock;

  /* fill the SRP_mutex_resource_des descriptor */
  for (i=0; i<MAX_PROC; i++) {
    m->nlocked[i]=0;
    m->proc_preempt[i].preempt = 0;
    m->proc_preempt[i].next = NIL;
    m->proc_preempt[i].prev = NIL;
  }

  m->tasklist = NIL;
  m->current = NIL;
  m->lobbylist = NIL;

  m->srpstack = NULL;
  m->srprecalc = NULL;
  m->srplist = NULL;

  return l;
}

