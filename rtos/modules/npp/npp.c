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
 CVS :        $Id: npp.c,v 1.1 2005/02/25 10:45:36 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:45:36 $
 ------------

 Non Preemptive Protocol. see npp.h for more details...

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


#include <npp/npp/npp.h>

#include <ll/ll.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>

/* The NPP resource level descriptor */
typedef struct {
  mutex_resource_des m;   /*+ the mutex interface +*/

  int nlocked;  /*+ how many mutex a task currently locks +*/
} NPP_mutex_resource_des;


#if 0
/*+ print resource protocol statistics...+*/
static void NPP_resource_status(RLEVEL r)
{
  NPP_mutex_resource_des *m = (NPP_mutex_resource_des *)(resource_table[r]);

  kern_printf("%d Resources owned by the tasks %d\n", m->nlocked, exec_shadow);
}
#endif

static int NPP_res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  /* NPP works with all tasks without Resource parameters */
  return -1;
}

static void NPP_res_detach(RLEVEL l, PID p)
{
  NPP_mutex_resource_des *m = (NPP_mutex_resource_des *)(resource_table[l]);

  if (m->nlocked)
    kern_raise(XMUTEX_OWNER_KILLED, p);
}

static int NPP_init(RLEVEL l, mutex_t *m, const mutexattr_t *a)
{
  if (a->mclass != NPP_MCLASS)
    return -1;

  m->mutexlevel = l;
  m->opt = (void *)NIL;

  return 0;
}


static int NPP_destroy(RLEVEL l, mutex_t *m)
{
//  NPP_mutex_resource_des *lev = (NPP_mutex_resource_des *)(resource_table[l]);

  if ( ((PID) m->opt) != NIL)
    return (EBUSY);

  return 0;
}

static int NPP_lock(RLEVEL l, mutex_t *m)
{
  NPP_mutex_resource_des *lev;
  SYS_FLAGS f;

  f = kern_fsave();

  if (((PID)m->opt) == exec_shadow) {
    /* the task already owns the mutex */
    kern_frestore(f);
    return (EDEADLK);
  }

  /* p->opt == NIL (It can't be the case of p->opt != NIL and != exec_shadow
     because when a task lock a mutex it become unpreemptable */

  /* the mutex is free, We can lock it! */
  lev = (NPP_mutex_resource_des *)(resource_table[l]);

  if (!lev->nlocked) task_nopreempt();
  lev->nlocked++;

  m->opt = (void *)exec_shadow;

  kern_frestore(f);

  return 0;
}

// static int NPP_trylock(RLEVEL l, mutex_t *m) is a non-sense!

static int NPP_unlock(RLEVEL l, mutex_t *m)
{
  NPP_mutex_resource_des *lev;

  /* the mutex is mine */
  lev = (NPP_mutex_resource_des *)(resource_table[l]);
  lev->nlocked--;

  m->opt = (void *)NIL;

  if (!lev->nlocked) task_preempt();

  return 0;
}

void NPP_register_module(void)
{
  RLEVEL l;                  /* the level that we register */
  NPP_mutex_resource_des *m;  /* for readableness only */

  printk("NPP_register_module\n");

  /* request an entry in the level_table */
  l = resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m = (NPP_mutex_resource_des *)kern_alloc(sizeof(NPP_mutex_resource_des));

  /* update the level_table with the new entry */
  resource_table[l] = (resource_des *)m;

  /* fill the resource_des descriptor */
  m->m.r.rtype                       = MUTEX_RTYPE;
  m->m.r.res_register                = NPP_res_register;
  m->m.r.res_detach                  = NPP_res_detach;

  /* fill the mutex_resource_des descriptor */
  m->m.init                          = NPP_init;
  m->m.destroy                       = NPP_destroy;
  m->m.lock                          = NPP_lock;
  m->m.trylock                       = NPP_lock;   // !!!!!!!!!!!!
  m->m.unlock                        = NPP_unlock;

  /* fill the NPP_mutex_resource_des descriptor */
  m->nlocked = 0;
}

