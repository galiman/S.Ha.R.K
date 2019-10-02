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
 * CVS :        $Id: trace.h,v 1.3 2003/03/13 13:37:02 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.3 $
 * Last update: $Date: 2003/03/13 13:37:02 $
 */

#ifndef __TRACE_TRACE_H
#define __TRACE_TRACE_H

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct trc_parms {
  char *path;
} TRC_PARMS;

#define trc_default_parms(m) (m).path="/temp"
#define trc_def_path(m,s) (m).path=(s)

int TRC_init_phase1(TRC_PARMS *p);
int TRC_init_phase2(void);

int trc_assign_event_to_queue(int event, int queue);
int trc_assign_class_to_queue(int class, int queue);

int trc_notrace_event(int event);
int trc_trace_event(int event);
int trc_notrace_class(int class);
int trc_trace_class(int class);

/* -- */

/* Register a "standard" tracer configuration; requires FAT16 filesystem
   (see documentation) */
int TRC_init_phase1_standard(void);
int TRC_init_phase2_standard(void);

__END_DECLS
#endif
