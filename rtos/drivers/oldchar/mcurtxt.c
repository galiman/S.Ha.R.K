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
 CVS :        $Id: mcurtxt.c,v 1.2 2005/02/25 11:04:03 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2005/02/25 11:04:03 $
 ------------

 Author:	Massimiliano Giorgi
 Date:	24/Giu/99

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

/*
 *
 * Auto Pointer management
 *
 */

#include <kernel/kern.h>
//#include <sys/sys.h>
//#include <sys/vm.h>
#include <drivers/mouse.h>
//#include <cons.h>
#include "_mouse.h"
#include <sem/sem/sem.h>

/* these are the actual mouse coordinates (defined into mouse.c) */
extern short int mouse_x;
extern short int mouse_y;

/* these variables are managed by this module but MUST be declared
 * into mouse.c to prevent implicit inclusion of this module
 * when a user link the mouse library
 */

extern MOUSE_HANDLER mouse_handler;
extern MOUSE_HANDLER user_mouse_handler;
extern int autocursormode;

/* a mutex semaphore */
static sem_t mutex=-1;

/* 
 *
 * autocursor mouse handler
 *
 */

/* >=0 hide cursor; <0 show cursor */
static int mouse_cursor_state=-1;

/* saved mouse_position */
static int  saved_x,saved_y;

static void dummy(int x,int y)
{}

static void (*show_cursor)(int x, int y)=dummy;
static void (*restore_cursor)(int x, int y)=dummy;

/* those are the 4 mouse handlers */
/* AUTOOFF    -> call the user handler with no mouse displayed */
/* WITHOUTSEM -> does not use a mutex semaphore */

/* with no flags */
static void autocursor_mouse_handler_1(MOUSE_EVT *event)
{
  if (user_mouse_handler!=NULL) user_mouse_handler(event);

  sem_wait(&mutex);
  if (mouse_cursor_state<0&&(event->x!=saved_x||event->y!=saved_y)) {
    restore_cursor(saved_x,saved_y);
    saved_x=event->x;
    saved_y=event->y;
    show_cursor(saved_x,saved_y);
  }
  sem_post(&mutex);
}

/* with WITHOUTSEM flag*/
static void autocursor_mouse_handler_2(MOUSE_EVT *event)
{
  if (user_mouse_handler!=NULL) user_mouse_handler(event);

  if (mouse_cursor_state<0&&(event->x!=saved_x||event->y!=saved_y)) {
    restore_cursor(saved_x,saved_y);
    saved_x=event->x;
    saved_y=event->y;
    show_cursor(saved_x,saved_y);
  }
}

/* with AUTOOFF flag*/
static void autocursor_mouse_handler_3(MOUSE_EVT *event)
{
  sem_wait(&mutex);

  if (mouse_cursor_state<0) {
      restore_cursor(saved_x,saved_y);
      saved_x=event->x;
      saved_y=event->y;
      if (user_mouse_handler!=NULL) user_mouse_handler(event);      
      show_cursor(saved_x,saved_y);
  } else
    if (user_mouse_handler!=NULL) user_mouse_handler(event);

  sem_post(&mutex);
}

/* with WITHOUTSEM & AUTOOFF flags */
static void autocursor_mouse_handler_4(MOUSE_EVT *event)
{
  if (mouse_cursor_state<0) {
      restore_cursor(saved_x,saved_y);
      saved_x=event->x;
      saved_y=event->y;
      if (user_mouse_handler!=NULL) user_mouse_handler(event);      
      show_cursor(saved_x,saved_y);
  } else
    if (user_mouse_handler!=NULL) user_mouse_handler(event);
}

/* --------------
 * TXT management
 * --------------
 */

/* text cursor shape */
#define DEFAULTTXTSHAPE 0x7700ffff
static DWORD saved_txtshape=DEFAULTTXTSHAPE;
static BYTE attr_andmask,attr_xormask; 
static BYTE c_andmask,c_xormask;
/* saved values */
static BYTE saved_attr,saved_c;

/* show txt cursor */
void show_txt_cursor(int x, int y)
{
  int attr,c;
  getc_xy(x,y,&saved_attr,&saved_c);
  attr=(saved_attr&attr_andmask)^attr_xormask;
  c=(saved_c&c_andmask)^c_xormask;
  putc_xy(x,y,attr,c);
}

/* restore txt cursor */
static void restore_txt_cursor(int x, int y)
{
  putc_xy(x,y,saved_attr,saved_c);
}

/* define text shape */
int mouse_txtshape(DWORD img)
{
  int cond;
  if (img==DEFAULT) img=DEFAULTTXTSHAPE;

  cond=(
	(autocursormode&STATUSMASK)==ENABLE
	&&
	(autocursormode&TXTCURSOR)==TXTCURSOR
       );
  
  if (cond)
    restore_txt_cursor(saved_x,saved_y);

  saved_txtshape=img;
  c_andmask=img&0xff;
  attr_andmask=(img&0xff00)>>8;
  c_xormask=(img&0xff0000)>>16;
  attr_xormask=(img&0xff000000)>>24;

  if (cond)
    show_txt_cursor(saved_x,saved_y);

  return 0;
}

/*
 * User interface to autocursor functions
 */


/* display the cursor */

#define MOUSE_ON() { \
  mouse_cursor_state--; \
  if (mouse_cursor_state==-1) { \
    saved_x=mouse_x; \
    saved_y=mouse_y; \
    show_cursor(saved_x,saved_y); \
  } \
}

static void mouse_on_sem(void)
{
  sem_wait(&mutex);
  MOUSE_ON();
  sem_post(&mutex);
}

static void mouse_on_nosem(void)
{
  MOUSE_ON();
}

void (*mouse_on)(void)=mouse_on_nosem;

/* hide the cursor */

#define MOUSE_OFF() { \
  mouse_cursor_state++; \
  if (mouse_cursor_state==0) restore_cursor(saved_x,saved_y); \
}

void mouse_off_sem(void)
{
  sem_wait(&mutex);
  MOUSE_OFF();
  sem_post(&mutex);
}

void mouse_off_nosem(void)
{
  MOUSE_OFF();
}

void (*mouse_off)(void)=mouse_off_nosem;

static MOUSE_HANDLER wich_handler(int cmd)
{
  if (((cmd&WITHOUTSEM)==WITHOUTSEM)&&((cmd&AUTOOFF)==AUTOOFF))
    return autocursor_mouse_handler_4;
  else if ((cmd&WITHOUTSEM)==WITHOUTSEM)
    return autocursor_mouse_handler_2;
  else if ((cmd&AUTOOFF)==AUTOOFF)
    return autocursor_mouse_handler_3;
  return autocursor_mouse_handler_1;
}

int mouse_txtcursor(int cmd)
{
  mouse_txtshape(saved_txtshape);  
  return _mouse_cursor_init(cmd|TXTCURSOR,show_txt_cursor,restore_txt_cursor);
}

/**/

int _mouse_cursor_init(int cmd,
			void(*my_show_cursor)(int,int),
			void(*my_restore_cursor)(int,int)
			)
{
  if (mutex==-1) {
    if (sem_init(&mutex,0,1) == -1)
      return -1;
  }

  switch (cmd&STATUSMASK) {
    case DISABLE:
      mouse_handler=user_mouse_handler;
      restore_cursor(saved_x,saved_y);
      show_cursor=dummy;
      restore_cursor=dummy;
      break;
    case ENABLE:
      restore_cursor(saved_x,saved_y);
      restore_cursor=my_restore_cursor;
      show_cursor=my_show_cursor;      
      if ((autocursormode&STATUSMASK)==DISABLE) {
	user_mouse_handler=mouse_handler;
	mouse_handler=wich_handler(cmd);
      }
      mouse_cursor_state=0;
      break;
    default:
      return -1;
      
  }  
  autocursormode=cmd;
  
  if ((autocursormode&STATUSMASK)==ENABLE) {
    if ((cmd&WITHOUTSEM)==WITHOUTSEM) {
      mouse_on=mouse_on_nosem;
      mouse_off=mouse_off_nosem;
    } else {
      mouse_on=mouse_on_sem;
      mouse_off=mouse_off_sem;
    }
  }
  
  return 0;
}

void _mouse_getsavedposition(int *xptr, int *yptr)
{
  *xptr=saved_x;
  *yptr=saved_y;
}
