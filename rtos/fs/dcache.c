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
 * Copyright (C) 1999 Massimiliano Giorgi
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
 * CVS :        $Id: dcache.c,v 1.2 2003/03/24 11:19:56 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/24 11:19:56 $
 */

#include <fs/util.h>
#include <fs/assert.h>
#include <fs/bdev.h>
#include <fs/magic.h>
#include <fs/major.h>
#include <fs/sysmacro.h>
#include "dcache.h"
#include "mutex.h"
#include "semaph.h"
#include "rwlock.h"

#include "debug.h"

/*
 * To do:
 * - separate the "buffer" field of a cache entry by the entry itself
 *   allowing a better memory management and a variable cache block size
 *   without wasting space.
 * - improve the purging algorithm (the algorithm that remove entries from
 *   the cache).
 * - implement a server to mantains an x% of cache buffers free and to write
 *   dirty cache entries to disk during idle periods.
 */

/*
 * How the cache is implemented:
 * copy-back algorithm or write-throught
 * (define only one of this)
 */

#define COPYBACK
//#define WRITETHROUGHT

/*
 * if a write throught policy is chosen for the cache if this
 * is defined the 'skipwt' bits into the dcache_t structure is honoured
 * (ie. if 'skipwt' is set the cache entry use a copyback policy and
 * not a write throught policy).
 */

//#define HONOURSKIPWTFLAG

/*
 * Data structures:
 *
 * We have two table:
 * "dcache" contains cache entries
 * "dhash" contains the index of the first entry of the hash list.
 * Every cache entries has two fields used to concatenates the entries
 * into lists.
 * There is the list of free entries: "freeindex" is an index into "dcache"
 * of the first free entries, every entry has a "next" field that points
 * to the next free entry.
 * Every entry into "dhash" is a pointer to the first cache entry that is
 * into a hash bucket; every hash bucket is organized like a double linked
 * list using the "next" and "prev" fields of the cache entry.
 *
 * DANGER:
 * - to prevent dead-lock locking of "freemutex" MUST be done always
 *   after a lock of "hashmutex".
 * - the functions that begins with "__????" do not lock/unlock
 *   the mutex.
 */
 
/*+ number of hash buckets +*/
#define MAXHASH     64
/*+ and-mask to obtain a number between 0 and MAXHASH-1 +*/
#define MAXHASHMASK 0x3f

/*+ hash buckets +*/
static int       dhash[MAXHASH];
/*+ mutex to operate on "dhash" +*/
static __fs_mutex_t hashmutex;

/*+ number of cache entries +*/
#define MAXCACHESIZE (8192*4)
/*+ cache entries table +*/
static dcache_t  dcache[MAXCACHESIZE];

/*+ mutex to operate with the "free list" +*/
static __fs_mutex_t freemutex;
/*+ index of first free cache entry +*/
static int       freeindex=0;

static __dev_t blocked_device=makedev(MAJOR_B_NULL,0);

/*++++++++++++++++++++++++++++++++++++++
  
  Hash function.

  int hash_fun
    return a number between 0 and MAXHASH-1.
    
  __dev_t ldisk
    block device.
    
  __blkcnt_t lsector
    block number.
  ++++++++++++++++++++++++++++++++++++++*/

static int hash_fun(__dev_t dev, __blkcnt_t lsector)
{
  static __uint32_t table[8]={3,5,7,11,13,17,19,23};
  return (table[dev&0x07]*(lsector+1))&MAXHASHMASK;
}

/*
 *
 * INITIALIZATION/TERMINATION FUNCTIONS
 *
 */

/*++++++++++++++++++++++++++++++++++++++

  Dump into the system logs the cache status.
  
  ++++++++++++++++++++++++++++++++++++++*/

void dcache_stats(void)
{
  int i,c,d,l,j,r,lc;
  long x,f;
  
  /* acquire all mutex */
  __fs_mutex_lock(&hashmutex);
  __fs_mutex_lock(&freemutex);
  
  /* count free entries */
  c=0;
  i=freeindex;
  while (i!=NIL) {
    c++;
    i=dcache[i].next;
  }
  
  /* dump informations */
  printk(KERN_INFO "max dcache entries: %4i",MAXCACHESIZE);
  printk(KERN_INFO "  free entries:     %4i",c);

  /* count used entries */
  for (i=0,c=0,d=0,l=0,r=0;i<MAXHASH;i++) {
    j=dhash[i];
    while (j!=NIL) {
      c++;
      if (dcache[j].used!=0) l++;
      if (dcache[j].dirty) d++;
      if (!dcache[j].ready) r++;
      j=dcache[j].next;      
    }
  }
  
  /* dump informations */
  printk(KERN_INFO "  cached entries:      %4i",c);
  printk(KERN_INFO "    used entries:      %4i",l);
  printk(KERN_INFO "    dirty entries:     %4i",d);
  printk(KERN_INFO "    not ready entries: %4i",r);

  /* compute hash function quality factor */
  for (i=0,x=0;i<MAXHASH;i++) {
    j=dhash[i];
    lc=0;
    while (j!=NIL) {
      lc++;
      j=dcache[j].next;      
    }
    f=((long)lc*MAXHASH-c);
    x+=f*f/(MAXHASH*2);
  }
  
  /* dump informations */
  printk(KERN_INFO "hash quality:     %6li",x);
 
  /* release all mutex */
  __fs_mutex_unlock(&freemutex);
  __fs_mutex_unlock(&hashmutex);
}

/*++++++++++++++++++++++++++++++++++++++
  
  Module initialization; must be called prior to other functions.

  int dcache_init
    return 0 on success, other values on failure.
  ++++++++++++++++++++++++++++++++++++++*/

int dcache_init(void)
{
  int i;

  /* mutex initialization */
  __fs_mutex_init(&freemutex);
  __fs_mutex_init(&hashmutex);
    
  /* free list and cache table initialization */
  freeindex=0;
  for (i=0;i<MAXCACHESIZE;i++) {
    magic_set(dcache[i].magic,DCACHE_MAGIC);
    dcache[i].next=i+1;
    dcache[i].prev=NIL;
    dcache[i].device=NULLDEV;
    dcache[i].lsector=0;
    dcache[i].used=-1;
    dcache[i].dirty=0;
    dcache[i].ready=0;
    dcache[i].numblocked=0;
    dcache[i].esyncreq=0;
    dcache[i].skipwt=0;
    __fs_sem_init(&dcache[i].sync,0);
    __fs_sem_init(&dcache[i].esync,0);
    __rwlock_init(&dcache[i].rwlock);
  }
  dcache[MAXCACHESIZE-1].next=NIL;

  /* hash buckets initialization */
  for (i=0;i<MAXHASH;i++)
    dhash[i]=NIL;
  
  return 0;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Terminate the cache flushing all dirty blocks and prevent
  other request to be made to the cache module; the only function
  that can be called after this is __dcache_flush().
  
  int dcache_end
    return 0 on success other values on failure
    
  int flag
    if set aquire exclusive use of all cache entries
    before ending
  ++++++++++++++++++++++++++++++++++++++*/

int dcache_end(int flag)
{
  int __dcache_flush(int flag);

  /* if a lock on all cache entries is required... */
  if (flag) {
    int i,k;
    __fs_mutex_lock(&hashmutex);

    /* lock every request of other cache entries*/
    __fs_mutex_lock(&freemutex);
    
    /* scan all hash bucket locking all cache entries */
    for (k=0;k<MAXHASH;k++) {
      i=dhash[k];
      while (i!=NIL) {
	/* so cache entry can not be removed! */
	dcache[i].used++;
	__fs_mutex_unlock(&hashmutex);
	
	/* exclusive lock */
	__rwlock_wrlock(&dcache[i].rwlock);
	
	__fs_mutex_lock(&hashmutex);
	dcache[i].used=1;
	i=dcache[i].next;
      }
    }
    __fs_mutex_unlock(&hashmutex);
  }

  /* this mutex is not released to prevent other modules use of this module */
  __fs_mutex_lock(&hashmutex);
  /* flush the cache */
  return __dcache_flush(flag);
}

/*
 *
 * GET/FREE cache entries FUNCTIONS
 *
 */


/*++++++++++++++++++++++++++++++++++++++
  
  Get a cache entry from the free list.
  
  int get_dcache
    return NIL on failure, an index into "dcache" on success.
  ++++++++++++++++++++++++++++++++++++++*/

static int get_dcache(void)
{
  int ret;

  /* remove the head of a list (if present) */
  
  __fs_mutex_lock(&freemutex);
  ret=freeindex;
  if (ret!=NIL) {
    freeindex=dcache[ret].next;
    _assert(dcache[ret].used==-1);
    dcache[ret].used=0;
    dcache[ret].skipwt=0;
    magic_assert(dcache[ret].magic,DCACHE_MAGIC,
		 "dcache: get_dcache(): cache_entry[%i] overwritten",
		 ret);    
  }
  __fs_mutex_unlock(&freemutex);

  return ret;
}


/*++++++++++++++++++++++++++++++++++++++
  
  Insert a free cache entry into the free list.
  
  int index
    Index of the "dcache" entry to free.
  ++++++++++++++++++++++++++++++++++++++*/

static void free_dcache(int index)
{
  /* insert an element into a list */
  
  _assert(index>=0&&index<MAXCACHESIZE);
  _assert(dcache[index].used==0);

  dcache[index].used=-1;
  
  magic_assert(dcache[index].magic,DCACHE_MAGIC,
	       "dcache: free_dcache(): cache_entry[%i] overwritten",
	       index);    

  __fs_mutex_lock(&freemutex);
  dcache[index].next=freeindex;
  freeindex=index;
  __fs_mutex_unlock(&freemutex);
}

/*
 *
 * HASH BUCKETS MANAGEMENT FUNCTIONS
 *
 */

/*++++++++++++++++++++++++++++++++++++++

  Insert a cache entry into an hash bucket.

  int index
    Entry to insert.
  ++++++++++++++++++++++++++++++++++++++*/

static void __insert_dcache(int index)
{
  int h;

  /* insert an element into a double linked list */
  
  _assert(index>=0&&index<MAXCACHESIZE);
  _assert(dcache[index].used==1);

  magic_assert(dcache[index].magic,DCACHE_MAGIC,
	       "dcache: insert_dcache(): cache_entry[%i] overwritten",
	       index);    
  
  h=hash_fun(dcache[index].device,dcache[index].lsector);

  dcache[index].prev=NIL;
  dcache[index].hash=h;
  
  dcache[index].next=dhash[h];

  if (dhash[h]!=NIL) dcache[dhash[h]].prev=index;

  dhash[h]=index;
}


/*++++++++++++++++++++++++++++++++++++++
  
  Remove a cache entry from an hash bucket.
  
  int index
    Entry to remove.
  ++++++++++++++++++++++++++++++++++++++*/

static void __remove_dcache(int index)
{
  /* remove an element from a double linked list */
  
  _assert(index>=0&&index<MAXCACHESIZE);
  _assert(dcache[index].used==0);
  
  magic_assert(dcache[index].magic,DCACHE_MAGIC,
	       "dcache: remove_dcache(): cache_entry[%i] overwritten",
	       index);
  
  if (dcache[index].prev!=NIL)
    dcache[dcache[index].prev].next=dcache[index].next;
  else {
    dhash[dcache[index].hash]=dcache[index].next;
  }
  if (dcache[index].next!=NIL)
    dcache[dcache[index].next].prev=dcache[index].prev;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Find an entry into an hash bucket to free.

  int __purge_dcache
    NIL on failure, an index of a free cache entry on success
  ++++++++++++++++++++++++++++++++++++++*/

static int __purge_dcache(void)
{
  int i,k;
  int flag;
  int ind;
  int res;
  __time_t t;

  /* TO IMPROVE!!! */
  
  /* algoritmo brutale ed inefficiente per rimuovere un'entry
   * in cache:
   * -- si scorrono sempre (sigh) tutte le entry
   * -- viene trovata la entry non in uso, non sporca, piu' vecchia 
   * -- altrimenti quella non in uso, piu' vecchia
   * -- altrimenti NIL (non rimuove niente)
   */

  /*
   * search an entry to purge
   */
  
  flag=1;
  ind=NIL;
  t=0xffffffff;  
  
  for (k=0;k<MAXHASH;k++) {        
    i=dhash[k];
    while (i!=NIL) {
      if (dcache[i].used==0) {
	if (dcache[i].dirty==0) {
	  flag=0;
	  if (dcache[i].time<t||(ind!=NIL&&dcache[ind].dirty==1)) {
	    ind=i;
	    t=dcache[i].time;
	  }
	} else if (flag) {
	  if (dcache[i].time<t) {
	    ind=i;
	    t=dcache[i].time;
	  }	  
	}
      }      
      i=dcache[i].next;
    }
  }

  /*
   * purging the entry
   */

  /* if an entry is found... */
  if (ind!=NIL) {
    /* if it is dirty... */
    if (dcache[ind].dirty) {

      /* the entry is dirty... try to write to disk!*/

      /* update reference count */
      dcache[ind].used++;
      /* set not ready flags */
      dcache[ind].ready=0;
      /* safety stuff */
      _assert(dcache[ind].used==1);
      /* release mutex */
      __fs_mutex_unlock(&hashmutex);

      /* try to write... */
      /* there is no need to acquire the entry for writing... it is not used */
      res=bdev_write(dcache[ind].device,dcache[ind].lsector,
		     dcache[ind].buffer);

      /* aquire the mutex */
      __fs_mutex_lock(&hashmutex);

      /* if error while writing... */
      if (res!=0) {
	/* restore old fields value */
	dcache[ind].used--;
	dcache[ind].ready=1;
      }

      /* set flag if some thread has been blocked waiting for "ready" field */
      flag=(dcache[ind].numblocked>0);
      
      /* wake up all thread waiting for synchronization */
      while (dcache[ind].numblocked>0) {
	__fs_sem_signal(&dcache[ind].sync);
	dcache[ind].numblocked--;	
      }

      /* if error while writing... return NIL*/
      if (res!=0) return NIL;

      /* to be ensure that no other treads can be blocked on this entry */
      dcache[ind].device=NULLDEV;

      /* if some thread have been waked up... */
      if (flag) {
	/* we must wait that all waked up thread had made the error test... */
	/* request error synchronization */
	dcache[ind].esyncreq=1;
	/* release mutex */
	__fs_mutex_unlock(&hashmutex);

	/* wait for synchronization */
	__fs_sem_wait(&dcache[ind].esync);

	/* reacquire mutex */
	__fs_mutex_lock(&hashmutex);
	/* safety stuff */
	/*_assert(dcache[ind].used==1);*/
      }
      
      /* restore old value */
      dcache[ind].used--;      
      /* safety stuff */
      _assert(dcache[ind].used==0);      
    }
    
    /* remove the entry from the hasb bucket*/
    __remove_dcache(ind);
  }
  
  return ind;
}

/*++++++++++++++++++++++++++++++++++++++

  Called when a cache entry has been modificated (AFTER has been
  modified not before).
  
  WARNING: this function CAN be called only by the task what has
  aquired (for exclusive use) the cache entry.

  void dcache_dirty
    nothing to return
    
  dcache_t *d
    entry that must be marked dirty
  ++++++++++++++++++++++++++++++++++++++*/

#ifdef COPYBACK

void dcache_dirty(dcache_t *d)
{
  /* safety stuff */
  _assert(d->used==1);
  /* we use copy back so only set the flag */
  d->dirty=1;
}

/* used internally when we want flush a cache entry during
 * generic unlock functions
 */

static __inline__ void __flush_cache(dcache_t *d)
{
  /* nothing to do */
  return;
}


#endif

#ifdef WRITETHROUGHT

void dcache_dirty(dcache_t *d)
{
  /* safety stuff */
  _assert(d->used==1);
  /* we only set the flag */
  d->dirty=1;
}

/* used internally when we want flush a cache entry during
 * generic unlock functions
 */
static __inline__ void __flush_cache(dcache_t *d)
{
  int res;

  #ifdef HONOURSKIPWTFLAG
  if (d->skipwt) return;
  #endif
  
  if (d->dirty) {
    /* safety stuff */
    _assert(d->used==1);
    _assert(d-dcache>=0&&d-dcache<MAXCACHESIZE);
    
    /* try to write... */
    /* there is no need to modify/lock the entry (see the warning!) */
    res=bdev_write(d->device,d->lsector,d->buffer);
    
    /* if write OK... */
    if (res==0) d->dirty=0;
  }  
}

#endif

/*++++++++++++++++++++++++++++++++++++++

  Find a free cache entry to use.

  int __catch_dcache
    NIL on failure, a index of a free cache entry on success.
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ int __catch_dcache(void)
{
  int ind;

  ind=get_dcache();
  if (ind==NIL) ind=__purge_dcache();
  return ind;
}

/*
 *
 * FLUSHING FUNCTIONS
 *
 */

/*++++++++++++++++++++++++++++++++++++++
  
  Flush all cache entries to disk; this function does not acquire the
  "hashmutex" mutex so it can be called only during system shut down (with
  no runnig thread) or after a call to dcache_end().
  The function dcache_end() call this function before ending.
  
  int __dcache_flush
    return 0 on success, other values on failure
    
  inf flag
    if set mean that we do have exclusive access to cache memory; if
    we have exclusive access the "used" field must be 1 else must be 0.
  ++++++++++++++++++++++++++++++++++++++*/

int __dcache_flush(int flag)
{
  int i,k,ret,res;
  
  ret=0;
  /* for every hash buckets... */
  for (k=0;k<MAXHASH;k++) {
    i=dhash[k];
    /* scan all entries... */
    while (i!=NIL) {    
      _assert(dcache[i].used==(flag?1:0));
      /* for every dirty entry... */
      if (dcache[i].dirty) {
	res=bdev_write(dcache[i].device,dcache[i].lsector,dcache[i].buffer);
	if (res==0)
	  dcache[i].dirty=0;
	else 
	  ret=-1;	  
      }
      /* go to next entry */
      i=dcache[i].next;
    }
  }
  
  return ret;
}


/*++++++++++++++++++++++++++++++++++++++
  
  Purge a device from the cache; remove every reference to the device
  from the cache.
  
  int dcache_purgedevice
    0 on success, -1 on error
    
  __dev_t dev
    device to purge
    
  int cont
    if setted continue on disk write error
  ++++++++++++++++++++++++++++++++++++++*/

int dcache_purgedevice(__dev_t dev, int cont)
{
  int retvalue=0;
  int i,k,inext;
  int res;

  printka("dcache_purgedevice() START for device(%08lx)",(long)dev);
  
  __fs_mutex_lock(&hashmutex);

  /* prevent other thread to cache this device*/
  blocked_device=dev;
  
  for (k=0;k<MAXHASH;k++) {
    i=dhash[k];
    /* scan all entries... */
    while (i!=NIL) {

      /* remember next entry */
      inext=dcache[i].next;
      
      if (dcache[i].device==dev) {
	/* an entry to purge is found... */
	//printka("  found %li",dcache[i].lsector);
	
	_assert(dcache[i].used!=-1);
	while (dcache[i].used!=0) {
	  /* someone is using this entry... we must wait! */
	  
	  dcache[i].used++;
	  __fs_mutex_unlock(&hashmutex);
	  
	  __rwlock_wrlock(&dcache[i].rwlock);
	  __rwlock_wrunlock(&dcache[i].rwlock);
	  
	  __fs_mutex_lock(&hashmutex);
	  dcache[i].used--;
	  
	  /* we must loop because, in general, we do not know how */
	  /* the __rwlock queues are managed; the loop must quit because */
	  /* we prevent other threads to be inserted into this queues */

	  /* we must update 'inext' */
	  inext=dcache[i].next;
	}

	if (dcache[i].dirty!=0) {
	  /* the cache entry is dirty... we must write to disk */
	  printka("  dirty %li",dcache[i].lsector);

	  dcache[i].used++;
	  __fs_mutex_unlock(&hashmutex);

	  res=bdev_write(dcache[i].device,dcache[i].lsector,dcache[i].buffer);
	  	  
	  __fs_mutex_lock(&hashmutex);
	  dcache[i].used--;

	  /* we must update 'inext' */
	  inext=dcache[i].next;

	  if (res!=0) {
	    /* an error! */
	    retvalue=-1;
	    if (!cont) {
	      /* ...we can not continue! */
	      blocked_device=makedev(MAJOR_B_NULL,0);
	      __fs_mutex_unlock(&hashmutex);
	      return -1;
	    }
	  }
	  
	}

	/* now we can purge the entry */
	__remove_dcache(i);
	
      }
      
      /* go to next entry */
      i=inext;
    }
  }

  blocked_device=makedev(MAJOR_B_NULL,0);  
  __fs_mutex_unlock(&hashmutex);

  printka("dcache_purgedevice() END");

  return retvalue;
}

/*++++++++++++++++++++++++++++++++++++++
  
  Flush all the cache to disk; it can be called always but it does not
  assure that there are no more dirty cache entries when it return.
  
  int dcache_flush
    return 0 on success, other value on failure.
  ++++++++++++++++++++++++++++++++++++++*/

int dcache_flush(void)
{
  int i,k,ret,res;

  __fs_mutex_lock(&hashmutex);
  ret=0;
  for (k=0;k<MAXHASH;k++) {
    i=dhash[k];
    /* for every dirty cache entries...*/
    while (i!=NIL) {
      
      magic_assert(dcache[i].magic,DCACHE_MAGIC,
		   "dcache: dcache_flush(): cache_entry[%i] overwritten",
		   i);    
      
      if (dcache[i].dirty) {
	/* to prevent purging the cache entry */
	dcache[i].used++;
	__fs_mutex_unlock(&hashmutex);
	
	/* aquire a "read lock" (we do not modify cache) to write to disk */
	__rwlock_rdlock(&dcache[i].rwlock);
	/* write to disk */
	res=bdev_write(dcache[i].device,dcache[i].lsector,dcache[i].buffer);
        /* release "read lock" */	
	__rwlock_rdunlock(&dcache[i].rwlock);
	
	__fs_mutex_lock(&hashmutex);
	dcache[i].used--;
	if (res==0)
	  dcache[i].dirty=0;
	else 
	  ret=-1;	  
      }
      i=dcache[i].next;
    }
  }
  __fs_mutex_unlock(&hashmutex);
  
  return ret;
}

/*
 *
 * LOCKING/UNLOCKING FUNCTIONS
 *
 *
 * the external modules are responsable for nested lock; they can cause
 * a dead lock of some threads if they lock blocks nesting the locks without
 * a "locking protocol"
 */


/*++++++++++++++++++++++++++++++++++++++
  
  Lock (for read or write) a cache entry.

  static __inline__ dcacdhe_t *dcache_genlock
    return a pointer to the locked cache entry or NULL in case of error.
    
  __dev_t device
    device where there is the block to lock.
    
  __blkcnt_t lsector
    block to lock.
    
  void (*lockfunc)(__rwlock_t*)
    function used to lock; must be __rwlock_rdlock (for read) or
    __rwlock_wrlock (for write).    
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ dcache_t *dcache_genlock(__dev_t device,
					    __blkcnt_t lsector,
					    void (*lockfunc)(__rwlock_t*))
{
  int h,i;
  int res;

  /* compute hash value */
  h=hash_fun(device,lsector);

  /* acquire the mutex */
  __fs_mutex_lock(&hashmutex);

  /*
   * STAGE 1: search into hash list
   */

  /* get first entry */
  i=dhash[h];
  /* while there are other cache entries... */
  while (i!=NIL)  {
    if (dcache[i].device==device&&dcache[i].lsector==lsector) {      
      /* found... */
      /* safety stuff */
      _assert(dcache[i].used>=0);
      /* update "used" count */
      dcache[i].used++;
      /* but is ready? */
      if (!dcache[i].ready) {	
	/* not ready! */
	/* someone has already done this request but the cache is not ready */
	dcache[i].numblocked++;
	/* release the mutex */
	__fs_mutex_unlock(&hashmutex);	
	/* wait for syncronization */
	__fs_sem_wait(&dcache[i].sync);
	/* sync is arrived but ready? */
	if (!dcache[i].ready) {	  
	  /* not ready! an error has occured */
	  /* reaquire mutex */
	  __fs_mutex_lock(&hashmutex);
	  /* safety stuff */
	  _assert(dcache[i].used>0);
	  /* update "used" count */
	  dcache[i].used--;
	  /* if request for error syncronization... */
	  if (dcache[i].esyncreq) {
	    /* request sync */
	    /* if it is the last thread (other than the waiting tread)... */
	    if (dcache[i].used==1) 
	      /* signal error syncronization */
	      __fs_sem_signal(&dcache[i].esync);
	  } else {
	    /* if there are not other threads... */
	    if (dcache[i].used==0) {
	      /* free this entry */
	      __remove_dcache(i);
	      free_dcache(i);
	    }
	  }
	  /* release mutex */
	  __fs_mutex_unlock(&hashmutex);	 
	  /* nothing to return */
	  return NULL;
	}
	/* aquire the "rwlock" */
	(*lockfunc)(&dcache[i].rwlock);
	/* return the cache entry */
	return dcache+i;	
      }      
      /* ready! */
      /* release the mutex */
      __fs_mutex_unlock(&hashmutex);
      /* aquire the "rwlock" */
      (*lockfunc)(&dcache[i].rwlock);
      /* safety stuff */
      magic_assert(dcache[i].magic,DCACHE_MAGIC,
		   "dcache: dcache_genlock() 1: cache_entry[%i] overwritten",
		   i);          
      /* return the cache entry */ 
      return dcache+i;
    }
    /* get next entry */
    i=dcache[i].next;
  }

  /*
   * STAGE 2: create a new dcache entry
   */

  /* PS: if __catch_dcache() calls __purge_dcache() there is a great overhead
   * => to improve efficency a server that mantains x% of cache free is
   * required
   */
  i=__catch_dcache();
  if (i==NIL) {
    /* there is not space for new dcache entry...*/
    __fs_mutex_unlock(&hashmutex);
    return NULL;
  }
  
  /* found a free dcache entry... fill the entry */
  dcache[i].used=1;
  dcache[i].dirty=0;
  dcache[i].device=device;
  dcache[i].lsector=lsector;
  dcache[i].ready=0;
  dcache[i].numblocked=0;
  dcache[i].esyncreq=0;
  
  /* insert the entry intro the cache data structure */
  __insert_dcache(i);
  
  /* release the mutex */
  __fs_mutex_unlock(&hashmutex);

  /* the read is done after the hashmutex has been unlocked to allow */
  /* thread cuncurrency */
  res=bdev_read(device,lsector,dcache[i].buffer);

  /* acquire the mutex */
  __fs_mutex_lock(&hashmutex);

  /* safety stuff */
  _assert(dcache[i].used>0);
  _assert(dcache[i].ready==0);
  
  /* if the read is OK... */
  if (res==0) {
    /* cache ready */
    dcache[i].ready=1;
    /* aquire the rwlock */
    (*lockfunc)(&dcache[i].rwlock);  
  }
  
  /* wake up all thread waiting for synchronization */
  while (dcache[i].numblocked>0) {
    __fs_sem_signal(&dcache[i].sync);
    dcache[i].numblocked--;
  }

  /* if the read is not OK... */
  if (res!=0) {
    /* to prevent other thread to take this cache entry */
    dcache[i].device=NULLDEV;
    /* update cache "thread" count */
    dcache[i].used--;
    /* if there are not other threads... */
    if (dcache[i].used==0) {
      /* free this entry */
      __remove_dcache(i);
      free_dcache(i);
    }
    /* release mutex */
    __fs_mutex_unlock(&hashmutex);
    /* nothing to return */
    return NULL;    
  }
  
  magic_assert(dcache[i].magic,DCACHE_MAGIC,
	       "dcache: dcache_genlock() 2: cache_entry[%i] overwritten",
	       i);          
  
  __fs_mutex_unlock(&hashmutex);
  return dcache+i;
}

/*++++++++++++++++++++++++++++++++++++++

  Unlock a previously locked cache entry.  

  dcache_t *ptr
    cache enry to unlock.
    
  void (*unlockfunc)(__rwlock_t*)
    unlocking function; must be __rwlock_rdunlock (for read) or
    __rwlock_wrunlock (for write).    
  ++++++++++++++++++++++++++++++++++++++*/

static __inline__ void dcache_genunlock(dcache_t *ptr,
					void (*unlockfunc)(__rwlock_t*))
{
  /* safety stuff */
  _assert(ptr>=dcache&&ptr<dcache+MAXCACHESIZE);
  /* try to flush the cache if is dirty */
  /* (this function depends of write thought/copy back) */
  __flush_cache(ptr);
  /* acquire the mutex */
  __fs_mutex_lock(&hashmutex);
  /* safety stuff */
  _assert(ptr->used>0);
  /* update some fields */
  ptr->used--;
  ptr->time=gettimek();
  /* DANGER!!! I hope this work */
  /* unlock the cache entry */
  (*unlockfunc)(&ptr->rwlock);
  /* release the mutex */
  __fs_mutex_unlock(&hashmutex);
}

/*++++++++++++++++++++++++++++++++++++++
  
  Lock a cache entry used for READING.
  
  dcacdhe_t *dcache_lock
    return a locked cache entry or NULL in case of error.
    
  __dev_t device
    device where there is the block to lock.
    
  __blkcnt_t lsector
    block to lock.
  ++++++++++++++++++++++++++++++++++++++*/

dcache_t *dcache_lock(__dev_t device, __blkcnt_t lsector)
{
  printk8("LOCK read request for (%04x,%li)",device,(long)lsector);
  if (device==blocked_device) return NULL;
  return dcache_genlock(device,lsector,__rwlock_rdlock);
}

/*++++++++++++++++++++++++++++++++++++++

  Unlock a cache entry used for READING.

  dcache_t *ptr
    pointer to the cache entry to unlock.
  ++++++++++++++++++++++++++++++++++++++*/

void dcache_unlock(dcache_t *ptr)     
{
  printk8("LOCK read release for (%04x,%li)",ptr->device,(long)ptr->lsector);
  return dcache_genunlock(ptr,__rwlock_rdunlock);
}

/*++++++++++++++++++++++++++++++++++++++
  
  Lock a cache entry used for WRITING (ie. acquire for exclusive use).
  If we write into cache we MUST call dcache_dirty().
  
  dcacdhe_t *dcache_acquire

  __dev_t device
    device where there is the block to lock.

  __blkcnt_t lsector
    block to lock.
  ++++++++++++++++++++++++++++++++++++++*/

dcache_t *dcache_acquire(__dev_t device, __blkcnt_t lsector)
{
  printk8("LOCK write request for (%04x,%li)",device,(long)lsector);
  if (device==blocked_device) return NULL;
  return dcache_genlock(device,lsector,__rwlock_wrlock);
}

/*++++++++++++++++++++++++++++++++++++++
  
  Unlock a cache entry used for WRITING (ie. acquired for exclusive use).

  dcache_t *ptr
    pointer to the cache entry to unlock.
  ++++++++++++++++++++++++++++++++++++++*/

void dcache_release(dcache_t *ptr)
{
  printk8("LOCK write released for (%04x,%li)",ptr->device,(long)ptr->lsector);
  return dcache_genunlock(ptr,__rwlock_wrunlock);
}

