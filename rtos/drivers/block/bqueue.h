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

/***************************************

  CVS :        $Id: bqueue.h,v 1.2 2003/03/13 13:43:13 pj Exp $
 
  Revision:    $Revision: 1.2 $

  Last update: $Date: 2003/03/13 13:43:13 $

  This module is responsable of the protocol between the IDE device driver
  interface and the host (the computer).

***************************************/

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

#ifndef __BQUEUE_H
#define __BQUEUE_H

struct request_prolog;

#ifdef _SCHED_FCFS
#include "fcfs/fcfs.h"
#endif
#ifdef _SCHED_SSTF
#include "sstf/sstf.h"
#endif
#ifdef _SCHED_LOOK
#include "look/look.h"
#endif
#ifdef _SCHED_CLOOK
#include "clook/clook.h"
#endif
#ifdef _SCHED_EDF
#include "edf/edf.h"
#endif
#ifdef _SCHED_PSCAN
#include "pscan/pscan.h"
#endif

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define REQ_DUMMY  0
#define REQ_SEEK   1
#define REQ_READ   2
#define REQ_WRITE  3

struct request_prologue {
  unsigned sector;
  unsigned head;
  unsigned cylinder;
  unsigned nsectors;
  unsigned operation;
  /*--*/
  struct request_specific x;
};
#define request_prologue_t struct request_prologue

int bqueue_init(bqueue_t *);
int bqueue_numelements(bqueue_t *);
int bqueue_insertrequest(bqueue_t *, struct request_prologue *);
struct request_prologue *bqueue_getrequest(bqueue_t *);
int bqueue_removerequest(bqueue_t *);  
__END_DECLS
#endif
