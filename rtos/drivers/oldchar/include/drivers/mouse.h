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
 CVS :        $Id: mouse.h,v 1.2 2004/06/02 13:54:52 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2004/06/02 13:54:52 $
 ------------

**/

/**
 ------------
 CVS :        $Id: mouse.h,v 1.2 2004/06/02 13:54:52 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2004/06/02 13:54:52 $
 ------------

 Author:	Gerardo Lamastra
 Date:	9/5/96

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

/* Revision:    1.0.1                                           */
/* -- added support for PS/2 mouse                              */
/* -- changed mouse_init()                                      */

/* By Massy:
 * -- added support for mouse cursor (pointer)
 * -- interface changed
 */

#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <kernel/const.h>

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* mouse buttons constant */  
#define MOUSE_RBUTT	1
#define MOUSE_CBUTT     2
#define MOUSE_LBUTT	4

/* the mouse event struct */
typedef struct {
  int x,y;        /* mouse position */
  int dx,dy;      /* distance covered by mouse */ 
  int buttons;    /* buttons flags */
} MOUSE_EVT;

/* macros to test mouse buttons */
#define isLeftButton(m)    ((m).buttons & MOUSE_LBUTT)
#define isRightButton(m)   ((m).buttons & MOUSE_RBUTT)
#define isCentralButton(m) ((m).buttons & MOUSE_CBUTT)

/* user mouse handler */
typedef void (*MOUSE_HANDLER)(MOUSE_EVT*);

/* mouse types */
/* (run examples/mfind.c for a description)*/
#define MSMOUSE     0x00
#define MSPMOUSE    0x01
#define MSPLRMOUSE  0x02
#define BAREMOUSE   0x03
#define MSCMOUSE    0x04
#define SUNMOUSE    0x05
#define MMMOUSE     0x06
#define LOGIMOUSE   0x07
#define PS2MOUSE    0x08
#define NOMOUSE     0xff

/*
 * mouse initialization
 */

#define MOUSE_DEFAULT (DWORD)-1

/* the MOUSE_PARMS structure used by mouse_init() */
typedef struct mouse_parms {
  TASK_MODEL *tm;
  int   type;            /* mouse types (constants above) */
  /* for serial mouse */
  int   port;            /* serial port (i.e. COM1, ...)  */   
} MOUSE_PARMS;

/* the default values for the MOUSE_PARMS structure */
#define BASE_MOUSE {(TASK_MODEL *)MOUSE_DEFAULT,MOUSE_DEFAULT,MOUSE_DEFAULT}

/* to change the MOUSE_PARMS struct */
#define mouse_default_parms(s)        (s).tm = (TASK_MODEL *)MOUSE_DEFAULT, \
                                      (s).type = MOUSE_DEFAULT,             \
                                      (s).port = MOUSE_DEFAULT
#define mouse_def_ms(s,p)             (s).type=MSMOUSE;    (s).port=(p)
#define mouse_def_msplus(s,p)         (s).type=MSPMOUSE;   (s).port=(p)
#define mouse_def_mspluslr(s,p)       (s).type=MSPLRMOUSE; (s).port=(p)
#define mouse_def_bare(s,p)           (s).type=BAREMOUSE;  (s).port=(p)
#define mouse_def_msc(s,p)            (s).type=MSCMOUSE;   (s).port=(p)
#define mouse_def_sun(s,p)            (s).type=SUNMOUSE;   (s).port=(p)
#define mouse_def_mm(s,p)             (s).type=MMMOUSE;    (s).port=(p)
#define mouse_def_logi(s,p)           (s).type=LOGIMOUSE;  (s).port=(p)
#define mouse_def_ps2(s)              (s).type=PS2MOUSE
#define mouse_def_task(s,m)           (s).tm=(m)

/*
 * user mouse interface
 */
					      
int  mouse_init(MOUSE_PARMS *s);
void mouse_enable(void);
void mouse_disable(void);
void mouse_get(int *x,int *y,BYTE *button);
void mouse_position(int x,int y);
void mouse_limit(int x1,int y1,int x2,int y2);
void mouse_threshold(unsigned t);
void mouse_hook(MOUSE_HANDLER h);
void mouse_end(void);

/*
 *
 * mouse autocursor management
 *
 */

/* commands for mouse_grxcursor() & mouse_txtcursor() */
#define DISABLE   0x00
#define ENABLE    0x01

/* flags for mouse_grxcursor() & mouse_txtcursor() (to use with '|') */
#define WITHOUTSEM 0x10
#define AUTOOFF    0x20

/* dimensions of the grx shape */
#define MOUSESHAPEDX 16
#define MOUSESHAPEDY 16

/* hot-spot of the grx image (coordinates of the center's shape, zero-based) */
#define MOUSEHOTSPOTX 3
#define MOUSEHOTSPOTY 1

/* those macros can be used to set the correct mouse_limit() when 
 * the graphics autocursor is enable (to avoid wrong shape position because
 * there is not graphics clip functions)
 */
#define XMINLIMIT(dimx,dimy) (MOUSEHOTSPOTX)
#define XMAXLIMIT(dimx,dimy) ((dimx)-MOUSESHAPEDX+MOUSEHOTSPOTX)
#define YMINLIMIT(dimx,dimy) (MOUSEHOTSPOTY)
#define YMAXLIMIT(dimx,dimy) ((dimy)-MOUSESHAPEDY+MOUSEHOTSPOTY)

#define mouse_grxlimit(dimx,dimy) mouse_limit(\
XMINLIMIT(dimx,dimy), \
YMINLIMIT(dimx,dimy), \
XMAXLIMIT(dimx,dimy), \
YMAXLIMIT(dimx,dimy)  \
)

/* these are used to select the mouse shape */
int mouse_txtshape(DWORD img);
int mouse_grxshape(BYTE *shape,BYTE *mask);

/* enable/disable mouse pointer */
/* (return <0 on error) */
/* (for the cmd parameter see above) */
int mouse_grxcursor(int cmd);
int mouse_txtcursor(int cmd);

/* mouse on/off (or show/hide) */
void (*mouse_on)(void);
void (*mouse_off)(void);

__END_DECLS
#endif
