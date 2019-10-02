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
 CVS :        $Id: cabs.h,v 1.1 2005/02/25 10:53:41 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:41 $
 ------------

 This file contains the Hartik 3.3.1 CAB functions


 Author:      Gerardo Lamastra
              Giuseppe Lipari
 Date:        9/5/96

 File:        Cabs.H
 Revision:    1.1
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

/* $HEADER+ */

#ifndef __CAB_H__
#define __CAB_H__

#include <ll/sys/types.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define MAX_CAB_NAME 10               /*+ Max. n. of chars for a CAB   +*/

#define MAX_CAB         50U           /*+ Maximum number of CABs       +*/


typedef int CAB;

/*+ This function must be inserted in the __hartik_register_levels__ +*/
void CABS_register_module(void);

/* User level CAB primitives */
CAB cab_create(char *name, int dim_mes, BYTE num_mes);
char *cab_reserve(CAB id);
int cab_putmes(CAB id, void *pbuf);
char *cab_getmes(CAB id);
int cab_unget(CAB id, void *pun_mes);
void cab_delete(CAB id);

__END_DECLS
#endif /* __CAB_H__ */

