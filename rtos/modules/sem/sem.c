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
 CVS :        $Id: sem.c,v 1.3 2006/06/12 12:29:34 tullio Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2006/06/12 12:29:34 $
 ------------

 This file contains the Hartik 3.3.1 Semaphore functions

 Author:      Giuseppe Lipari

 Semaphores:
 this is the generalized version of the primitives signal & wait
 In this case, the user can specify the number to inc/dec the
 semaphore's counter. It is useful in the buffer management
 (see port section)

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
#include <sem/sem/sem.h>
#include <kernel/config.h>
#include <ll/ll.h>
#include <ll/string.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <limits.h>
#include <fcntl.h>

#include <tracer.h>

/* Semaphores descriptor tables */
static struct sem_des {
    char *name;     /* a name, for named semaphores */
    int index;      /* an index for sem_open, containing the sem number */
    int count;      /* the semaphore counter */
    IQUEUE blocked; /* the blocked processes queue */
    int next;       /* the semaphore queue */
    BYTE used;      /* 1 if the semaphore is used */
} sem_table[SEM_NSEMS_MAX];


/* this -IS- an extension to the proc_table!!! */
static struct {
    int decsem;   /* the value required in sem_xwait */
    int sem;      /* the semaphore on whitch the process is blocked */
} sp_table[MAX_PROC];

static int free_sem;         /* Queue of free sem                    */



/*----------------------------------------------------------------------*/
/* Cancellation test for semaphores                                     */
/*----------------------------------------------------------------------*/

/* this is the test that is done when a task is being killed
   and it is waiting on a sigwait */
static int semwait_cancellation_point(PID i, void *arg)
{
    LEVEL l;

    if (proc_table[i].status == WAIT_SEM) {
      /* the task that have to be killed is waiting on a sig_wait.
         we reset the data structures set in sig_wait and then when the
         task will return on the sig_wait it will fall into a
         task_testcancel */

      /* extract the process from the semaphore queue... */
      iq_extract(i,&sem_table[ sp_table[i].sem ].blocked);

      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------*/
/* Init the semaphoric structures                                       */
/*----------------------------------------------------------------------*/
void SEM_register_module(void)
{
    int i;

    for (i = 0; i < SEM_NSEMS_MAX; i++) {
        sem_table[i].name = NULL;
	sem_table[i].index = i;
        sem_table[i].count = 0;
        iq_init(&sem_table[i].blocked, &freedesc, 0);
	sem_table[i].next = i+1;
	sem_table[i].used = 0;
    }
    sem_table[SEM_NSEMS_MAX-1].next = NIL;
    free_sem = 0;

    register_cancellation_point(semwait_cancellation_point, NULL);
}

/*----------------------------------------------------------------------*/
/* Allocate a semaphoric descriptor and sets the counter to n           */
/*----------------------------------------------------------------------*/

// the pshared parameter is NRQ for PSE52
int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    SYS_FLAGS f;
   
    if (value > SEM_VALUE_MAX)
      return EINVAL;

    f = kern_fsave();
    *sem = free_sem;
    if (*sem != NIL) {
	free_sem = sem_table[*sem].next;
	sem_table[*sem].name = NULL;
	sem_table[*sem].count = value;
	iq_init(&sem_table[*sem].blocked, &freedesc, 0);
	sem_table[*sem].used = 1;
    }
    else {
	errno = ENOSPC;
	kern_frestore(f);
	return -1;
    }
    kern_frestore(f);
    return 0;
}

/*----------------------------------------------------------------------*/
/* Frees a semaphores descriptor                                        */
/*----------------------------------------------------------------------*/
int sem_destroy(sem_t *sem)
{
    SYS_FLAGS f;
 
    f = kern_fsave();

    if (*sem < 0 || *sem >= SEM_NSEMS_MAX || !sem_table[*sem].used) {
      errno = EINVAL;
      kern_frestore(f);
      return -1;
    }

    if (sem_table[*sem].blocked.first != NIL) {
      errno = EBUSY;
      kern_frestore(f);
      return -1;
    }

    sem_table[*sem].used = 0;
    sem_table[*sem].next = free_sem;
    free_sem = *sem;

    kern_frestore(f);
    return 0;
}

/*----------------------------------------------------------------------*/
/* Allocate a named semaphore                                           */
/*----------------------------------------------------------------------*/

// the pshared parameter is NRQ for PSE52
sem_t *sem_open(const char *name, int oflag, ...)
{
    int i, j;
    int found = 0;
    mode_t m;
    sem_t sem;
    SYS_FLAGS f;

    f = kern_fsave();

    for (i = 0; i < SEM_NSEMS_MAX; i++)
      if (sem_table[i].used) {
        if (strcmp(name, sem_table[i].name) == 0) {
          found = 1;
          break;
        }
      }
    if (found) {
      if (oflag == (O_CREAT | O_EXCL)) {
          errno = EEXIST;
          kern_frestore(f);
          return SEM_FAILED;
      } else {
          kern_frestore(f);
          return &sem_table[i].index;
      }
    } else {
      if (!(oflag & O_CREAT)) {
          errno = ENOENT;
          kern_frestore(f);
          return SEM_FAILED;
      } else {
          va_list l;

          va_start(l, oflag);
            m = va_arg(l,mode_t);
            j = va_arg(l, int);
          va_end(l);

          if (j > SEM_VALUE_MAX) {
            errno = EINVAL;
            kern_frestore(f);
            return SEM_FAILED;
          }

          sem = free_sem;
          if (sem != -1) {
            free_sem = sem_table[sem].next;
            sem_table[sem].name = kern_alloc(strlen((char *)name)+1);
            strcpy(sem_table[sem].name, (char *)name);
            sem_table[sem].count = j;
            iq_init(&sem_table[sem].blocked, &freedesc, 0);
            sem_table[sem].used = 1;
            kern_frestore(f);
            return &sem_table[sem].index;
          }
          else {
    	    errno = ENOSPC;
    	    kern_frestore(f);
    	    return SEM_FAILED;
          }
      }
    }
}

/*----------------------------------------------------------------------*/
/* Frees a named semaphore                                              */
/*----------------------------------------------------------------------*/
int sem_close(sem_t *sem)
{
    SYS_FLAGS f;

    f = kern_fsave();

    if (*sem < 0 || *sem >= SEM_NSEMS_MAX || !sem_table[*sem].used) {
      errno = EINVAL;
      kern_frestore(f);
      return -1;
    }

/*  why not???
    if (sem_table[*sem].q_first != -1) {
      errno = EBUSY;
      kern_sti();
      return -1;
    } */

    kern_free(sem_table[*sem].name,strlen(sem_table[*sem].name)+1);
    sem_table[*sem].used = 0;
    sem_table[*sem].next = free_sem;
    free_sem = *sem;

    kern_frestore(f);
    return 0;
}

/*----------------------------------------------------------------------*/
/* Unlink a named semaphore                                             */
/*----------------------------------------------------------------------*/
int sem_unlink(const char *name)
{
    int i;
    int found = 0;
    SYS_FLAGS f;

    f = kern_fsave();

    for (i = 0; i < SEM_NSEMS_MAX; i++)
      if (sem_table[i].used) {
        if (strcmp(name, sem_table[i].name) == 0) {
          found = 1;
        }
      }

    if (found) {
      kern_free(sem_table[i].name,strlen((char *)name)+1);
      sem_table[i].used = 0;
      sem_table[i].next = free_sem;
      free_sem = i;
      kern_frestore(f);
      return 0;
    } else {
      errno = ENOENT;
      kern_frestore(f);
      return SEM_FAILED;
    }
}

/*----------------------------------------------------------------------*/
/* Generic wait. If it is possible, decrements the sem counter of n,    */
/* else blocks the task.                                                */
/*----------------------------------------------------------------------*/
int sem_wait(sem_t *s)
{
    struct sem_des *s1; /* It speeds up access */

    if (*s < 0 || *s >= SEM_NSEMS_MAX || !sem_table[*s].used) {
      errno = EINVAL;
      return -1;
    }

    task_testcancel();

    proc_table[exec_shadow].context = kern_context_save();

    s1 = &sem_table[*s];

    if (s1->blocked.first != NIL || s1->count == 0)  {
            /* We must block exec task   */
            LEVEL l;            /* for readableness only */

	    /* tracer stuff */
	    TRACER_LOGEVENT(FTrace_EVT_set_mutex_wait,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);

	    kern_epilogue_macro();

            l = proc_table[exec_shadow].task_level;
            level_table[l]->public_block(l,exec_shadow);

            /* we insert the task in the semaphore queue */
	    proc_table[exec_shadow].status = WAIT_SEM;

            /* Prepare sem_table des... */
            sp_table[exec_shadow].decsem = 1;
	    sp_table[exec_shadow].sem = *s;

            /* ...and put it in sem queue */
	    iq_insertlast(exec_shadow,&s1->blocked);

            /* and finally we reschedule */
            exec = exec_shadow = -1;
            scheduler();
            kern_context_load(proc_table[exec_shadow].context);

            /* sem_wait is a cancellation point... */
            task_testcancel();
    }
    else {
	    s1->count--;
	    /* tracer stuff */
	    TRACER_LOGEVENT(FTrace_EVT_set_mutex_wait,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
            kern_context_load(proc_table[exec_shadow].context);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
/* Non-blocking wait                                                    */
/*----------------------------------------------------------------------*/
int sem_trywait(sem_t *s)
{
    struct sem_des *s1; /* It speeds up access */
    SYS_FLAGS f;

    if (*s < 0 || *s >= SEM_NSEMS_MAX || !sem_table[*s].used) {
      errno = EINVAL;
      return -1;
    }

    f = kern_fsave();

    s1 = &sem_table[*s];

    if (s1->blocked.first != NIL || s1->count == 0)  {
      errno = EAGAIN;
      kern_frestore(f);
      return -1;
    }
    else
      s1->count--;
    
    kern_frestore(f);
    return 0;
}


/*----------------------------------------------------------------------*/
/* Generic wait. If it is possible, decrements the sem counter of n,    */
/* else blocks the task.                                                */
/*----------------------------------------------------------------------*/
int sem_xwait(sem_t *s, int n, int wait)
{
    struct sem_des *s1; /* It speeds up access */

    if (*s < 0 || *s >= SEM_NSEMS_MAX || !sem_table[*s].used) {
      errno = EINVAL;
      return -1;
    }

    /* We do not need to save context if we are sure we shall not block! */
    if (wait == NON_BLOCK)
      kern_cli();
    else
      proc_table[exec_shadow].context = kern_context_save();

    s1 = &sem_table[*s];

    /* The non blocking wait is really simple! */
    /* We do not suspend or schedule anything  */    
    if (wait == NON_BLOCK) {
      if (s1->blocked.first != NIL || s1->count < n)  {
        errno = EAGAIN;
        kern_sti();
        return -1;
      }
      else
        s1->count -= n;

      kern_sti();
      return 0;
    }
    /* The blocking wait is more complex... */
    else {
        /* the blocking wait is a cancellation point */
        task_testcancel();

        if (s1->blocked.first != NIL || s1->count < n)  {
                    /* We must block exec task   */
                    LEVEL l;            /* for readableness only */

		    /* tracer */
		    TRACER_LOGEVENT(FTrace_EVT_set_mutex_wait,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
		    
		    kern_epilogue_macro();
                  
                    l = proc_table[exec_shadow].task_level;
                    level_table[l]->public_block(l,exec_shadow);
        
                    /* we insert the task in the semaphore queue */
        	    proc_table[exec_shadow].status = WAIT_SEM;
        
                    /* Prepare sem_table des... */
                    sp_table[exec_shadow].decsem = n;
        	    sp_table[exec_shadow].sem = *s;
        
                    /* ...and put it in sem queue */
        	    iq_insertlast(exec_shadow,&s1->blocked);
        
                    /* and finally we reschedule */
                    exec = exec_shadow = -1;
                    scheduler();
                    kern_context_load(proc_table[exec_shadow].context);
        
                    /* sem_wait is a cancellation point... */
                    task_testcancel();
            }
            else {
        	    s1->count -= n;
		    /* tracer */
		    TRACER_LOGEVENT(FTrace_EVT_set_mutex_wait,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
                    kern_context_load(proc_table[exec_shadow].context);
            }
    }

    return 0;
}

/*----------------------------------------------------------------------*/
/* Generic signal. It increments the sem counter of 1, and wakes one    */
/* of the tasks that are blocked on the semaphore, if it is possible.The*/
/* semaphoric queue is a FIFO queue, in order to eliminate deadlocks    */
/*----------------------------------------------------------------------*/
int sem_post(sem_t *s)
{
    struct sem_des *s1;        /* it speeds up access          */
    int p;                     /* idem                         */
    LEVEL l;

    if (*s < 0 || *s >= SEM_NSEMS_MAX || !sem_table[*s].used) {
      errno = EINVAL;
      return -1;
    }

    // ugly patch to call a sem_post!!!
    if (ll_ActiveInt()) {
      SYS_FLAGS f;
      f = kern_fsave();
      s1 = &sem_table[*s];
      s1->count ++;              /* inc sem count                */

      p = s1->blocked.first;
      if (p != NIL && sp_table[p].decsem <= s1->count) {
	/* Dec sem count */
	s1->count -= sp_table[p].decsem;
	
	/* Get task from blocked queue */
	iq_extract(p,&s1->blocked);
	
	l = proc_table[p].task_level;
	level_table[l]->public_unblock(l,p);
	/* only a task can be awaken */
	/* Preempt if necessary */
	event_need_reschedule();
      }
      
      /* tracer */
      TRACER_LOGEVENT(FTrace_EVT_set_mutex_post,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
      kern_frestore(f);
    }
    else {
      proc_table[exec].context = kern_context_save();
      
      s1 = &sem_table[*s];
      s1->count ++;              /* inc sem count                */
      
      p = s1->blocked.first;
      if (p != NIL && sp_table[p].decsem <= s1->count) {
	/* Dec sem count */
	s1->count -= sp_table[p].decsem;
	
	/* Get task from blocked queue */
	iq_extract(p,&s1->blocked);
	
	l = proc_table[p].task_level;
	level_table[l]->public_unblock(l,p);
	/* only a task can be awaken */
	/* Preempt if necessary */
	scheduler();
      }
      
      /* tracer */
      TRACER_LOGEVENT(FTrace_EVT_set_mutex_post,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
      
      kern_context_load(proc_table[exec_shadow].context);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
/* Generic signal. It increments the sem counter of n, and wakes all the*/
/* tasks that are blocked on the semaphore, if it is possible. The      */
/* semaphoric queue is a FIFO queue, in order to eliminate deadlocks    */
/*----------------------------------------------------------------------*/
int sem_xpost(sem_t *s, int n)
{
    struct sem_des *s1;        /* it speeds up access          */
    int p;                     /* idem                         */
    int fl = 0;                /* a flag                       */
    LEVEL l;

    if (*s < 0 || *s >= SEM_NSEMS_MAX || !sem_table[*s].used) {
      errno = EINVAL;
      return -1;
    }

    // ugly patch to call a sem_post!!!
    if (ll_ActiveInt()) {      
      SYS_FLAGS f;
      f = kern_fsave();
      s1 = &sem_table[*s];
      s1->count += n;                     /* inc sem count                */
      
      p = s1->blocked.first;
      while (p != NIL && sp_table[p].decsem <= s1->count) {
	/* Dec sem count */
	s1->count -= sp_table[p].decsem;
	
	/* Get task from blocked queue */
	iq_extract(p,&s1->blocked);
	
	l = proc_table[p].task_level;
	level_table[l]->public_unblock(l,p);
	
	/* Next task to wake            */
	p = s1->blocked.first;
	
	fl = 1;
      }
      
      /* tracer */
      TRACER_LOGEVENT(FTrace_EVT_set_mutex_post,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
      
      /* Preempt if necessary */
      if (fl) event_need_reschedule();
      kern_frestore(f);
    }
    else {
      proc_table[exec].context = kern_context_save();
      
      s1 = &sem_table[*s];
      s1->count += n;                     /* inc sem count                */
      
      p = s1->blocked.first;
      while (p != NIL && sp_table[p].decsem <= s1->count) {
	/* Dec sem count */
	s1->count -= sp_table[p].decsem;
	
	/* Get task from blocked queue */
	iq_extract(p,&s1->blocked);
	
	l = proc_table[p].task_level;
	level_table[l]->public_unblock(l,p);
	
	/* Next task to wake            */
	p = s1->blocked.first;
	
	fl = 1;
      }
      
      /* tracer */
      TRACER_LOGEVENT(FTrace_EVT_set_mutex_post,(unsigned short int)proc_table[exec_shadow].context,(unsigned int)*s);
      
      /* Preempt if necessary */
      if (fl) scheduler();
      
      kern_context_load(proc_table[exec_shadow].context);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
/* Getvalue returns the value of the semaphore (>=0). If someone is     */
/* blocked on the semaphore, return the number of process blocked (<0)  */
/*----------------------------------------------------------------------*/
int sem_getvalue(sem_t *sem, int *sval)
{
    PID p;
    SYS_FLAGS f;

    if (*sem < 0 || *sem >= SEM_NSEMS_MAX || !sem_table[*sem].used) {
      errno = EINVAL;
      return -1;
    }

    f = kern_fsave();

    if (iq_isempty(&sem_table[*sem].blocked))
      /* the sem is free */
      *sval = sem_table[*sem].count;
    else {
      /* the sem is busy */
      *sval = 0;
      p = iq_query_first(&sem_table[*sem].blocked);
      do {
        (*sval)--;
        p = iq_query_next(p, &sem_table[*sem].blocked);
      } while (p != NIL);
    }

    kern_frestore(f);
    return 0;
}


/*----------------------------------------------------------------------*/
/* this function returns 1 if the task is blocked on a semaphore        */
/*----------------------------------------------------------------------*/
int isBlocked(PID i)
{
    if (proc_table[i].status == WAIT_SEM) return 1;
    else return 0;
}

