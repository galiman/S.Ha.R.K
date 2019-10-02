/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
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
 CVS :        $Id: fly.c,v 1.11 2005/02/25 11:10:46 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.11 $
 Last update: $Date: 2005/02/25 11:10:46 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai and Giorgio Buttazzo
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

/*--------------------------------------------------------------*/
/*              SIMULATION OF RANDOM FLIES                      */
/*--------------------------------------------------------------*/

#include <kernel/kern.h>
#include <sem/sem/sem.h>
#include <stdlib.h>
#include <math.h>

#include <drivers/shark_keyb26.h>
#include <drivers/shark_fb26.h>

#define YMENU    10             /* menu level                   */
#define XMIN     50
#define XMAX     600
#define YMIN     100
#define YMAX     450
#define VEL      5              /* linear velocity (def. = 5)   */
#define ANG      30             /* angolo massimo sterzata (30) */
#define D        3              /* raggio mosca                 */
#undef ESC
#define ESC      27             /* ASCII code of ESCAPE key     */
#define MAX_P    35             /* max number of flies          */
#define FLYGROUP 1

double  tick = 1.0;             /* system tick = 1 ms           */
int     fly_period = 40000;     /* task period                  */
int     fly_wcet = 1000;        /* task wcet                    */
PID     pid;

sem_t   grx_mutex;

/*--------------------------------------------------------------*/

void    draw_fly(int x, int y, int c)
{
	grx_disc(x, y, D, c);
}

/******************************************************************/

TASK    fly(void *arg)
{
int     x, y;
int     ox, oy;
int     dx, dy, da;
int     teta, col,red;
int     outx, outy;
double  r;
int     i = (int)arg;

	x = ox = (XMIN+XMAX)/2;
	y = oy = (YMIN+YMAX)/2;
	teta = 0;
	red = 100+10*i;
	if (red > 255) red = 255;
	col = rgb16(red,0,50); /* colore fly           */

	while (1) {

		da = rand()%(2*ANG) - ANG;      /*  da = [-ANG,ANG] */
		teta += da;

		if (teta > 360) teta -= 360;
		if (teta < 0) teta += 360;
		r = (double)teta * PI / 180.;

		dx = (float)(VEL * cos(r));
		dy = (float)(VEL * sin(r));
		x += dx;
		y += dy;

		outx = (x >= XMAX) || (x <= XMIN);
		outy = (y >= YMAX) || (y <= YMIN);

		if (outx || outy) {
			x = x - dx;
			y = y - dy;
			if (outx) teta = 180 - teta;
			if (outy) teta = -teta;
			if (teta > 360) teta -= 360;
			if (teta < 0) teta += 360;
			r = (double)teta * PI / 180.;

			dx = (float)(VEL * cos(r));
			dy = (float)(VEL * sin(r));

			x += dx;
			y += dy;
		}

		sem_wait(&grx_mutex);
		  draw_fly(ox, oy, 0);
		  draw_fly(x, y, col);
		  ox = x; oy = y;
                sem_post(&grx_mutex);

		task_endcycle();
	}
}

/****************************** MAIN ******************************/

int main(int argc, char **argv)
{
    HARD_TASK_MODEL m;

    char c;             /* character from keyboard      */
    int  i = 0;         /* number of tasks created      */
    TIME seme;          /* used to init the random seed */

    /* Init the mutex */
    sem_init(&grx_mutex,0,1);

    /* The scenario */
    grx_rect(XMIN-D-1, YMIN-D-1, XMAX+D+1, YMAX+D+1, rgb16(255,255,255));
    grx_text("Simulation of Random Flies", XMIN, YMENU+10, rgb16(255,255,255), 0);
    grx_text("SPACE create a fly"        , XMIN, YMENU+20, rgb16(255,255,255), 0);
    grx_text("ESC   exit to DOS"         , XMIN, YMENU+30, rgb16(255,255,255), 0);

    /* The program waits a space to create a fly */
    c = keyb_getch(BLOCK);

    /* randomize!!!! */
    seme = sys_gettime(NULL);
    srand(seme);

    do {
	if ((c == ' ') && (i < MAX_P)) {
	    hard_task_default_model(m);
	    hard_task_def_ctrl_jet (m);
	    hard_task_def_arg      (m, (void *)i);
	    hard_task_def_wcet     (m, fly_wcet);
	    hard_task_def_mit      (m, fly_period);
	    hard_task_def_group    (m, FLYGROUP);
	    hard_task_def_usemath  (m);
	    pid = task_create("fly", fly, &m, NULL);
	    if (pid == NIL) {
	      sys_shutdown_message("Could not create task <fly>");
	      exit(1);
	    }
	    task_activate(pid);
	    i++;
	}
	c = keyb_getch(BLOCK);

    } while (c != ESC);

    exit(0);

    return 0;

}

/*--------------------------------------------------------------*/
