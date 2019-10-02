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
 CVS :        $Id: mqueue.c,v 1.4 2003/11/05 15:05:11 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2003/11/05 15:05:11 $
 ------------

 POSIX message queues

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

#include <mqueue.h>
#include <ll/string.h>
#include <kernel/types.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/types.h>

/* some flags... */
#define MQ_USED                 1
#define MQ_NONBLOCK             2
#define MQ_NOTIFICATION_PRESENT 4

static int mq_once = 1;

struct mq_elem {
  unsigned int mq_prio;   /* the priority of a message */
  ssize_t msglen;         /* the length of a message   */
  int next;               /* the priority queue        */
};

/* Semaphores descriptor tables */
static struct mq_des {
    char *name;              /* a name */
    int  flags;              /* flags... */

    long maxmsg;             /* maximum number of messages */
    long msgsize;            /* Maximum message size */

    long count;              /* Number of messages currently queued */
    long start;              /* first not-empty message */

    BYTE *mq_data;           /* the data... */
    struct mq_elem *mq_info; /* the priorities */
    int mq_first;            /* the first empty message */

    struct sigevent notification; /* the notification, valid only if the
                                     correct bit is set */

    /* the blocked processes queues */
    IQUEUE blocked_send;
    IQUEUE blocked_rcv;

    int next;            /* the mq queue */
} mq_table[MQ_OPEN_MAX];


/* this -IS- an extension to the proc_table!!! */
static struct {
    int intsig;   /* Normally it is =0, -1 only when a task is woken up
                     by a signal */
    int mqdes;    /* message queue on which a task is blocked (meaningless
                     if the task is not blocked...) */
} mqproc_table[MAX_PROC];

static int free_mq;         /* Queue of free sem                    */

mqd_t mq_open(const char *name, int oflag, ...)
{
    int     i;
    int     found = 0;
    mode_t  m;
    mqd_t   mq;
    struct mq_attr *attr;
    SYS_FLAGS f;

    f = kern_fsave();

    for (i = 0; i < MQ_OPEN_MAX; i++)
      if (mq_table[i].flags & MQ_USED) {
        if (strcmp((char*)name, mq_table[i].name) == 0) {
          found = 1;
          break;
        }
      }
    if (found) {
      if (oflag == (O_CREAT | O_EXCL)) {
          errno = EEXIST;
          kern_frestore(f);
          return -1;
      } else {
          kern_frestore(f);
          return i;
      }
    } else {
      if (!(oflag & O_CREAT)) {
          errno = ENOENT;
          kern_frestore(f);
          return -1;
      } else if (!(oflag & O_RDWR)) {
          errno = EACCES;
          kern_frestore(f);
          return -1;
      } else {
          va_list l;

          va_start(l, oflag);
            m = va_arg(l,mode_t);
            attr = va_arg(l, struct mq_attr *);
          va_end(l);

          mq = free_mq;
          if (mq != -1) {
            mq_table[mq].name = kern_alloc(strlen((char *)name)+1);
            if (!mq_table[mq].name) {
              errno = ENOSPC;
              kern_frestore(f);
              return -1;
            }
            strcpy(mq_table[mq].name, (char *)name);

            if (attr) {
              mq_table[mq].maxmsg  = attr->mq_maxmsg;
              mq_table[mq].msgsize = attr->mq_msgsize;
            }
            else {
              mq_table[mq].maxmsg  = MQ_DEFAULT_MAXMSG;
              mq_table[mq].msgsize = MQ_DEFAULT_MSGSIZE;
            }
            iq_init(&mq_table[mq].blocked_send, &freedesc, 0);
            iq_init(&mq_table[mq].blocked_rcv, &freedesc, 0);

            mq_table[mq].count = 0;
            mq_table[mq].start = -1;
        
            mq_table[mq].mq_first = 0;

            if (oflag & O_NONBLOCK)
              mq_table[mq].flags = MQ_USED | MQ_NONBLOCK;
            else
              mq_table[mq].flags = MQ_USED;

            mq_table[mq].mq_data = (BYTE *)
              kern_alloc(mq_table[mq].maxmsg * mq_table[mq].msgsize);
            if (!mq_table[mq].mq_data) {
              kern_free(mq_table[mq].name,strlen((char *)name)+1);

              errno = ENOSPC;
              kern_frestore(f);
              return -1;
            }

            mq_table[mq].mq_info = (struct mq_elem *)
              kern_alloc(mq_table[mq].maxmsg * sizeof(struct mq_elem));
            if (!mq_table[mq].mq_info) {
              kern_free(mq_table[mq].name,strlen((char *)name)+1);
              kern_free(mq_table[mq].mq_data,
                        mq_table[mq].maxmsg * mq_table[mq].msgsize);

              errno = ENOSPC;
              kern_frestore(f);
              return -1;
            }

            /* set up the element queue */
            for (i=0; i<mq_table[mq].maxmsg-1; i++)
              mq_table[mq].mq_info[i].next = i+1;
            mq_table[mq].mq_info[mq_table[mq].maxmsg-1].next = -1;
            mq_table[mq].mq_first = 0;

            free_mq = mq_table[mq].next;
            kern_frestore(f);
            return mq;
          }
          else {
    	    errno = ENOSPC;
    	    kern_frestore(f);
    	    return -1;
          }
      }
    }
}

int mq_close(mqd_t mqdes)
{
    SYS_FLAGS f;

    f = kern_fsave();

    if (mqdes < 0 ||
        mqdes >= MQ_OPEN_MAX ||
        !(mq_table[mqdes].flags & MQ_USED) ) {
      errno = EBADF;
      kern_frestore(f);
      return -1;
    }

    kern_free(mq_table[mqdes].name, strlen(mq_table[mqdes].name)+1);
    kern_free(mq_table[mqdes].mq_data,
              mq_table[mqdes].maxmsg * mq_table[mqdes].msgsize);
    kern_free(mq_table[mqdes].mq_info,
              mq_table[mqdes].maxmsg * sizeof(struct mq_elem));

    mq_table[mqdes].flags = 0;
    mq_table[mqdes].next = free_mq;
    free_mq = mqdes;

    kern_frestore(f);
    return 0;
}

int mq_unlink(const char *name)
{
    int i;
    int found = 0;
    SYS_FLAGS f;

    f = kern_fsave();

    for (i = 0; i < MQ_OPEN_MAX; i++)
      if (mq_table[i].flags & MQ_USED) {
        if (strcmp((char*)name, mq_table[i].name) == 0) {
          found = 1;
        }
      }

    if (found) {
      kern_free(mq_table[i].name, strlen((char *)name)+1);
      kern_free(mq_table[i].mq_data,
                mq_table[i].maxmsg * mq_table[i].msgsize);
      kern_free(mq_table[i].mq_info,
                mq_table[i].maxmsg * sizeof(struct mq_elem));
  
      mq_table[i].flags = 0;
      mq_table[i].next = free_mq;
      free_mq = i;
      kern_frestore(f);
      return 0;
    } else {
      errno = ENOENT;
      kern_frestore(f);
      return -1;
    }
}

/* this function inserts a message in amessage queue mantaining the
   priority order */
static void insert_mq_entry(mqd_t mqdes, int newmsg)
{
    int prio; /* the priority of the message to insert */
    int p,q;  /* the messages... */

    p = NIL;
    q = mq_table[mqdes].start;
    prio = mq_table[mqdes].mq_info[ newmsg ].mq_prio;

    while ((q != NIL) && (prio <= mq_table[mqdes].mq_info[ q ].mq_prio)) {
	p = q;
	q = mq_table[mqdes].mq_info[ q ].next;
    }

    if (p != NIL)
      mq_table[mqdes].mq_info[ p ].next = newmsg;
    else
      mq_table[mqdes].start = newmsg;

    mq_table[mqdes].mq_info[ newmsg ].next = q;
}






/* this is the test that is done when a task is being killed
   and it is waiting on a sigwait */
static int mq_cancellation_point(PID i, void *arg)
{
    LEVEL l;

    if (proc_table[i].status == WAIT_MQSEND) {
      /* the task that have to be killed is waiting on a mq_send */

      /* we have to extract the task from the blocked queue... */
      iq_extract(i,&mq_table[mqproc_table[i].mqdes].blocked_send);

      /* and the task have to be reinserted into the ready queues, so it
         will fall into task_testcancel */
      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    if (proc_table[i].status == WAIT_MQRECEIVE) {
      /* the task that have to be killed is waiting on a mq_send */

      /* we have to extract the task from the blocked queue... */
      iq_extract(i, &mq_table[mqproc_table[i].mqdes].blocked_rcv);

      /* and the task have to be reinserted into the ready queues, so it
         will fall into task_testcancel */
      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    return 0;
}

int mq_interrupted_by_signal(PID i, void *arg)
{
    LEVEL l;

    if (proc_table[i].status == WAIT_MQSEND) {
      /* the task is waiting on a nanosleep and it is still receiving a
         signal... */
      mqproc_table[exec_shadow].intsig = 1;

      /* we have to extract the task from the blocked queue... */
      iq_extract(i, &mq_table[mqproc_table[i].mqdes].blocked_send);

      /* and the task have to be reinserted into the ready queues, so it
         will fall into task_testcancel */
      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    if (proc_table[i].status == WAIT_MQRECEIVE) {
      /* the task is waiting on a nanosleep and it is still receiving a
         signal... */
      mqproc_table[exec_shadow].intsig = 1;

      /* we have to extract the task from the blocked queue... */
      iq_extract(i, &mq_table[mqproc_table[i].mqdes].blocked_rcv);

      /* and the task have to be reinserted into the ready queues, so it
         will fall into task_testcancel */
      l = proc_table[i].task_level;
      level_table[l]->public_unblock(l,i);

      return 1;
    }

    return 0;
}





int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
            unsigned int msg_prio)
{
  int newmsg;
  SYS_FLAGS f;

  task_testcancel();

  f = kern_fsave();

  /* first, if it is the first time that mq_receive or mq_send is called,
     register the cancellation point */
  if (mq_once) {
    mq_once = 0;
    register_cancellation_point(mq_cancellation_point, NULL);
    register_interruptable_point(mq_interrupted_by_signal, NULL);
  }

  if (mqdes < 0 ||
      mqdes >= MQ_OPEN_MAX ||
      !(mq_table[mqdes].flags & MQ_USED) ) {
    errno = EBADF;
    kern_frestore(f);
    return -1;
  }

  if (msg_len > mq_table[mqdes].msgsize) {
    errno = EMSGSIZE;
    kern_frestore(f);
    return -1;
  }

  if (msg_prio > MQ_PRIO_MAX) {
    errno = EINVAL;
    kern_frestore(f);
    return -1;
  }

  /* block the task if necessary */
  if (mq_table[mqdes].mq_first == -1) {
    /* the message queue is full!!! */
    if (mq_table[mqdes].flags & O_NONBLOCK) {
      errno = EAGAIN;
      kern_frestore(f);
      return -1;
    }
    else {
      LEVEL l;

      /* we block the task until:
         - a message is received, or
         - a signal is sent to the task, or
         - the task is killed               */

      mqproc_table[exec_shadow].intsig = 0;

      kern_epilogue_macro();
    
      l = proc_table[exec_shadow].task_level;
      level_table[l]->public_block(l,exec_shadow);

      /* we insert the task in the message queue */
      proc_table[exec_shadow].status = WAIT_MQSEND;
      iq_priority_insert(exec_shadow,&mq_table[mqdes].blocked_send);

      /* and finally we reschedule */
      exec = exec_shadow = -1;
      scheduler();
      ll_context_to(proc_table[exec_shadow].context);
      kern_deliver_pending_signals();

      /* mq_send is a cancellation point... */
      task_testcancel();

      if (mqproc_table[exec_shadow].intsig) {
        errno = EINTR;
        kern_frestore(f);
        return -1;
      }
    }
  }

  /* Now there is space to insert a new message */
  /* alloc a descriptor */
  newmsg = mq_table[mqdes].mq_first;
  mq_table[mqdes].mq_first = mq_table[mqdes].mq_info[newmsg].next;
  mq_table[mqdes].count++;

  /* fill the data */
  memcpy(mq_table[mqdes].mq_data + newmsg * mq_table[mqdes].msgsize,
         msg_ptr, msg_len);
  mq_table[mqdes].mq_info[ newmsg ].mq_prio = msg_prio;
  mq_table[mqdes].mq_info[ newmsg ].msglen  = msg_len;

  /* insert the data in an ordered way */
  insert_mq_entry(mqdes, newmsg);

//  kern_printf("Ûmq_des=%d, newmsg=%d, count=%dÛ",
//              mqdes, newmsg, mq_table[mqdes].count);

  if (mq_table[mqdes].count == 1) {
    /* the mq was empty */
    PID p;

    p = iq_getfirst(&mq_table[mqdes].blocked_rcv);

    if ( p != NIL) {
      /* The first blocked task has to be woken up */
      LEVEL l;

      proc_table[exec_shadow].context = ll_context_from();

      l = proc_table[p].task_level;
      level_table[l]->public_unblock(l,p);

      /* Preempt if necessary */
      scheduler();
      kern_context_load(proc_table[exec_shadow].context);
      return 0;
    }
    else if (mq_table[mqdes].flags & MQ_NOTIFICATION_PRESENT) {
      mq_table[mqdes].flags &= ~MQ_NOTIFICATION_PRESENT;

      // manage the notification...
      if (mq_table[mqdes].notification.sigev_notify == SIGEV_SIGNAL) {
          // there is no signal pending... post the signal!!!
          sigqueue_internal(0,
                            mq_table[mqdes].notification.sigev_signo,
                            mq_table[mqdes].notification.sigev_value,
                            SI_MESGQ);
      } else if (mq_table[mqdes].notification.sigev_notify == SIGEV_THREAD) {
        /* a new thread must be created; note that the pthread_create
           calls task_createn and task_activate; if task_activate is called
           into signal handlers and calls event_need_reschedule */
        pthread_t new_thread;
    
        if (mq_table[mqdes].notification.sigev_notify_attributes)
          pthread_create(&new_thread,
                         mq_table[mqdes].notification.sigev_notify_attributes,
                         (void *(*)(void *))mq_table[mqdes].notification.sigev_notify_function,
                         mq_table[mqdes].notification.sigev_value.sival_ptr);
        else {
          pthread_attr_t new_attr;
          // the task must be created detached
          pthread_attr_init(&new_attr);
          pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
    
          pthread_create(&new_thread,
                         &new_attr,
                         (void *(*)(void *))mq_table[mqdes].notification.sigev_notify_function,
                         &mq_table[mqdes].notification.sigev_value);
        }
      }
    }
  }

  kern_frestore(f);
  return 0;
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned int *msg_prio)
{
  int msg;
  PID p;
  ssize_t returnvalue;
  SYS_FLAGS f;

  task_testcancel();

  f = kern_fsave();

  /* first, if it is the first time that mq_receive or mq_send is called,
     register the cancellation point */
  if (mq_once) {
    mq_once = 0;
    register_cancellation_point(mq_cancellation_point, NULL);
    register_interruptable_point(mq_interrupted_by_signal, NULL);
  }

  if (mqdes < 0 ||
      mqdes >= MQ_OPEN_MAX ||
      !(mq_table[mqdes].flags & MQ_USED) ) {
    errno = EBADF;
    kern_frestore(f);
    return -1;
  }

  if (msg_len > mq_table[mqdes].msgsize) {
    errno = EMSGSIZE;
    kern_frestore(f);
    return -1;
  }

  /* block the task if necessary */
  if (mq_table[mqdes].start == -1) {
    /* the message queue is empty!!! */
    if (mq_table[mqdes].flags & O_NONBLOCK) {
      errno = EAGAIN;
      kern_frestore(f);
      return -1;
    }
    else {
      LEVEL l;

      /* we block the task until:
         - a message arrives, or
         - a signal is sent to the task, or
         - the task is killed               */

      mqproc_table[exec_shadow].intsig = 0;

      kern_epilogue_macro();
    
      l = proc_table[exec_shadow].task_level;
      level_table[l]->public_block(l,exec_shadow);

      /* we insert the task into the message queue */
      proc_table[exec_shadow].status = WAIT_MQRECEIVE;
      iq_priority_insert(exec_shadow,&mq_table[mqdes].blocked_rcv);

      /* and finally we reschedule */
      exec = exec_shadow = -1;
      scheduler();
      ll_context_to(proc_table[exec_shadow].context);
      kern_deliver_pending_signals();

      /* mq_receive is a cancellation point... */
      task_testcancel();

      if (mqproc_table[exec_shadow].intsig) {
        errno = EINTR;
        kern_frestore(f);
        return -1;
      }
    }
  }

  /* Now there is at least one message...
     copy it to the destination, ... */
  msg = mq_table[mqdes].start;
  memcpy(msg_ptr,
         mq_table[mqdes].mq_data + msg * mq_table[mqdes].msgsize,
         mq_table[mqdes].msgsize);

  /* ...update the first messagee and the counters, ... */
  mq_table[mqdes].count++;
  mq_table[mqdes].start = mq_table[mqdes].mq_info[ msg ].next;
  /* and finally the free message queue */
  mq_table[mqdes].mq_info[ msg ].next = mq_table[mqdes].mq_first;
  mq_table[mqdes].mq_first = msg;

  /* return the priority if required */
  if (msg_prio) {
    *msg_prio = mq_table[mqdes].mq_info[ msg ].mq_prio;
  }

  /* set the returnvalue */
  returnvalue = mq_table[mqdes].mq_info[ msg ].msglen;

  /* if the mq was full, there may be a task into blocked-send queue */
  p = iq_getfirst(&mq_table[mqdes].blocked_send);

  if ( p != NIL) {
    /* The first blocked task on send has to be woken up */
    LEVEL l;

    proc_table[exec_shadow].context = ll_context_from();

    l = proc_table[p].task_level;
    level_table[l]->public_unblock(l,p);

    /* Preempt if necessary */
    scheduler();
    kern_context_load(proc_table[exec_shadow].context);
    return returnvalue;
  }

  kern_frestore(f);
  return returnvalue;
}

int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (mqdes < 0 ||
      mqdes >= MQ_OPEN_MAX ||
      !(mq_table[mqdes].flags & MQ_USED) ) {
    errno = EBADF;
    kern_frestore(f);
    return -1;
  }

  if (mq_table[mqdes].flags & MQ_NOTIFICATION_PRESENT) {
    if (!notification) {
      mq_table[mqdes].flags &= ~MQ_NOTIFICATION_PRESENT;
      kern_frestore(f);
      return 0;
    }
    else {
      errno = EBUSY;
      kern_frestore(f);
      return -1;
    }
  }

  mq_table[mqdes].flags |= MQ_NOTIFICATION_PRESENT;

  memcpy(&mq_table[mqdes].notification, notification,sizeof(struct sigevent));

  kern_frestore(f);
  return 0;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat,
               struct mq_attr *omqstat)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (mqdes < 0 ||
      mqdes >= MQ_OPEN_MAX ||
      !(mq_table[mqdes].flags & MQ_USED) ) {
    errno = EBADF;
    kern_frestore(f);
    return -1;
  }

  if (omqstat) {
    omqstat->mq_flags   = mq_table[mqdes].flags & O_NONBLOCK;
    omqstat->mq_maxmsg  = mq_table[mqdes].maxmsg;
    omqstat->mq_msgsize = mq_table[mqdes].msgsize;
    omqstat->mq_curmsgs = mq_table[mqdes].count;
  }

  mq_table[mqdes].flags = (mq_table[mqdes].flags & ~O_NONBLOCK) |
                          (mqstat->mq_flags & O_NONBLOCK);
  kern_frestore(f);
  return 0;
}

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (mqdes < 0 ||
      mqdes >= MQ_OPEN_MAX ||
      !(mq_table[mqdes].flags & MQ_USED) ) {
    errno = EBADF;
    kern_frestore(f);
    return -1;
  }

  mqstat->mq_flags   = mq_table[mqdes].flags & O_NONBLOCK;
  mqstat->mq_maxmsg  = mq_table[mqdes].maxmsg;
  mqstat->mq_msgsize = mq_table[mqdes].msgsize;
  mqstat->mq_curmsgs = mq_table[mqdes].count;

  kern_frestore(f);
  return 0;
}
