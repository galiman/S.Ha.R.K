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
 CVS :        $Id: sermouse.h,v 1.1 2003/03/24 10:54:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 10:54:17 $
 ------------

 Author: Massimiliano Giorgi

 Virtual operation for a serial mouse
 (for mouse.c)

 Initial date: 22/mar/1999

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

#ifndef __SERMOUSE_H__
#define __SERMOUSE_H__

#include <drivers/mouse.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct sermouse_info {
  int type;       /* MSMOUSE, MMMOUSE,... */
  int port;       /* COM1,COM2,... */
  PID pid;        /* task PID */
} SERMOUSE_INFO;

SERMOUSE_INFO *sermouse_present(void);

//extern TASK microsoftmouse_server(void);
//extern TASK mousesystem_server(void);
TASK (*sermouse_getserveraddr(SERMOUSE_INFO *))(void);

int  sermouse_open(void*);
void sermouse_close(void);
void sermouse_wait(void);
int  sermouse_get(BYTE *);
void sermouse_enable(void);
void sermouse_disable(void);

int M_ms(MOUSE_EVT *evt,unsigned char *data);
int M_ms_plus(MOUSE_EVT *evt,unsigned char *data);
int M_ms_plus_lr(MOUSE_EVT *evt,unsigned char *data);
int M_bare(MOUSE_EVT *evt,unsigned char *data);
int M_msc(MOUSE_EVT *evt,unsigned char *data);
int M_sun(MOUSE_EVT *evt,unsigned char *data);
int M_mm(MOUSE_EVT *evt,unsigned char *data);
int M_logi(MOUSE_EVT *evt,unsigned char *data);

__END_DECLS
#endif
