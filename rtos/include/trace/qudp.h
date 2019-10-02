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

#ifndef __TRACE_QUDP_H
#define __TRACE_QUDP_H

#include <kernel/model.h>
#include <drivers/udpip.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct TAGtrc_udp_queue_args_t {
  int size;
  UDP_ADDR local,remote; 
  TASK_MODEL *model;
} TRC_UDP_PARMS;

#define trc_udp_default_parms(m,l,r) (m).size=8192, \
                                     (m).model = NULL, \
                                     (m).local = (l), \
                                     (m).remote = (r);
                                    
#define trc_udp_def_size(m,s)    (m).size=(s)
#define trc_udp_def_local(m,l)   (m).local=(l)
#define trc_udp_def_remote(m,r)  (m).remote=(r)
#define trc_udp_def_model(m,mod) (m).model=(mod)


int trc_register_udp_queue(void);

__END_DECLS
#endif
