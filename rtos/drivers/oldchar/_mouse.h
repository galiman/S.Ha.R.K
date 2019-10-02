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
 CVS :        $Id: _mouse.h,v 1.1 2003/03/24 10:54:16 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 10:54:16 $
 ------------

 _mouse.h

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

#ifndef ___MOUSE_H__
#define ___MOUSE_H__
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* "virtual operations" on a mouse */
struct mouse_operations {
  char *name;             /* name! */
  char *desc;             /* a descriptive line */  

  BYTE proto[4];          
  int  packetlen;         /* lenght of a packet */ 
  int  howmany;           /* howmany bytes to read at a time (unused) */
  int  getextra;          /* there is a extra byte (unused) */
  int  absolute;          /* absolute pointing devide (unused) */
  int  dummy;             /* (unused) */

  int  (*open)(void*);    /* open comunication device */
  void (*close)(void);    /* close comunication device */
  void (*wait)(void);     /* wait to have a BYTE (blocking operation)*/
  int  (*get)(BYTE *ptr); /* get last BYTE */
  void (*enable)(void);   /* enable interface */ 
  void (*disable)(void);  /* disable interface */

  /* this function decode the mouse packet into a MOUSE_EVT structure */
  int  (*decode)(MOUSE_EVT *evt,unsigned char *data);
};

/* to virtualize interface beetwen a mouse device and a mouse protocol */
extern int    mousetype;
extern struct mouse_operations vmouse[];

/* functions and variables (to use virtual operation)*/
extern int  (*open_mouse)(void*);     /* open comunication device */
extern void (*close_mouse)(void);     /* close comunication device */
extern void (*wait_mouse)(void);      /* wait to have a BYTE (blocking operation)*/
extern int  (*get_mouse)(BYTE *data); /* get last BYTE */
extern void (*enable_mouse)(void);    /* enable interface */
extern void (*disable_mouse)(void);   /* disable interface */

extern char *name_mouse;
extern char *desc_mouse;
extern BYTE *proto_mouse;
extern int  packetlen_mouse;

/* used by mouse protocol task */
extern short int mouse_lim_x1;
extern short int mouse_lim_y1;
extern short int mouse_lim_x2;
extern short int mouse_lim_y2;
extern short int mouse_x;
extern short int mouse_x_mick;
extern short int mouse_y;
extern short int mouse_y_mick;
extern short int mouse_buttons;
extern short int mouse_thresholdlim;
extern MOUSE_HANDLER mouse_handler;

/* a generic mouse server */
extern TASK generalmouse_server(void);

/*
 * mouse autocursor variables
 */

/* mask to extrac the status from autocursormode */
#define STATUSMASK 0x0f

/* flags for autocursormode (there are some other flags into mouse.h) */
#define GRXCURSOR 0x100
#define TXTCURSOR 0x200

/* cursor status and flags */
extern int autocursormode;

/* this functions is used to enable/disable the cursor */
/* (it is here to be used by the mcurgrx.c  module) */
int _mouse_cursor_init(int cmd,
		       void(*my_show_cursor)(int,int),
		       void(*my_restore_cursor)(int,int)
		       );
		       
/* this is used by mcurgrx.c to know the last mouse position */
/* (to not declare saved_x & saved_y public) */
void _mouse_getsavedposition(int *xptr, int *yptr);

__END_DECLS
#endif




