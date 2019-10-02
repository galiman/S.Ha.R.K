
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

#ifndef __GLUE_H
#define __GLUE_H

#include <fs/glue.h>

#include <fs/const.h>
#include <fs/util.h>
#include <fs/assert.h>
#include <fs/maccess.h>
#include <fs/irq.h>

#include <kernel/model.h>
#include <kernel/func.h>
#include <kernel/assert.h>
#include <kernel/int_sem.h>

/*
 * mutex
 */
     
#include <kernel/int_sem.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+ a semaphore object +*/
typedef internal_sem_t __b_mutex_t;

/*
  The following macros can be used to synchronize events; all
  require a pointer to a semaphore object and return nothing;
  a semaphore can have "val" resource free.
 */

/*+ Initialize a semaphore (to integer val) +*/
#define __b_mutex_init(ptr)     internal_sem_init((ptr),1)

/*+ Wait for a semaphore +*/
#define __b_mutex_lock(ptr)     (internal_sem_wait(ptr))

/*+ Try to wait for a semaphore (return 0 on success) +*/
#define __b_mutex_trylock(ptr)  (internal_sem_wait(ptr))

/*+ Signal a semaphore +*/
#define __b_mutex_unlock(ptr)   (internal_sem_post(ptr))

#if 0

/*+ a mutex object +*/
typedef mutex_t  __b_mutex_t;

/*
  The following macros require a pointer to a mutex object and
  return nothing.
*/

extern void *bmutexattr;

/* Initialize a mutex object */
#define __b_mutex_init(ptr)    assertk(mutex_init(ptr,bmutexattr)==0)

/*+ Lock a mutex object +*/
#define __b_mutex_lock(ptr)    assertk(mutex_lock((ptr))==0)

/*+ Try to lock a mutex (return 0 on success locking) +*/
#define __b_mutex_trylock(ptr) mutex_trylock((ptr))

/*+ Unlock a mutex +*/
#define __b_mutex_unlock(ptr)  assertk(mutex_unlock(ptr)==0)

#endif

/*
 * fast mutex
 */

typedef SYS_FLAGS  __b_fastmutex_t;

/*+ Mutex initialization +*/
#define __b_fastmutex_init(ptr) 

/*+ Lock a mutex object +*/
#define __b_fastmutex_lock(ptr)    *(ptr)=kern_fsave()

/*+ Unlock a mutex +*/
#define __b_fastmutex_unlock(ptr)  kern_frestore(*(ptr))

/*
 *semaphores
 */

/*+ a semaphore object +*/
typedef internal_sem_t __b_sem_t;

/*
  The following macros can be used to synchronize events; all
  require a pointer to a semaphore object and return nothing;
  a semaphore can have "val" resource free.
 */

/*+ Initialize a semaphore (to integer val) +*/
#define __b_sem_init(ptr,val) internal_sem_init((ptr),(val))

/*+ Wait for a semaphore +*/
#define __b_sem_wait(ptr)     internal_sem_wait(ptr)

/*+ Try to wait for a semaphore (return 0 on success) +*/
#define __b_sem_trywait(ptr)  internal_sem_trywait(ptr)

/*+ Signal a semaphore +*/
#define __b_sem_signal(ptr)   internal_sem_post(ptr)

/*
 *
 */

#define panic(x) sys_panic(x)

/* timer functions */

#include <ll/time.h>

/* all in usec */

#define __gettimer() sys_gettime(NULL)

/* I need an active delay! */
#define __delayk(delta) {       \
  TIME t=__gettimer()+delta;    \
  while (__gettimer()<t);       \
}

__END_DECLS
#endif
