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
 * CVS :        $Id: queues.h,v 1.1 2003/12/10 16:48:46 giacomo Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1 $
 * Last update: $Date: 2003/12/10 16:48:46 $
 */

#ifndef __TRACE_QUEUES_H
#define __TRACE_QUEUES_H

#include <ll/sys/types.h>
#include "types.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct TAGtrc_queue_t {
  int type;
  trc_event_t *(*get)(void *);
  int (*post)(void *);
  void *data;
} trc_queue_t;

#define TRC_DUMMY_QUEUE       0
#include <qdummy.h>

#define TRC_FIXED_QUEUE       1
#define TRC_DOSFS_FIXED_QUEUE 2
#include <qfixed.h>

#define TRC_CIRCULAR_QUEUE    3
#include <qcirc.h>

#define TRC_UDP_QUEUE         4
#include <qudp.h>

#define TRC_QUEUETYPESNUMBER  5

/* uniq!=0 append a this unique number to name */
void trc_create_name(char *basename, int uniq, char *pathname);

/*--*/

int trc_register_queuetype(int queuetype,
			   int(*create)(trc_queue_t *,void *),
			   int(*activate)(void *,int),
			   int(*terminate)(void *));

int trc_create_queue(int queuetype, void *arg);				    

__END_DECLS
#endif
