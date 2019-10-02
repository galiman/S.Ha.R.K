
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

#include <fs/mutex.h>
#include <fs/semaph.h>
#include "mutex.h"
#include "rwlock.h"

void __rwlock_init(__rwlock_t *ptr)
{
  __mutex_init(&ptr->mutex,fsmutexattr);
  __sem_init(&ptr->readers_semaph,0);
  __sem_init(&ptr->writers_semaph,0);
  ptr->active_readers=0;
  ptr->active_writers=0;
  ptr->blocked_readers=0;
  ptr->blocked_writers=0;  
}

void __rwlock_rdlock(__rwlock_t *ptr)
{
  __mutex_lock(&ptr->mutex);

  if (ptr->active_writers==0&&ptr->blocked_writers==0) {
    ptr->active_readers++;
    __sem_signal(&ptr->readers_semaph);
  } else
    ptr->blocked_readers++;
  
  __mutex_unlock(&ptr->mutex);
  
  __sem_wait(&ptr->readers_semaph);
}

int __rwlock_tryrdlock(__rwlock_t *ptr)
{
  int ret;
  
  __mutex_lock(&ptr->mutex);

  ret=0;
  if (ptr->active_writers==0&&ptr->blocked_writers==0) {
    ptr->active_readers++;
    __sem_signal(&ptr->readers_semaph);
  } else
    ret=-1;
  
  __mutex_unlock(&ptr->mutex);
  
  if (ret==0) __sem_wait(&ptr->readers_semaph);
  return ret;
}

void __rwlock_wrlock(__rwlock_t *ptr)
{
  __mutex_lock(&ptr->mutex);
  
  if (ptr->active_readers==0&&ptr->active_writers==0) {
    ptr->active_writers=1;
    __sem_signal(&ptr->writers_semaph);
  }
  else
    ptr->blocked_writers++;
  
  __mutex_unlock(&ptr->mutex);
  
  __sem_wait(&ptr->writers_semaph);
}

int __rwlock_trywrlock(__rwlock_t *ptr)
{
  int ret;
  
  __mutex_lock(&ptr->mutex);

  ret=0;
  if (ptr->active_readers==0&&ptr->active_writers==0) {
    ptr->active_writers=1;
    __sem_signal(&ptr->writers_semaph);
  }
  else
    ret=-1;
  
  __mutex_unlock(&ptr->mutex);
  
  if (ret==0) __sem_wait(&ptr->writers_semaph);
  return ret;
}

void __rwlock_rdunlock(__rwlock_t *ptr)
{
  __mutex_lock(&ptr->mutex);
  
  ptr->active_readers--;
  if (ptr->active_readers==0&&ptr->blocked_writers>0) {
    ptr->active_writers=1;
    ptr->blocked_writers--;
    __sem_signal(&ptr->writers_semaph);
  }
      
  __mutex_unlock(&ptr->mutex);
}

void __rwlock_wrunlock(__rwlock_t *ptr)
{
  __mutex_lock(&ptr->mutex);

  ptr->active_writers=0;
  if (ptr->blocked_readers>0) {
    do {
      ptr->blocked_readers--;
      ptr->active_readers++;
      __sem_signal(&ptr->readers_semaph);
    } while(ptr->blocked_readers!=0);
  } else if (ptr->blocked_writers>0) {
    ptr->active_writers=1;
    ptr->blocked_writers--;
    __sem_signal(&ptr->writers_semaph);
  }
  
  __mutex_unlock(&ptr->mutex);
}
