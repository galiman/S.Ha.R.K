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
 CVS :        $Id: mouse.c,v 1.2 2004/06/03 09:51:45 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2004/06/03 09:51:45 $
 ------------

 Author:	Gerardo Lamastra
 Date:	9/5/96

 Revision:	1.1b
 Last update: 14/apr/1999

 Serial mouse driver
 The mouse driver receives data from the serial RxServer()
 Then it processes the mouse packet and sets the variables
 representing the external mouse status.

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

/* Revision 1.1                     
 * Author:      Massimiliano Giorgi 
 * 
 * -- added support for PS/2 mouse  
 *
 * Detailed changed from 1.0g:
 * -- now mouse.c use "virtual operation"
 * -- all procedure changed to use "virtual operation"
 * ("virtual operation" are indipendent of the phisical device that
 * is used to comunicate with mouse so it can handle serial mouse, ps/2 mouse)
 * (all changed marked with a "MG")
 */

/* Revision 1.1b
 * Author:      Massimiliano Giorgi
 *
 * Changed to compile on 3.2.0
 * -- added wcet time
 * -- RECEIVE ports now are STREAM ports
 */

/*
 * Changelog:
 * -- Added support for various serial mouse
 * -- Virtual operations changed
 * -- Added support for auto-pointer
 */

/*
 * auto-pointer functions now are into
 * mcurtxt.c and mcurgrx.c
 * (to prevent automatic graphical library inclusion)
 */

//#include <string.h>
//#include <stdlib.h>
//#include <cons.h>
//#include <mem.h>

//#include "vm.h"
#include <kernel/kern.h>
//#include "exc.h"

#include <drivers/mouse.h>

/*if defined: then a trace of the initialization is performed */
// #define __DEBUG_INIT__

/* if defined: when a byte is received is displayed on the screen */
//#define __XTRA_DEBUG_MOUSE__

/*
 *
 * Operation on a virtual mouse
 * (added by MG)
 *
 */

#include "_mouse.h"
#include "sermouse.h"
#include "ps2mouse.h"

struct mouse_operations vmouse[]={

  {"ms",   "The original ms protocol, with a middle-button extension.",
           {0x40, 0x40, 0x40, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_ms
  },

  {"ms+",  "Like 'ms', but allows dragging with the middle button.",
           {0x40, 0x40, 0x40, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_ms_plus
  },

  {"ms+lr", "'ms+', but you can reset m by pressing lr.",
            {0x40, 0x40, 0x40, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_ms_plus_lr
  },

  {"bare", "Unadorned ms protocol. Needed with some 2-buttons mice.",
           {0x40, 0x40, 0x40, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_bare
  },

  {"msc",  "Mouse-Systems-Compatible (5bytes). Most 3-button mice.",
           {0xf8, 0x80, 0x00, 0x00}, 5, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_msc
  },

  {"sun",  "'msc' protocol, but only 3 bytes per packet.",
           {0xf8, 0x80, 0x00, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_sun
  },

  {"mm",   "MM series. Probably an old protocol...",
           {0xe0, 0x80, 0x80, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_mm
  },

  {"logi", "Used in some Logitech devices (only serial).",
           {0xe0, 0x80, 0x80, 0x00}, 3, 1, 0, 0, 0,
    sermouse_open,
    sermouse_close,
    sermouse_wait,
    sermouse_get,
    sermouse_enable,
    sermouse_disable,
    M_logi
  },

  {"ps2",  "Busmice of the ps/2 series. Most busmice, actually.",
           {0xc0, 0x00, 0x00, 0x00}, 3, 1, 0, 0, 0,
    ps2mouse_open,
    ps2mouse_close,
    ps2mouse_wait,
    ps2mouse_get,
    ps2mouse_enable,
    ps2mouse_disable,
    M_ps2
  },

  {"\0",     "",
           {0, 0, 0, 0}, 0, 0, 0, 0, 0,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
  }

};

int  (*open_mouse)(void*);    /* open comunication device */
void (*close_mouse)(void);    /* close comunication device */
void (*wait_mouse)(void);     /* wait to have a BYTE (blocking operation)*/
int  (*get_mouse)(BYTE *);      /* get last BYTE */
void (*enable_mouse)(void);   /* enable interface */
void (*disable_mouse)(void);  /* disable interface */

char *name_mouse;             /* mouse name */ 
char *desc_mouse;             /* mouse description */
BYTE *proto_mouse;            /* mouse protocol (see below) */
int  packetlen_mouse;         /* mouse packet lenght */

int (*decode_mouse)(MOUSE_EVT *evt,unsigned char *data);

/*
 * proto_mouse is used to syncronize the device driver:
 * if A is the first byte read and B the second, this is a valid 
 * start of a mouse protocol packet if
 * (A&proto_mouse[0])==proto_mouse[1])&&(b&proto_mouse[2])==proto_mouse[3])
 */

/* type of mouse to use for virtual operation */
int mousetype=NOMOUSE;

/* fill the variabled used to call the virtuals operations */
static void mouse_settype(int type)
{
    mousetype=type;

    open_mouse   =vmouse[mousetype].open;
    close_mouse  =vmouse[mousetype].close;
    wait_mouse   =vmouse[mousetype].wait;
    get_mouse    =vmouse[mousetype].get;
    enable_mouse =vmouse[mousetype].enable;
    disable_mouse=vmouse[mousetype].disable;

    name_mouse     =vmouse[mousetype].name;
    desc_mouse     =vmouse[mousetype].desc;
    proto_mouse    =vmouse[mousetype].proto;
    packetlen_mouse=vmouse[mousetype].packetlen;

    decode_mouse=vmouse[mousetype].decode;
}

/*
 *
 * end (operation on a virtual mouse)
 *
 */

/* MG: this are no "static" (they are used by sermouse.c, ps2mouse.c) */
/*     "threshold" renamed "mouse_thresholdlim" */
short int mouse_lim_x1 = 0;
short int mouse_lim_y1 = 0;
short int mouse_lim_x2 = 79;
short int mouse_lim_y2 = 24;
short int mouse_x = 40;
short int mouse_x_mick = 0;
short int mouse_y = 12;
short int mouse_y_mick = 0;
short int mouse_buttons = 0;
short int mouse_thresholdlim = 5;
MOUSE_HANDLER mouse_handler = NULL;

MOUSE_HANDLER user_mouse_handler = NULL;
int autocursormode=DISABLE;

static PID mouse_pid = NIL;

/*
 * this is a general mouse server
 * useable by most mouse protocol implementation
 */

TASK generalmouse_server(void)
{
  static MOUSE_EVT event;
  BYTE data[8];
  int index;
  int mickey;

#ifdef __XTRA_DEBUG_MOUSE__
  char str[40];
#endif
    
  index=0;
  for(;;) {
#ifdef __XTRA_DEBUG_MOUSE__
    puts_xy(0,11,WHITE,"Waiting..");
#endif

    /* wait for mouse data */
    wait_mouse();

    /* get mouse data */
    //data[index]=get_mouse();
    index+=get_mouse(data+index);
    
#ifdef __XTRA_DEBUG_MOUSE__
    sprintf(str,"Mouse data[%01d]: %2x",index,data[index]);
    puts_xy(0,12+index,WHITE,str);
#endif

    switch (index) {
      case 1:
	if ((data[0]&proto_mouse[0])!=proto_mouse[1]) index=0;
	break;
      case 2:
	if ((data[1]&proto_mouse[2])==proto_mouse[3]) index=0;
	break;
      default:
	if (index<packetlen_mouse) break;
	
	if (decode_mouse(&event,data)==0) {

	  /* Y */
	  mickey=event.dy;
	  mouse_y_mick += mickey;
	  while (mouse_y_mick > mouse_thresholdlim) {
	    mouse_y++;
	    mouse_y_mick -= mouse_thresholdlim;
	  }
	  while (mouse_y_mick < -mouse_thresholdlim) {
	    mouse_y--;
	    mouse_y_mick += mouse_thresholdlim;
	  }
	  if (mouse_y > mouse_lim_y2) mouse_y = mouse_lim_y2;
	  else if (mouse_y < mouse_lim_y1) mouse_y = mouse_lim_y1;
	  event.y=mouse_y;

	  /* X */
	  mickey = event.dx;
	  mouse_x_mick += mickey;
	  while (mouse_x_mick > mouse_thresholdlim) {
	    mouse_x++;
	    mouse_x_mick -= mouse_thresholdlim;
	  }
	  while (mouse_x_mick < -mouse_thresholdlim) {
	    mouse_x--;
	    mouse_x_mick += mouse_thresholdlim;
	  }
	  if (mouse_x > mouse_lim_x2) mouse_x = mouse_lim_x2;
	  else if (mouse_x < mouse_lim_x1) mouse_x = mouse_lim_x1;
	  event.x=mouse_x;

	  /* mouse handler */
	  if (mouse_handler!=NULL) mouse_handler(&event);
	}

	index-=packetlen_mouse;
	if (index!=0) memcpy(data,data+packetlen_mouse,index);
	break;

    } /* switch */
  } /* for */
}

/* [for SERIAL MOUSE] */
/* Well, this implementation is a little dispendious as we use	*/
/* two different tasks, one as serial receiver, the other as	*/
/* mouse-protocol decoder; this has been chosen in order to	*/
/* make any mouse-protocol easier. We only need to rewrite the	*/
/* the appropriate mouse packet interpreter			*/

/*
 * MG: rewritten to handle serial and PS/2 mouse and other mouses
 */

/* changed user interface */


int mouse_init(MOUSE_PARMS *parms)
{
  int status=0;
  TASK_MODEL *m;
  SOFT_TASK_MODEL base_m;
  MOUSE_PARMS mparms=BASE_MOUSE;

#ifdef __DEBUG_INIT__
  cprintf("mouse_init: START\n");
#endif

  if (mouse_pid != NIL) return -1;
  if (parms==NULL) parms=&mparms;

  /* default values */

#ifdef __DEBUG_INIT__
  cprintf("mouse_init: default values\n");
#endif

  if (parms->tm == (TASK_MODEL *)MOUSE_DEFAULT) {
    soft_task_default_model(base_m);
    soft_task_def_wcet(base_m,2000);
    soft_task_def_met(base_m,500);
    soft_task_def_period(base_m,8000);
    soft_task_def_system(base_m);
    soft_task_def_nokill(base_m);
    soft_task_def_aperiodic(base_m);
    m = (TASK_MODEL *)&base_m;
  }
  else
    m = parms->tm;

  /* try autodetect mouse() */
  if (parms->type==MOUSE_DEFAULT) {
    SERMOUSE_INFO *sinfoptr;

#ifdef __DEBUG_INIT__
    cprintf("mouse_init: try autodetecting\n");
#endif

    if (ps2mouse_present()) parms->type=PS2MOUSE;
    else {
      sinfoptr=sermouse_present();
      if (sinfoptr==NULL) return -2;
      parms->type=sinfoptr->type;
      parms->port=sinfoptr->port;
    }
  }

  mouse_settype(parms->type);
  switch(mousetype) {
  case PS2MOUSE:
    /* max PS/2 mouse rate: 40 event per second with 3 bytes per event */
    /* 120 bytes/sec -> 8 msec between activation (I hope)*/

#ifdef __DEBUG_INIT__
    cprintf("mouse_init: creating ps2 server task\n");
#endif

    if (parms->tm==(TASK_MODEL *)MOUSE_DEFAULT)
      soft_task_def_period(base_m,8000);
    mouse_pid=task_create("PS2Mouse",
			  ps2mouse_getserveraddr(),
                          m, NULL);
    if (mouse_pid==-1) return -3;

    //task_activate(mouse_pid);
    
#ifdef __DEBUG_INIT__
    cprintf("mouse_init: opening mouse\n");
#endif

    status=open_mouse((void*)&mouse_pid);
    break;

  case MSMOUSE:
  case MSPMOUSE:
  case MSPLRMOUSE:
  case BAREMOUSE:
  case MSCMOUSE:
  case SUNMOUSE:
  case LOGIMOUSE:
  case MMMOUSE:
    break;
  }

  if (status!=0) {
    task_kill(mouse_pid);
    mouse_pid=NIL;
    return -4;
  }

#ifdef __DEBUG_INIT__
  cputs("mouse_init: mouse activated\n");
#endif

  return 0;
}

void mouse_end(void)
{
  if (mouse_pid==NIL) return;
  task_kill(mouse_pid);
  mouse_pid=NIL;
  close_mouse();
}

/* MG: all the following procedure are modified */
/*     to use virtual operation                 */

void mouse_disable(void)
{
  disable_mouse();
}

void mouse_enable(void)
{
  enable_mouse();
}

void mouse_get(int *x,int *y,BYTE *button)
{
    if (x != NULL) *x = mouse_x;
    if (y != NULL) *y = mouse_y;
    if (button != NULL) *button = mouse_buttons;
}

void mouse_position(int x,int y)
{
    disable_mouse();
    mouse_x = x;
    mouse_y = y;
    enable_mouse();
}

void mouse_limit(int x1,int y1,int x2,int y2)
{
    disable_mouse();
    mouse_lim_x1 = x1;
    mouse_lim_y1 = y1;
    mouse_lim_x2 = x2;
    mouse_lim_y2 = y2;
    mouse_x = (x1+x2)/2;
    mouse_y = (y1+y2)/2;
    enable_mouse();
}

void mouse_threshold(unsigned t)
{
    disable_mouse();
    if (t < 1) t = 1;
    else if (t > 100) t = 100;
    mouse_thresholdlim = t;
    enable_mouse();
}

void mouse_hook(MOUSE_HANDLER h)
{
  /* changed to use the autocursor functions */
  if ((autocursormode&STATUSMASK)==DISABLE) {
    mouse_handler = h;
  } else {
    user_mouse_handler = h;
  }
}
