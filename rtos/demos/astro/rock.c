/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it
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

#include "asteroid.h"

PID pid_RC;
int nrock;
int kill_rock;
rock_pos rocks[ROCK_NMAX];

void draw_rock(int x, int y, int r, int c, int m)
{
	sem_wait(&mx_grf);
	if (m==0)
		grx_disc(x, y, r, c);
	else
		grx_circle(x, y, r, c);
	sem_post(&mx_grf);
}

TASK rock(rock_ini* ri)
{
	int   x, y, r;  /* rock graphic position */
	int   ox,  oy;  /* rock old position */
	int   x0,  y0;  /* rock initial position */
	float vx,  vy;  /* rock speed */
	float vxo, vyo; /* rock initial speed */
	float ty,  tx;  
	float dt;       
	float rand_ang;
	int   rand_side;
	int   ab, ax, ay, i, l;

	x = y = x0 = y0 = ox = oy = vx = vy = ab = 0;

	i = ri->i;
	r = ri->r;

	rand_ang = ((rand()%120) - 60) * PI / 180.;

	sem_wait(&mx_st_scr);
	l = score / 50;
	sem_post(&mx_st_scr);

	sem_wait(&mx_mat);
	vxo = - (ROCK_VEL + l) * sin(rand_ang);
	vyo = - (ROCK_VEL + l) * cos(rand_ang);
	sem_post(&mx_mat);

#ifdef ASTRO_MOVE
	rand_side = rand()%4;
#else
	rand_side = 1;
#endif

	if (rand_side == 0) { // Bottom
		vx =  vxo;
		vy =  vyo;
	}
	if (rand_side == 1) { // Top
		vx = -vxo;
		vy = -vyo;
	}
	if (rand_side == 2) { // Right
		vx =  vyo;
		vy = -vxo;
	}
	if (rand_side == 3) { // Left
		vx = -vyo;
		vy =  vxo;
	}

	if ( (ri->x == 0) && (ri->y == 0) ) {
		if (rand_side < 2) { // Bottom or Top
			x = ox = x0 = GB_XMIN + (GB_XMAX-GB_XMIN)/4 + (rand()%((GB_XMAX-GB_XMIN)/2));
			if (rand_side == 0) { // Bottom
				y = oy = y0 = GB_YMAX - (r+1);
			}
			if (rand_side == 1) { // Top
				y = oy = y0 = GB_YMIN + (r+1);
			}
		} else {
			y = oy = y0 = GB_YMIN + (GB_YMAX-GB_YMIN)/4 + (rand()%((GB_YMAX-GB_YMIN)/2));
			if (rand_side == 2) { // Right
				x = ox = x0 = GB_XMAX - (r+1);
			}
			if (rand_side == 3) { // Left
				x = ox = x0 = GB_XMIN + (r+1);
			}
		}
	} else {
		x = ox = x0 = ri->x;
		y = oy = y0 = ri->y;
	}

	sem_wait(&mx_rk);
	rocks[i].x = x;
	rocks[i].y = y;
	rocks[i].r = r;
	sem_post(&mx_rk);

	tx = 0;
	ty = 0;
	dt = ((float)ROCK_PERIOD)/100000;

	while (1) {
		y = y0 + vy * ty;
		x = x0 + vx * tx;

		sem_wait(&mx_rk);
		rocks[i].x = x;
		rocks[i].y = y;
		r = rocks[i].r;
		sem_post(&mx_rk);

		draw_rock(ox, oy, r, RGB_BLACK, 0);

		if ((kill_rock) || (crash)){
			nrock--;
			sem_wait(&mx_rk);
			rocks[i].pid = NIL;
			sem_post(&mx_rk);
			return 0;
		}

		ox = x;
		oy = y;

		sem_wait(&mx_xy);
		ax = astro_x;
		ay = astro_y;
		sem_post(&mx_xy);

		if (dist_xy(x, y, ax, ay) < (ASTRO_RADIUS/2+r)) {
			if (!ab) {
				sem_wait(&mx_st_nrg);
				energy -= ENERGY_GOT;
				sem_post(&mx_st_nrg);
				ab = 1;
			}
		} else
			ab = 0;

		sem_wait(&mx_xy);
		if (dist_xy(x, y, astro_x, astro_y) < (ASTRO_RADIUS+r)/2) crash = 1;
		sem_post(&mx_xy);

#ifdef ASTRO_MOVE
		if (x <= GB_XMIN + r) {
			x0 = x = GB_XMAX - (r+1);
			y0 = y;
			tx = ty = 0;
		}
		if (x >= GB_XMAX - r) {
			x0 = x = GB_XMIN + (r+1);
			y0 = y;
			tx = ty = 0;
		}
		if (y <= GB_YMIN + r) {
			x0 = x;
			y0 = y = GB_YMAX - (r+1);
			tx = ty = 0;
		}
		if (y >= GB_YMAX - r) {
			x0 = x;
			y0 = y = GB_YMIN + (r+1);
			tx = ty = 0;
		}
#else
		if ( (x <= GB_XMIN + r) || (x >= GB_XMAX - r) || (y >= GB_YMAX - 2*r) ) {
			if (y >= GB_YMAX - 2*r) {
				sem_wait(&mx_st_nrg);
				energy -= ENERGY_GOT;
				sem_post(&mx_st_nrg);
			}
			nrock--;
			sem_wait(&mx_rk);
			rocks[i].pid = NIL;
			sem_post(&mx_rk);
			return 0;
		}
#endif
		draw_rock(ox, oy, r, RGB_YELLOW, 1);

		ty += dt;
		tx += dt;

		task_endcycle();
	}
}

void rock_create(rock_ini* ri)
{
	SOFT_TASK_MODEL mp;
	PID pid;
	int i;

	soft_task_default_model(mp);
	soft_task_def_ctrl_jet(mp);
	soft_task_def_group(mp, ROCK_GROUP);
	soft_task_def_met(mp, ROCK_WCET);
	soft_task_def_period(mp,ROCK_PERIOD);
	soft_task_def_usemath(mp);
	i = -1;
	do {
		i++;
		sem_wait(&mx_rk);
		pid = rocks[i].pid;
		sem_post(&mx_rk);
	} while (pid != NIL);
	ri->i = i;
	soft_task_def_arg(mp, (void *)ri);
	pid = task_create("Rock", rock, &mp, NULL);

	if (pid != NIL) {
		sem_wait(&mx_rk);
		rocks[i].pid = pid;
		sem_post(&mx_rk);
		task_activate(pid);
		nrock++;
	}
}

TASK rock_creator()
{
	while (1) {
		sem_wait(&mx_rn);
		if (rock_new.i == 0) {
			rock_create(&rock_new);
			rock_new.i = -1;
		}
		sem_post(&mx_rn);

		if ((nrock < ROCK_NMAX/2) && (!kill_rock) && (!crash)) {
			if ((rand()%ROCK_NMAX) > nrock) {
				rock_new.r = ROCK_RADIUS_I;
				rock_new.x = rock_new.y = 0;
				rock_create(&rock_new);
			}
		}
		task_endcycle();
	}
}

void reset_rock()
{
	kill_rock = 1;
}

void start_rock()
{
	kill_rock = 0;
}

void create_rock_task()
{
	SOFT_TASK_MODEL ms;

	soft_task_default_model(ms);
	soft_task_def_ctrl_jet(ms);
	soft_task_def_met(ms, ROCK_WCET);
	soft_task_def_period(ms,ROCK_PERIOD*10);
	soft_task_def_usemath(ms);
	pid_RC = task_create("RockCreator", rock_creator, &ms, NULL);
	if (pid_RC == NIL) {
		sys_shutdown_message("Could not create task <RockCreator>\n");
		exit(1);
	} else
		task_activate(pid_RC);
}

void init_rock()
{
	int i;

	nrock = 0;
	kill_rock = 1;
	for ( i=0; i<ROCK_NMAX; i++ ) rocks[i].pid = NIL;

	create_rock_task();
}

