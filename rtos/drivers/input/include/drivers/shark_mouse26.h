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
 *
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

/* Glue Layer Header Linux Input Driver*/

#ifndef __SHARK_MOUSE26_H__
#define __SHARK_MOUSE26_H__

#include <kernel/const.h>
#include <kernel/model.h>

#ifdef __cplusplus
extern "C" {
#endif

/* mouse buttons constant */  
#define MOUSE_RBUTTON	0x02
#define MOUSE_CBUTTON	0x04
#define MOUSE_LBUTTON	0x01

/* the mouse event struct */
typedef struct {
	int x, y, z;		/* mouse position */
	int dx, dy, dz;		/* distance covered by mouse */ 
	unsigned long buttons;	/* buttons flags */
} MOUSE_EVT;

/* macros to test mouse buttons */
#define isLeftButton(b)		((b) & MOUSE_LBUTTON)
#define isRightButton(b)	((b) & MOUSE_RBUTTON)
#define isCentralButton(b)	((b) & MOUSE_CBUTTON)

/*
 * mouse initialization
 */
 
/* the MOUSE_PARMS structure used by MOUSE26_init() */
typedef struct mouse_parms {
	TASK_MODEL *tm;
	int x0, y0, z0;
	int xmin;
	int ymin;
	int xmax;
	int ymax;
	int threshold;
} MOUSE_PARMS;

#define MOUSE_DEFAULT (DWORD)-1

/* the default values for the MOUSE_PARMS structure */
#define BASE_MOUSE {(TASK_MODEL *)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT, \
		    (int)MOUSE_DEFAULT}

/* to change the MOUSE_PARMS struct */
#define mouse_default_parms(s)	(s).tm = (TASK_MODEL *)MOUSE_DEFAULT, \
				(s).x0 = (int)MOUSE_DEFAULT, \
				(s).y0 = (int)MOUSE_DEFAULT, \
				(s).z0 = (int)MOUSE_DEFAULT, \
				(s).xmin = (int)MOUSE_DEFAULT, \
				(s).xmax = (int)MOUSE_DEFAULT, \
				(s).ymin = (int)MOUSE_DEFAULT, \
				(s).ymax = (int)MOUSE_DEFAULT, \
				(s).threshold = (int)MOUSE_DEFAULT

#define mouse_def_task(s,m)	(s).tm = (TASK_MODEL *)(m)
#define mouse_def_threshold(s,v)	(s).threshold = (v)
#define mouse_def_xmin(s,v)	(s).xmin = (v)
#define mouse_def_ymin(s,v)	(s).ymin = (v)
#define mouse_def_xmax(s,v)	(s).xmax = (v)
#define mouse_def_ymax(s,v)	(s).ymax = (v)
#define mouse_def_x0(s,v)	(s).x0 = (v)
#define mouse_def_y0(s,v)	(s).y0 = (v)
#define mouse_def_z0(s,v)	(s).y0 = (v)

/* user mouse handler */
typedef void (*MOUSE_HANDLER)(MOUSE_EVT*);

/*
 * user mouse interface
 */
int  MOUSE26_installed(void);
int  MOUSE26_init(MOUSE_PARMS *s);
int  MOUSE26_close(void);
  
void mouse_enable(void);
void mouse_disable(void);
void mouse_getposition(int *x, int *y, int *z, unsigned long *buttons);
void mouse_setposition(int  x, int  y, int z);
void mouse_getlimits(int *xmin, int *ymin, int *xmax, int *ymax);
int  mouse_setlimits(int  xmin, int  ymin, int  xmax, int  ymax);
int  mouse_getthreshold(void);
int  mouse_setthreshold(int t);
void mouse_hook(MOUSE_HANDLER h);


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

/* mask to extrac the status from autocursormode */
#define STATUSMASK 0x0f

/* flags for autocursormode (there are some other flags into mouse.h) */
#define GRXCURSOR 0x100
#define TXTCURSOR 0x200

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

#define mouse_grxlimits(dimx,dimy) mouse_setlimits(\
	XMINLIMIT(dimx,dimy), \
	YMINLIMIT(dimx,dimy), \
	XMAXLIMIT(dimx,dimy), \
	YMAXLIMIT(dimx,dimy)  \
)

/* these are used to select the mouse shape */
int mouse_txtshape(DWORD img);
int mouse_grxshape(BYTE *shape, BYTE *mask, int bpp_in);

/* enable/disable mouse pointer */
/* (return <0 on error) */
/* (for the cmd parameter see above) */
int mouse_grxcursor(int cmd, int bpp);
int mouse_txtcursor(int cmd);

/* mouse on/off (or show/hide) */
void (*mouse_on)(void);
void (*mouse_off)(void);

#ifdef __cplusplus
};
#endif

#endif

