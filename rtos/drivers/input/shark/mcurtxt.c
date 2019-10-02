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
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
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

/*
 *
 * Auto Pointer management
 *
 */

#include <kernel/kern.h>
#include <kernel/int_sem.h>

#include "../include/drivers/shark_input26.h"
#include "../include/drivers/shark_mouse26.h"

//#define CURTXT_DEBUG

/* these variables are managed by this module but MUST be declared
 * into shark_mouse.c to prevent implicit inclusion of this module
 * when a user link the mouse library
 */
extern MOUSE_HANDLER show_mouse_handler;
extern MOUSE_HANDLER user_mouse_handler;

/* a mutex semaphore */
static int mcur_mutex_init = 0;
static internal_sem_t mutex;

/* Setup function */
int _mouse_cursor_init(int cmd, void(*my_show_cursor)(int, int), void(*my_restore_cursor)(int, int));

/* 
 *
 * autocursor mouse handler
 *
 */

/* >=0 hide cursor; <0 show cursor */
static int mouse_cursor_state = 0;

/* mouse status */
int autocursormode = 0;

/* saved mouse_position */
static int  saved_x, saved_y;

static void dummy(int x,int y){}

static void (*show_cursor)(int x, int y) = dummy;
static void (*restore_cursor)(int x, int y) = dummy;

/* those are the 4 mouse handlers */
/* AUTOOFF    -> call the user handler with no mouse displayed */
/* WITHOUTSEM -> does not use a mutex semaphore */

/* with no flags */
static void autocursor_mouse_handler_1(MOUSE_EVT *event)
{
#ifdef CURTXT_DEBUG
	printk(KERN_DEBUG "mcurtxt.c: autocursor_mouse_handler_1\n");
#endif

	if (user_mouse_handler != NULL)
		user_mouse_handler(event);

	internal_sem_wait(&mutex);
	if ( (mouse_cursor_state < 0) && (event->x != saved_x || event->y != saved_y)) {
		restore_cursor(saved_x, saved_y);
		saved_x = event->x;
		saved_y = event->y;
		show_cursor(saved_x, saved_y);
	}
	internal_sem_post(&mutex);
}

/* with WITHOUTSEM flag*/
static void autocursor_mouse_handler_2(MOUSE_EVT *event)
{
#ifdef CURTXT_DEBUG
	printk(KERN_DEBUG "mcurtxt.c: autocursor_mouse_handler_2\n");
#endif
	if (user_mouse_handler != NULL)
		user_mouse_handler(event);

	if ( (mouse_cursor_state < 0) && (event->x != saved_x || event->y != saved_y)) {
		restore_cursor(saved_x, saved_y);
		saved_x = event->x;
		saved_y = event->y;
		show_cursor(saved_x, saved_y);
	}
}

/* with AUTOOFF flag*/
static void autocursor_mouse_handler_3(MOUSE_EVT *event)
{
#ifdef CURTXT_DEBUG
	printk(KERN_DEBUG "mcurtxt.c: autocursor_mouse_handler_3\n");
#endif
	internal_sem_wait(&mutex);

	if (mouse_cursor_state < 0) {
		restore_cursor(saved_x, saved_y);
		saved_x = event->x;
		saved_y = event->y;
		if (user_mouse_handler != NULL)
			user_mouse_handler(event);      
		show_cursor(saved_x, saved_y);
	} else if (user_mouse_handler != NULL)
		user_mouse_handler(event);

	internal_sem_post(&mutex);
}

/* with WITHOUTSEM & AUTOOFF flags */
static void autocursor_mouse_handler_4(MOUSE_EVT *event)
{
#ifdef CURTXT_DEBUG
	printk(KERN_DEBUG "mcurtxt.c: autocursor_mouse_handler_4\n");
#endif
	if (mouse_cursor_state < 0) {
		restore_cursor(saved_x, saved_y);
		saved_x = event->x;
		saved_y = event->y;
		if (user_mouse_handler != NULL)
			user_mouse_handler(event);      
		show_cursor(saved_x, saved_y);
	} else if (user_mouse_handler != NULL)
		user_mouse_handler(event);
}

/* --------------
 * TXT management
 * --------------
 */

/* text cursor shape */
#define DEFAULTTXTSHAPE 0x7700ffff
static DWORD saved_txtshape = DEFAULTTXTSHAPE;
static BYTE attr_andmask, attr_xormask; 
static BYTE c_andmask, c_xormask;

/* saved values */
static BYTE saved_attr, saved_c;

/* show txt cursor */
void show_txt_cursor(int x, int y)
{
	int attr,c;

	getc_xy(x, y, &saved_attr, &saved_c);
	attr = (saved_attr & attr_andmask) ^ attr_xormask;
	c = (saved_c & c_andmask) ^ c_xormask;
	putc_xy(x, y, attr, c);
}

/* restore txt cursor */
static void restore_txt_cursor(int x, int y)
{
	putc_xy(x, y, saved_attr, saved_c);
}

/* define text shape */
int mouse_txtshape(DWORD img)
{
	int cond;

	if (img == DEFAULT)
		img = DEFAULTTXTSHAPE;

	cond = ( (autocursormode & STATUSMASK) == ENABLE && (autocursormode & TXTCURSOR) == TXTCURSOR );
  
	if (cond)
		restore_txt_cursor(saved_x, saved_y);

	saved_txtshape = img;
	c_andmask = img & 0xff;
	attr_andmask = (img & 0xff00) >> 8;
	c_xormask = (img & 0xff0000) >> 16;
	attr_xormask = (img & 0xff000000) >> 24;
	
#ifdef CURTXT_DEBUG
	printk(KERN_DEBUG "MouseTxt_Shape: %x %x %x %x\n", c_andmask, attr_andmask, c_xormask, attr_xormask);
#endif
	if (cond)
		show_txt_cursor(saved_x, saved_y);

	return 0;
}

/*
 * User interface to autocursor functions
 */

/* display the cursor */
#define MOUSE_ON() { \
	int unused; \
	unsigned long lunused; \
	mouse_cursor_state--; \
	if (mouse_cursor_state == -1) { \
		mouse_getposition(&saved_x, &saved_y, &unused, &lunused); \
		show_cursor(saved_x, saved_y); \
	} \
}

static void mouse_on_sem(void)
{
	internal_sem_wait(&mutex);
	MOUSE_ON();
	internal_sem_post(&mutex);
}

static void mouse_on_nosem(void)
{
	MOUSE_ON();
}

void (*mouse_on)(void)=mouse_on_nosem;

/* hide the cursor */
#define MOUSE_OFF() { \
	mouse_cursor_state++; \
	if (mouse_cursor_state == 0) \
		restore_cursor(saved_x, saved_y); \
}

void mouse_off_sem(void)
{
	internal_sem_wait(&mutex);
	MOUSE_OFF();
	internal_sem_post(&mutex);
}

void mouse_off_nosem(void)
{
	MOUSE_OFF();
}

void (*mouse_off)(void) = mouse_off_nosem;

static MOUSE_HANDLER wich_handler(int cmd)
{
	if ( ((cmd & WITHOUTSEM) == WITHOUTSEM) && ((cmd & AUTOOFF) == AUTOOFF) )
		return autocursor_mouse_handler_4;
	if ((cmd & WITHOUTSEM) == WITHOUTSEM)
		return autocursor_mouse_handler_2;
	if ((cmd & AUTOOFF) == AUTOOFF)
		return autocursor_mouse_handler_3;
	return autocursor_mouse_handler_1;
}

int mouse_txtcursor(int cmd)
{
	mouse_txtshape(saved_txtshape);  
	return _mouse_cursor_init(cmd|TXTCURSOR, show_txt_cursor, restore_txt_cursor);
}

/* Generic functions, both for text & graphics mode */

int _mouse_cursor_init(int cmd, void(*my_show_cursor)(int, int), void(*my_restore_cursor)(int, int))
{
	if (mcur_mutex_init == 0) {
		internal_sem_init(&mutex, 1);
		mcur_mutex_init = 1;
	}

#ifdef CURTXT_DEBUG
	printk(KERN_DEBUG "mouse_cursor_init: command %x\n", cmd);
#endif

	switch (cmd & STATUSMASK) {
		case DISABLE:
			//show_mouse_handler = user_mouse_handler;
			show_mouse_handler = NULL;
			restore_cursor(saved_x, saved_y);
			show_cursor = dummy;
			restore_cursor = dummy;
			mouse_cursor_state = 0;
			break;
		case ENABLE:
			restore_cursor(saved_x, saved_y);
			restore_cursor = my_restore_cursor;
			show_cursor = my_show_cursor;      
			if ((autocursormode & STATUSMASK) == DISABLE) {
				//user_mouse_handler = show_mouse_handler;
				show_mouse_handler = wich_handler(cmd);
			}
			mouse_cursor_state = -1;
			break;
		default:
			return -1;
	}  

	autocursormode = cmd;
  
	if ((autocursormode & STATUSMASK) == ENABLE) {
		if ((cmd & WITHOUTSEM) == WITHOUTSEM) {
			mouse_on = mouse_on_nosem;
			mouse_off = mouse_off_nosem;
		} else {
			mouse_on = mouse_on_sem;
			mouse_off = mouse_off_sem;
		}
	}
  
	return 0;
}

void _mouse_cursor_getposition(int *xptr, int *yptr)
{
	*xptr = saved_x;
	*yptr = saved_y;
}
