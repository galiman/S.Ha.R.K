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

#ifndef __TRACE_QCIRC_H
#define __TRACE_QCIRC_H

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct TAGtrc_circular_queue_args_t {
  char *filename;
  int size;

  long period;
  long slice;
  
  int flags;
} TRC_CIRCULAR_PARMS;

#define TRC_CIRCULAR_ONLINETASK          0x0001

#define trc_circular_default_parms(m)    (m).filename=NULL,\
                                         (m).size=8192,\
					 (m).period=500000,\
					 (m).slice=25000,\
				         (m).flags=0
#define trc_circular_def_filename(m,s)   (m).filename=(s)
#define trc_circular_def_size(m,s)       (m).size=(s)
					 
#define trc_circular_def_onlinetask(m)   (m).flags|=TRC_CIRCULAR_ONLINETASK
#define trc_circular_def_period(m,p)     (m).period=p
#define trc_circular_def_slice(m,s)      (m).slice=s
					 
int trc_register_circular_queue(void);

__END_DECLS
#endif
