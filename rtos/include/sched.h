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
 CVS :        $Id: sched.h,v 1.2 2003/03/13 13:41:04 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:41:04 $
 ------------

 sched.h

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


#ifndef __SCHED_H__
#define __SCHED_H__

#include <kernel/model.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define SCHED_FIFO  NRT_FIFO_POLICY
#define SCHED_RR    NRT_RR_POLICY
#define SCHED_OTHER NRT_RR_POLICY

struct sched_param {
  int sched_priority;
};

// functions NRQ for PSE52
int sched_get_priority_max(int policy);
extern __inline__ int sched_get_priority_min(int policy) {return 0; }
int sched_rr_get_interval(pid_t pid, struct timespec *interval);
//  sched_getparam
//  sched_getscheduler
//  sched_setparam
//  sched_setscheduler

int sched_yield(void);

__END_DECLS
#endif
