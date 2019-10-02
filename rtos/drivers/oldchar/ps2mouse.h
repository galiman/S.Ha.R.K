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
 CVS :        $Id: ps2mouse.h,v 1.1 2003/03/24 10:54:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 10:54:17 $
 ------------

 Author:      Massimiliano Giorgi


 Virtual operation on a PS/2 mouse
 (for mouse.c)


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

#ifndef __PS2MOUSE_H__
#define __PS2MOUSE_H__

#include <drivers/mouse.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

extern int C8042_ps2mousepresent(void);
#define ps2mouse_present() C8042_ps2mousepresent()

int  ps2mouse_open(void *);
void ps2mouse_close(void);
void ps2mouse_wait(void);
int  ps2mouse_get(BYTE *);
void ps2mouse_enable(void);
void ps2mouse_disable(void);

//extern TASK ps2mouse_server(void);
extern TASK (*ps2mouse_getserveraddr(void))(void);

extern int M_ps2(MOUSE_EVT *evt, unsigned char *data);

__END_DECLS
#endif
