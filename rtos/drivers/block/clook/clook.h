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

#ifndef __CLOOK_H
#define __CLOOK_H

#include "phdsk.h"
#include "glue.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct TAGclook_queue_t {
  struct phdskinfo        *disk;
  /**/
  __b_fastmutex_t         mutex;
  int                     act;  
  struct request_prologue *queue[2];
  int counter;  
} clook_queue_t;

struct request_specific {
  void *next;
};

#define bqueue_t clook_queue_t

#define bqueue_init            clook_init
#define bqueue_numelements     clook_numelements 
#define bqueue_insertrequest   clook_insertrequest
#define bqueue_getrequest      clook_getrequest
#define bqueue_removerequest   clook_removerequest

__END_DECLS
#endif
