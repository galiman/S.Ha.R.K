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
 CVS :        $Id: hartport.h,v 1.1 2005/02/25 10:53:41 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:41 $
 ------------

 This file contains the Hartik 3.3.0's port primitives

 Title:
   HARTPORT (Hartik Ports)

 Resource Models Accepted:
   None

 Description:
   This module contains a port library compatible with the Hartik one.

 Exceptions raised:
   None

 Restrictions & special features:
   - This module is NOT Posix compliant

 Author:	Giuseppe Lipari
 Date:	9/5/96
 Revision:	2.0
 Date:	14/3/97

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

#ifndef __PORT_H__
#define __PORT_H__

#include <sem/sem/sem.h>
#include <ll/ll.h>

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* $HEADER-    */
//#ifndef __HW_DEP_H__
//#include "hw_dep.h"
//#endif
/* $HEADER+    */

typedef	short PORT;

#define MAX_PORT        15U           /*+ Maximum number of ports      +*/
#define MAX_PORT_INT    30U           /*+ Max num. of port connections +*/
#define MAX_PORT_NAME   20U           /*+ Maximum port name length     +*/
#define MAX_HASH_ENTRY  MAX_PORT      /*+ More port stuff              +*/


#define STREAM		1
#define MAILBOX		2
#define STICK		3

#define	READ		0
#define WRITE		1



/*+ This function must be inserted in the __hartik_register_levels__ +*/
void HARTPORT_init(void);


/* Debugging functions */
void print_port(void);
void port_write(PORT p);

/* User level port primitives */
PORT port_create(char *name, int dim_mes, int num_mes, BYTE type, BYTE access);
PORT port_connect(char *name, int dim_mes, BYTE type, BYTE access);
void port_delete(PORT p);
void port_disconnect(PORT p);

WORD port_send(PORT p,void *m,BYTE wait);
WORD port_receive(PORT p,void *m,BYTE wait);

__END_DECLS
#endif /* __PORT_H__ */

