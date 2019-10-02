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

/*
 * Copyright (C) 1999,2000 Massimiliano Giorgi
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

#ifndef __PSCAN_H
#define __PSCAN_H

#include "phdsk.h"
#include "glue.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* must be in sync with <modules/bd_pscan.h> */
#define NUMPRIORITY 4

typedef struct TAGpscan_queue_t {
  struct phdskinfo          *disk;
  /**/
  __b_fastmutex_t           mutex;
  int                       counter;
  int                       actprior;
  struct pscan_queues{
    int                     dir;  
    struct request_prologue *queue[2];
  } pri[NUMPRIORITY];
} pscan_queue_t;

struct request_specific {
  void *next;
};

#define bqueue_t pscan_queue_t

#define bqueue_init            pscan_init
#define bqueue_numelements     pscan_numelements 
#define bqueue_insertrequest   pscan_insertrequest
#define bqueue_getrequest      pscan_getrequest
#define bqueue_removerequest   pscan_removerequest

__END_DECLS
#endif
