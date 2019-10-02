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
 CVS :        $Id: keys.c,v 1.2 2003/11/05 15:05:11 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/11/05 15:05:11 $
 ------------

 task_specific data

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

#include <stdarg.h>
#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>


/*+ This data contains key definition... +*/
static struct key_data_struct {
  void (*destr)(void *);
  int next;
  int used;
} key_data[PTHREAD_KEYS_MAX];

/*+ This is the key free queue... used for alloc new keys +*/
static int freekey;

void call_task_specific_data_destructors()
{
  int keycount;    /* number of times the key destructors are called */
  int key_exit;    /* flag used to exit from the destruction process */
  int key_i;       /* a counter */

  for (keycount = 0;
       keycount < PTHREAD_DESTRUCTOR_ITERATIONS;
       keycount++) {

      key_exit = 0;
      for (key_i=1; key_i<PTHREAD_KEYS_MAX; key_i++) {
        if (key_data[key_i].used      &&
            key_data[key_i].destr     &&
            proc_table[exec_shadow].keys[key_i] ) {
          kern_sti();
          key_data[key_i].destr(proc_table[exec_shadow].keys[key_i]);
          kern_cli();
          key_exit |= (int)proc_table[exec_shadow].keys[key_i];
        }
      }
      if (!key_exit) break;
  }
}

void task_specific_data_init()
{
  int j;

  for (j=1; j<PTHREAD_KEYS_MAX; j++) {
     key_data[j].destr = NULL;
     key_data[j].next  = j+1;
     key_data[j].used  = 0;
  }
  key_data[PTHREAD_KEYS_MAX-1].next  = -1;

  /* alloc the free key 0 for cleanup handlers */
  freekey = 1;
  key_data[0].used = 1;
  key_data[0].destr = NULL;
  key_data[0].next  = -1;
}

/*---------------------------------------------------------------------*/
/* Task specific data Handling                                         */
/*---------------------------------------------------------------------*/

/* look at the POSIX standard's Section 17 for more details... */

int task_key_create(task_key_t *key, void (*d)(void *))
{
  PID p;
  SYS_FLAGS f;

  f = kern_fsave();

  if (freekey == -1) {
    kern_frestore(f);
    return (EAGAIN);
  }

  /* alloc a free key */
  key_data[freekey].used = 1;
  *key = freekey;
  freekey = key_data[freekey].next;

  /* fill the descriptor */
  key_data[*key].destr = d;

  /* fill the task descriptor keys */
  for (p=0; p<MAX_PROC; p++)
    proc_table[p].keys[*key] = NULL;

  kern_frestore(f);
  return 0;
}

/*---------------------------------------------------------------------*/
/* 17.1.2 Thread Specific Data Management                              */
/*---------------------------------------------------------------------*/

void *task_getspecific(task_key_t key)
{
  void *ret;
  SYS_FLAGS f;

  f = kern_fsave();
  ret = proc_table[exec_shadow].keys[key];
  kern_frestore(f);

  return ret;
}

int task_setspecific(task_key_t key, const void *value)
{
  SYS_FLAGS f;

  f = kern_fsave();
  if (key < 0 || key >= PTHREAD_KEYS_MAX || !key_data[key].used) {
    kern_frestore(f);
    return (EINVAL);
  }
  proc_table[exec_shadow].keys[key] = (void *)value;
  kern_frestore(f);
  return 0;
}

/*---------------------------------------------------------------------*/
/* 17.1.3 Thread-Specific Data key deletion                            */
/*---------------------------------------------------------------------*/

int task_key_delete(task_key_t key)
{
  SYS_FLAGS f; /* task_key dete can be called in a destructor!!! */

  f = kern_fsave();
  if (key < 0 || key >= PTHREAD_KEYS_MAX || !key_data[key].used) {
    kern_frestore(f);
    return (EINVAL);
  }

  key_data[key].next = freekey;
  key_data[key].used = 0;

  freekey = key;
  kern_frestore(f);
  return 0;
}
