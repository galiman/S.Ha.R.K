/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
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

//#define __MOUSE_DEBUG__
#define MOUSE_TASK

#include <kernel/kern.h>

#include "../include/drivers/shark_input26.h"
#include "../include/drivers/shark_mouse26.h"

/* Devices */
extern int psmouse_init(void);
extern int psmouse_exit(void);

/* Handlers */
extern int  mouse_init(void);
extern void mouse_exit(void);

/* Functions */
extern int mouse_get(int *dx, int *dy, int *dz, unsigned long *buttons);

/* Mouse driver currently installed/actived */
static int mouse_installed = FALSE;
static int mouse_enabled = FALSE;

static int mouse_xmin_tick;
static int mouse_ymin_tick;
static int mouse_xmax_tick;
static int mouse_ymax_tick;

#define MOUSE_DEF_THRESHOLD 10
static int mouse_threshold;

static int           mouse_x = 0;
static int           mouse_y = 0;
static int           mouse_x_tick = 0;
static int           mouse_y_tick = 0;
static int           mouse_z =  0;
static unsigned long mouse_buttons = 0;

MOUSE_HANDLER user_mouse_handler = NULL;
MOUSE_HANDLER show_mouse_handler = NULL;

#ifdef MOUSE_TASK
/* mouse task PID */
static PID mousepid = NIL;
#else
void mouseProc(void);
#endif


/*
 * Start mouseProc Task
 */
void shark_mouse_exec(void)
{
#ifdef MOUSE_TASK
	task_activate(mousepid);
#else
	mouseProc();
#endif
}

#ifdef MOUSE_TASK
TASK mouseProc(void)
#else
void mouseProc(void)
#endif
{
	static MOUSE_EVT ev;
	unsigned long dbuttons;
	int dx, dy, dz;
	int res;
	
#ifdef MOUSE_TASK
	while (1) {
#endif
		if (mouse_enabled) {
			res = mouse_get(&dx, &dy, &dz, &dbuttons);
			if (res >= 0) {
				mouse_x_tick += dx;
				if (mouse_x_tick < mouse_xmin_tick)
					mouse_x_tick = mouse_xmin_tick;
				if (mouse_x_tick > mouse_xmax_tick)
					mouse_x_tick = mouse_xmax_tick;
				mouse_x = (mouse_x_tick + (mouse_threshold/2)) / mouse_threshold;
				ev.x  = mouse_x;
				ev.dx = dx;

				mouse_y_tick -= dy;
				if (mouse_y_tick < mouse_ymin_tick)
					mouse_y_tick = mouse_ymin_tick;
				if (mouse_y_tick > mouse_ymax_tick)
					mouse_y_tick = mouse_ymax_tick;
				mouse_y = (mouse_y_tick + (mouse_threshold/2)) / mouse_threshold;
				ev.y  = mouse_y;
				ev.dy = dy;

				mouse_z += dz;
				ev.z  = mouse_z;
				ev.dz = dz;

				mouse_buttons = dbuttons;
				ev.buttons  = mouse_buttons;
#ifdef __MOUSE_DEBUG__
				printk(KERN_DEBUG "shark_mouse.c: delta ( %3d %3d %3d - %6x) -> ( %3d %3d %3d - %6x)\n", 
				       dx, dy, dz, (int)dbuttons, mouse_x, mouse_y, mouse_z, (int)mouse_buttons);
#endif
				/* mouse handler */
				if (show_mouse_handler != NULL)
					show_mouse_handler(&ev);
				else if (user_mouse_handler != NULL) 
					user_mouse_handler(&ev);
			}
		}
#ifdef MOUSE_TASK
		task_endcycle();
	}
#endif
}

/**** Start User Functions ****/

void mouse_enable(void)
{
	mouse_enabled = TRUE;	/* TODO */
}

void mouse_disable(void)
{
	mouse_enabled = FALSE;	/* TODO */
}

void mouse_getposition(int *x,int *y,int *z, unsigned long *buttons)
{
	if (x)
		*x = mouse_x;
	if (y)
		*y = mouse_y;
	if (z)
		*z = mouse_z;
	if (buttons)
		*buttons = mouse_buttons;
}

void mouse_setposition(int x,int y, int z)
{
	mouse_enabled = FALSE;
	
	mouse_x_tick = x * mouse_threshold;
	if (mouse_x_tick < mouse_xmin_tick)
		mouse_x_tick = mouse_xmin_tick;
	if (mouse_x_tick > mouse_xmax_tick)
		mouse_x_tick = mouse_xmax_tick;
	mouse_x = (mouse_x_tick + (mouse_threshold/2)) / mouse_threshold;
	
	mouse_y_tick = y * mouse_threshold;
	if (mouse_y_tick < mouse_ymin_tick)
		mouse_y_tick = mouse_ymin_tick;
	if (mouse_y_tick > mouse_ymax_tick)
		mouse_y_tick = mouse_ymax_tick;
	mouse_y = (mouse_y_tick + (mouse_threshold/2)) / mouse_threshold;
	
	mouse_z = z;

	mouse_enabled = TRUE;
}

void mouse_getlimits(int *xmin, int *ymin, int *xmax, int *ymax)
{
	if (xmin)
		*xmin = mouse_xmin_tick / mouse_threshold;
	if (ymin)
		*ymin = mouse_ymin_tick / mouse_threshold;
	if (xmax)
		*xmax = mouse_xmax_tick / mouse_threshold;
	if (ymax)
		*ymax = mouse_ymax_tick / mouse_threshold;
}

int mouse_setlimits(int xmin, int ymin, int xmax, int ymax)
{
	if ((xmin < 0) && (ymin < 0) && (xmax < xmin) && (ymax < ymin))
		return -1;

	mouse_xmin_tick = xmin * mouse_threshold;
	mouse_ymin_tick = ymin * mouse_threshold;
	mouse_xmax_tick = xmax * mouse_threshold;
	mouse_ymax_tick = ymax * mouse_threshold;
	
	return 0;
}

void mouse_hook(MOUSE_HANDLER h)
{
	user_mouse_handler = h;
}

int mouse_getthreshold(void)
{
	return mouse_threshold;
}

int mouse_setthreshold(int t)
{
	mouse_enabled = FALSE;

	if ((t>0) && (t<=200)) {
		mouse_xmin_tick = (mouse_xmin_tick * t) / mouse_threshold;
		mouse_ymin_tick = (mouse_ymin_tick * t) / mouse_threshold;
		mouse_xmax_tick = (mouse_xmax_tick * t) / mouse_threshold;
		mouse_ymax_tick = (mouse_ymax_tick * t) / mouse_threshold;
		mouse_threshold = t;
		return 0;
	} else {
		return -1;
	}

	mouse_enabled = TRUE;
}

/**** End User Functions ****/

/* Init the Linux Speaker Driver */
int MOUSE26_installed(void)
{
	return mouse_installed;
}

int MOUSE26_init(MOUSE_PARMS *s)
{
	MOUSE_PARMS mparms = BASE_MOUSE;
	int status = 0;

#ifdef MOUSE_TASK
	TASK_MODEL *m;
	SOFT_TASK_MODEL base_m;
#endif

	if (mouse_installed == TRUE) return 0;

	/* if a NULL is passed */
	if (s == NULL)
		s = &mparms;

	/* set mouse threshold */
	if ((s->threshold == (int) MOUSE_DEFAULT) || (s->threshold == 0))
		mouse_threshold = MOUSE_DEF_THRESHOLD;
	else
		mouse_threshold = s->threshold;

	/* set mouse limits */
	if ((s->xmin == (int) MOUSE_DEFAULT) || (s->xmin < 0))
		mouse_xmin_tick = 0;
	else
		mouse_xmin_tick = s->xmin * mouse_threshold;
		
	if ((s->ymin == (int) MOUSE_DEFAULT) || (s->ymin < 0))
		mouse_ymin_tick = 0;
	else
		mouse_ymin_tick = s->ymin * mouse_threshold;
		
	if ((s->xmax == (int) MOUSE_DEFAULT) || ((s->xmax * mouse_threshold) < mouse_xmin_tick))
		mouse_xmax_tick = 79 * mouse_threshold;
	else
		mouse_xmax_tick = s->xmax * mouse_threshold;
		
	if ((s->ymax == (int) MOUSE_DEFAULT) || ((s->ymax * mouse_threshold) < mouse_ymin_tick))
		mouse_ymax_tick = 24 * mouse_threshold;
	else
		mouse_ymax_tick = s->ymax * mouse_threshold;

	/* set initial mouse position */
	if (s->x0 == (int) MOUSE_DEFAULT)
		mouse_x_tick = mouse_x = 0;
	else {
		mouse_x_tick = s->x0 * mouse_threshold;
		mouse_x = mouse_x_tick / mouse_threshold;
	}

	if (s->y0 == (int) MOUSE_DEFAULT)
		mouse_y_tick = mouse_y = 0;
	else {
		mouse_y_tick = s->y0 * mouse_threshold;
		mouse_y = mouse_y_tick / mouse_threshold;
	}

	if (s->z0 == (int) MOUSE_DEFAULT)
		mouse_z = 0;
	else
		mouse_z = s->z0;

#ifdef MOUSE_TASK
	if (s->tm == (TASK_MODEL *)MOUSE_DEFAULT) {
		soft_task_default_model(base_m);
		soft_task_def_wcet(base_m,2000);
		soft_task_def_met(base_m,500);
		soft_task_def_period(base_m,8000);
		soft_task_def_system(base_m);
		soft_task_def_nokill (base_m);
		soft_task_def_aperiodic(base_m);
		m = (TASK_MODEL *)&base_m;
	} else
		m = s->tm;

	mousepid = task_create ("MouseTask", mouseProc, m, NULL);
	if (mousepid == -1) {
		return -1;
	}
#endif

	if (INPUT26_installed() == FALSE) {
		status = INPUT26_init();
		if (status) {
		
			printk(KERN_ERR "shark_mouse.c: Unable to open Input SubSystem.\n");
			return -1;
		}
	}

	status = psmouse_init();
	if (status) {
		printk(KERN_ERR "shark_mouse.c: PsMouse_Init return: %d\n", status);
		return -1;
	}

	status = mouse_init();
	if (status) {
		printk(KERN_ERR "shark_mouse.c: Mouse_Init return: %d\n", status);
		return -1;
	}
	
	mouse_installed = TRUE;
	mouse_enabled = TRUE;
	
	return status;
}

int MOUSE26_close() 
{
#ifdef MOUSE_TASK
	int free;
	SYS_FLAGS f;
#endif

	if (!mouse_installed)
		return -1;

	mouse_enabled = FALSE;
	mouse_exit();
	psmouse_exit();

#ifdef MOUSE_TASK
	f = kern_fsave();
	free = (proc_table[mousepid].status == FREE);
	kern_frestore(f);
#ifdef __MOUSE_DEBUG__
	printk(KERN_DEBUG "shark_mouse.c: MouseTask is %s.\n", free ? "killed" : "alive");
#endif
	if (free)
		task_kill (mousepid);
#endif

	mouse_installed = FALSE;
	
	return 0;
}

