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
 CVS :        $Id: mqueue.h,v 1.2 2003/03/13 13:41:04 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:41:04 $
 ------------

 POSIX message queues:

 - mq names have not a correspondence into the File System.
 - a message queue can only be opened with the O_RDWR flag
   (there is only a process!!!)

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

#ifndef __MQUEUE_H__
#define __MQUEUE_H__

#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef int mqd_t;

struct mq_attr {
  long mq_flags;        /* Message queue flags */
  long mq_maxmsg;       /* maximum number of messages */
  long mq_msgsize;      /* Maximum message size */
  long mq_curmsgs;      /* Number of messages currently queued */
};

// non-standard defaults limits used into mq_open when no mqstat is defined
#define MQ_DEFAULT_MAXMSG   10
#define MQ_DEFAULT_MSGSIZE 128


// defined into the POSIX Standard, 15.1.1

mqd_t mq_open(const char *name, int oflag, ...);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
            unsigned int msg_prio);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned int *msg_prio);
int mq_notify(mqd_t mqdes, const struct sigevent *notification);
int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat,
               struct mq_attr *omqstat);
int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);

__END_DECLS
#endif
