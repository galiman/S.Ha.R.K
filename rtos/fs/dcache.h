/*
 * Project: HARTIK (HA-rd R-eal TI-me K-ernel)
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *               Gerardo Lamastra <gerardo@sssup.it>
 *
 * Authors     : Massimiliano Giorgi <massy@hartik.sssup.it>
 * (see authors.txt for full list of hartik's authors)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it 
 */

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

/*
 * CVS :        $Id: dcache.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 This is the caching module: all the I/O requestes of filesystems MUST pass throught this to
 read/write on disks.
 ***/

#ifndef __DCACHE_H__
#define __DCACHE_H__

#include <fs/types.h>
#include <fs/magic.h>
#include <fs/bdev.h>

#include "semaph.h"
#include "rwlock.h"

#define MAXSECTORSIZE 512

typedef struct TAGdcache {

  DECLARE_MAGIC(magic);
  
  /*+ index of previous cache entry (if used) +*/
  int        prev;
  /*+ index of next cache entry +*/
  int        next;
  /*+ hash value +*/
  int        hash;

  
  /*+ device of this entry +*/
  __dev_t    device;
  /*+ logical sector of this entry +*/
  __blkcnt_t lsector;
  /*+ sector buffer +*/
  __uint8_t  buffer[MAXSECTORSIZE];
   

  
  /*+ how many threads are using this entry? +*/
  /* -1: unused and free 0: unused >0: used by 'used' thread */
  int        used;
  /*+ system tyme of last release +*/
  __time_t   time;

  
  /*+ number of threads waiting for 'ready' field +*/
  int        numblocked;
  /*+ all 'numblocked' threads wait on this semaphore +*/
  __fs_sem_t sync;
  /*+ for error syncronization [see __purge_dcache()] +*/
  __fs_sem_t esync;
  

  /*+ reading/writing mutex object [see rwlock.c module] +*/
  __rwlock_t rwlock;

  
  /* flags */
  
  /*+ the buffer has been modified +*/
  __uint32_t dirty:1;
  /*+ the buffer is ready +*/
  __uint32_t ready:1;
  /*+ request for error syncronization [see __purge_dcache()] +*/
  __uint32_t esyncreq:1;
  /*+ if set a write throught cache policy for this entry is not honoured +*/
  __uint32_t skipwt:1;
  
} dcache_t;

int dcache_init(void);
int dcache_end(int flag);

dcache_t *dcache_lock(__dev_t dev, __blkcnt_t lsector);
void dcache_unlock(dcache_t *);
dcache_t *dcache_acquire(__dev_t dev, __blkcnt_t lsector);
void dcache_release(dcache_t *);

void dcache_dirty(dcache_t *d);

static __inline__ void dcache_skipwt(dcache_t *d)
{
  d->skipwt=1;
}

int  dcache_flush(void);
int dcache_purgedevice(__dev_t device, int cont);

/* -- */

#define DCACHE_OK    BDEV_OK
#define DCACHE_FAIL  BDEV_FAIL
#define DCACHE_ERR   BDEV_ERROR

extern __inline__ int dcache_lockdevice(__dev_t device)
{
  return bdev_trylock(device);
}

extern __inline__ int dcache_unlockdevice(__dev_t device)
{
  return bdev_tryunlock(device);
}


#endif


