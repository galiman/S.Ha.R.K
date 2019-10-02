/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
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
 CVS :        $Id: mutex.c,v 1.6 2004/03/10 14:51:42 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.6 $
 Last update: $Date: 2004/03/10 14:51:42 $
 ------------

This file contains the mutex and condition variables handling functions.


The mutex init function try to find a resource level with
rtype = MUTEX_RTYPE that can accept a mutex of the type of that
contained in mutexattr_t.

when found, it simply calls the level mutex init function.

Note that a mutex attribute must be specificated, whereas the
pthread_mutex_init accepts a pthread_mutexattr equal to NULL!!!


returns an integer of value:
0       All Ok!
EAGAIN  The system lacked the necessarty resources to initialize another mutex
ENOMEM  Insufficient memory exist to initialize the mutex
EPERM   The caller does not have the privilege to perform the operation
EBUSY   An init on a non-destroyed mutex is called
EINVAL  the value specified by attr parameter is invalid
        (no resource module found to handle the rtype...)


The others functions simply call the module-dependent functions...

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



#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <errno.h>

#include <tracer.h>

/*---------------------------------------------------------------------*/
/* Mutexes                                                             */
/*---------------------------------------------------------------------*/


int mutex_init(mutex_t *mutex, const mutexattr_t *attr)
{
  RLEVEL l;
  SYS_FLAGS f;
  int result;

  f = kern_fsave();
  mutex->mutexlevel = -1;
  mutex->use = 0;

  for (l=0; l<res_levels; l++) {
     if (resource_table[l]->rtype == MUTEX_RTYPE) {
       /* the cast to mutex_resource_des is legal */
       mutex_resource_des *m = (mutex_resource_des *)resource_table[l];

       /* can the mutex level manage the mutexattr_t ? */
       if ((result = m->init(l, mutex, attr)) >=0) {
         kern_frestore(f);
         return result;
       }
     }
  }
  kern_frestore(f);

  return EINVAL;
}


int mutex_destroy(mutex_t *mutex)
{
  mutex_resource_des *m;

  if (mutex->mutexlevel == -1)
    return (EINVAL);

  if (mutex->use)
    return (EBUSY);

  m = (mutex_resource_des *)resource_table[mutex->mutexlevel];

  return m->destroy(mutex->mutexlevel, mutex);
}

int mutex_lock(mutex_t *mutex)
{
  mutex_resource_des *m;

  if (mutex->mutexlevel == -1)
    return (EINVAL);

  TRACER_LOGEVENT(FTrace_EVT_set_mutex_lock,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)(mutex));

  m = (mutex_resource_des *)resource_table[mutex->mutexlevel];

  return m->lock(mutex->mutexlevel, mutex);
}

int mutex_trylock(mutex_t *mutex)
{
  mutex_resource_des *m;

  if (mutex->mutexlevel == -1)
    return (EINVAL);

  m = (mutex_resource_des *)resource_table[mutex->mutexlevel];

  return m->trylock(mutex->mutexlevel, mutex);
}

int mutex_unlock(mutex_t *mutex)
{
  mutex_resource_des *m;

  if (mutex->mutexlevel == -1)
    return (EINVAL);

  TRACER_LOGEVENT(FTrace_EVT_set_mutex_unlock,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)(mutex));

  m = (mutex_resource_des *)resource_table[mutex->mutexlevel];

  return m->unlock(mutex->mutexlevel, mutex);
}

