/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
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

#include "asteroid.h"

int nshot;             /* number of shot active */
int astro_x, astro_y;  /* astro position */
int astro_grad;        /* astro angolar position */
PID pid_TR, pid_TL;    /* pid of turn tasks */
int astro_vel;         /* astro speed */
PID pid_SU, pid_SD;    /* pid of acc tasks */
PID pid_SZ, pid_FL;    /* pid of acc tasks */
int freez_astro;       /* turn of pad*/
int kill_shot;

rock_ini rock_new;

TASK shot(int i)
{
	int   x, y;   /* shot graphic position	*/
	int   ox, oy; /* shot old position	*/
	int   x0, y0; /* shot initial position	*/
	float vx, vy; /* shot speed	*/
	float ty, tx;
	float dt;
	float a_rad;
	int   a_vel;

	int      rx,  ry,  rr;
	PID      rp;

	sem_wait(&mx_xy);
	y = oy = y0 = astro_y;
	x = ox = x0 = astro_x;
	sem_post(&mx_xy);

	sem_wait(&mx_pos);
	a_rad = astro_grad * PI / 180.;
	sem_post(&mx_pos);
	sem_wait(&mx_vel);
	a_vel = astro_vel;
	sem_post(&mx_vel);
	sem_wait(&mx_mat);
	vx = - (SHOT_VEL + a_vel) * sin(a_rad);
	vy = - (SHOT_VEL + a_vel) * cos(a_rad);
	sem_post(&mx_mat);
	tx = 0;
	ty = 0;
	dt = ((float)SHOT_PERIOD)/100000;

	while (1) {
		y = y0 + vy * ty;
		x = x0 + vx * tx;

		sem_wait(&mx_grf);
		grx_disc(ox, oy, SHOT_RADIUS, RGB_BLACK);
		sem_post(&mx_grf);
		ox = x;
		oy = y;

		if ((kill_shot)||(crash)||(y < GB_YMIN+SHOT_RADIUS)||(x < GB_XMIN+SHOT_RADIUS)||(x > GB_XMAX-SHOT_RADIUS)||(y > GB_YMAX-SHOT_RADIUS)){
			nshot--;
			return 0;
		}

		for ( i=0; i<ROCK_NMAX; i++ ) {
			sem_wait(&mx_rk);
			rp = rocks[i].pid;
			rx = rocks[i].x;
			ry = rocks[i].y;
			rr = rocks[i].r;
			sem_post(&mx_rk);

			if ((rp!=NIL) && (dist_xy(x, y, rx, ry) < (SHOT_RADIUS+rr))) {
				draw_rock(rx, ry, rr, RGB_BLACK, 0);
				if (rr == ROCK_RADIUS_I) {
					sem_wait(&mx_rk);
					rocks[i].r = ROCK_RADIUS_S;
					sem_post(&mx_rk);
					sem_wait(&mx_rn);
					rock_new.r = ROCK_RADIUS_S;
					rock_new.x = rx;
					rock_new.y = ry;
					rock_new.i = 0;
					sem_post(&mx_rn);
					sem_wait(&mx_st_scr);
					score += SCORE_GOT;
					sem_post(&mx_st_scr);
				} else {
					task_kill(rp);
					sem_wait(&mx_rk);
					rocks[i].pid = NIL;
					sem_post(&mx_rk);
					nrock--;
					sem_wait(&mx_st_scr);
					score += 2*SCORE_GOT;
					sem_post(&mx_st_scr);
				}
				sem_wait(&mx_st_nrg);
				energy += ENERGY_GOT;
				sem_post(&mx_st_nrg);
				sem_wait(&mx_st_kil);
				enemy++;
				sem_post(&mx_st_kil);
				nshot--;
				return 0;
			}
		}

		sem_wait(&mx_grf);
		grx_disc(ox, oy, SHOT_RADIUS, RGB_RED);
		sem_post(&mx_grf);

		ty += dt;
		tx += dt;

		task_endcycle();
	}
}

TASK astro()
{
	float astro_rad;
	int ox, oy;
	float s, c;

	//leggo posizione
	sem_wait(&mx_pos);
	astro_rad = astro_grad * PI / 180.;
	sem_post(&mx_pos);
	sem_wait(&mx_mat);
	s = -sin(astro_rad);
	c = -cos(astro_rad);
	sem_post(&mx_mat);
	sem_wait(&mx_xy);
	ox = astro_x;
	oy = astro_y;
	sem_post(&mx_xy);

	while (1) {

		sem_wait(&mx_grf);
		//DRAW SHADOW ASTRO
		grx_line(ox + 15.*s       , oy + 15.*c       , ox -  5.*c -  9.*s, oy +  5.*s -  9.*c, RGB_BLACK);
		grx_line(ox + 15.*s       , oy + 15.*c       , ox +  5.*c -  9.*s, oy -  5.*s -  9.*c, RGB_BLACK);
		grx_line(ox               , oy               , ox - 12.*c -  9.*s, oy + 12.*s -  9.*c, RGB_BLACK);
		grx_line(ox               , oy               , ox + 12.*c -  9.*s, oy - 12.*s -  9.*c, RGB_BLACK);
		grx_line(ox - 12.*c - 9.*s, oy + 12.*s - 9.*c, ox + 12.*c -  9.*s, oy - 12.*s -  9.*c, RGB_BLACK);
		sem_post(&mx_grf);

		sem_wait(&mx_pos);
		astro_rad = astro_grad * PI / 180.;
		sem_post(&mx_pos);
		sem_wait(&mx_mat);
		s = -sin(astro_rad);
		c = -cos(astro_rad);
		sem_post(&mx_mat);
		sem_wait(&mx_xy);
		ox = astro_x;
		oy = astro_y;
		sem_post(&mx_xy);

		sem_wait(&mx_grf);
		//DRAW ASTRO
		grx_line(ox + 15.*s       , oy + 15.*c       , ox -  5.*c -  9.*s, oy +  5.*s -  9.*c, RGB_WHITE);
		grx_line(ox + 15.*s       , oy + 15.*c       , ox +  5.*c -  9.*s, oy -  5.*s -  9.*c, RGB_WHITE);
		grx_line(ox               , oy               , ox - 12.*c -  9.*s, oy + 12.*s -  9.*c, RGB_WHITE);
		grx_line(ox               , oy               , ox + 12.*c -  9.*s, oy - 12.*s -  9.*c, RGB_WHITE);
		grx_line(ox - 12.*c - 9.*s, oy + 12.*s - 9.*c, ox + 12.*c -  9.*s, oy - 12.*s -  9.*c, RGB_WHITE);
		sem_post(&mx_grf);

/*		{
			int xxx;
			for (xxx=0; xxx<10000; xxx++);
		}*/

		task_endcycle();
	}
}

TASK look()
{
	while (1) {
		if (crash) {
			reset_rock();
			reset_astro();
			if (nrock==0) {
				reset_game();
				crash = 0;
			}
		}
		task_endcycle();
	}
}

TASK turn(int i)
{

	while (1) {

		sem_wait(&mx_pos);
		if (i==0)
			astro_grad += 180;
		else {
			astro_grad += i;
		}
		if (astro_grad < -180) astro_grad += 360;
		if (astro_grad >  180) astro_grad -= 360;
		if (astro_grad < -ASTRO_MAX_GRAD) astro_grad = -ASTRO_MAX_GRAD;
		if (astro_grad >  ASTRO_MAX_GRAD) astro_grad =  ASTRO_MAX_GRAD;
		sem_post(&mx_pos);

		task_endcycle();
	}
}

TASK speed(int i)
{

	while (1) {

		sem_wait(&mx_vel);
		if (i==0)
			astro_vel = 0;
		else {
			astro_vel += i;
			if (astro_vel < -ASTRO_MAX_VEL) astro_vel = -ASTRO_MAX_VEL;
			if (astro_vel >  ASTRO_MAX_VEL) astro_vel =  ASTRO_MAX_VEL;
		}
		sem_post(&mx_vel);

		task_endcycle();
	}
}

TASK move()
{
	int dv, x, y;
	float drad;

	drad = x = y = 0;

	while (1) {

#ifdef ASTRO_MOVE
		sem_wait(&mx_pos);
		drad = astro_grad * PI / 180.;
		sem_post(&mx_pos);
#endif
		sem_wait(&mx_vel);
		dv = astro_vel;
		sem_post(&mx_vel);
		sem_wait(&mx_xy);
		x = astro_x;
		y = astro_y;
		sem_post(&mx_xy);
#ifdef ASTRO_MOVE
		x -= dv * sin(drad);
		y -= dv * cos(drad);
		if (x < GB_XMIN + ASTRO_RADIUS) x = GB_XMAX - ASTRO_RADIUS;
		if (x > GB_XMAX - ASTRO_RADIUS) x = GB_XMIN + ASTRO_RADIUS;
		if (y < GB_YMIN + ASTRO_RADIUS) y = GB_YMAX - ASTRO_RADIUS;
		if (y > GB_YMAX - ASTRO_RADIUS) y = GB_YMIN + ASTRO_RADIUS;
#else
		x += dv;
		if (x < GB_XMIN + 2*ASTRO_RADIUS) {
			x = GB_XMIN + 2*ASTRO_RADIUS;
			sem_wait(&mx_vel);
			astro_vel = - astro_vel - ASTRO_VEL_INC;
			sem_post(&mx_vel);
		}
		if (x > GB_XMAX - 2*ASTRO_RADIUS) {
			x = GB_XMAX - 2*ASTRO_RADIUS;
			sem_wait(&mx_vel);
			astro_vel = - astro_vel + ASTRO_VEL_INC;
			sem_post(&mx_vel);
		}
#endif
		sem_wait(&mx_xy);
		astro_x = x;
		astro_y = y;
		sem_post(&mx_xy);

		task_endcycle();
	}
}

void pad(KEY_EVT *k)
{
	if (freez_astro) return;
	if (k->scan == KEY_O) task_activate(pid_TL);
	if (k->scan == KEY_P) task_activate(pid_TR);
	if (k->scan == KEY_S) task_activate(pid_SZ);
	if (k->scan == KEY_Z) task_activate(pid_SD);
#ifdef ASTRO_MOVE
	if (k->scan == KEY_A) task_activate(pid_SU);
	if (k->scan == KEY_F) task_activate(pid_FL);
#else
	if (k->scan == KEY_X) task_activate(pid_SU);
#endif
}

void new_shot(KEY_EVT *k)
{
	SOFT_TASK_MODEL mp;
	PID pid;

	if ((nshot >= SHOT_NMAX)||(freez_astro)||(kill_shot)||(crash)) return;

	soft_task_default_model(mp);
	soft_task_def_ctrl_jet(mp);
	soft_task_def_arg(mp, (void *)nshot);
	soft_task_def_group(mp, SHOT_GROUP);
	soft_task_def_met(mp, SHOT_WCET);
	soft_task_def_period(mp,SHOT_PERIOD);
	soft_task_def_usemath(mp);
	pid = task_create("Shot", shot, &mp, NULL);

	if (pid != NIL) {
		task_activate(pid);
		nshot++;
		sem_wait(&mx_st_nrg);
		energy -= ENERGY_SHOT;
		sem_post(&mx_st_nrg);
	}
}

void start_astro()
{
	freez_astro = 0;
	kill_shot = 0;
}

void reset_astro()
{
	freez_astro = 1;
	sem_wait(&mx_xy);
	astro_x = ASTRO_X;
	astro_y = ASTRO_Y;
	sem_post(&mx_xy);
	sem_wait(&mx_pos);
	astro_grad = 0;
	sem_post(&mx_pos);
	sem_wait(&mx_vel);
	astro_vel = 0;
	sem_post(&mx_vel);
	kill_shot = 1;
}

void create_astro_task()
{
	HARD_TASK_MODEL mp;
	SOFT_TASK_MODEL ms;
	PID pid;
	int incr;

	soft_task_default_model(ms);
	soft_task_def_ctrl_jet(ms);
	soft_task_def_arg(ms, (void *)nshot);
	soft_task_def_met(ms, ASTRO_WCET);
	soft_task_def_period(ms,ASTRO_PERIOD);
	soft_task_def_usemath(ms);
	pid = task_create("Astro", astro, &ms, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <astro>\n");
		exit(1);
	} else
		task_activate(pid);

	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_wcet(mp,LOOK_WCET);
	hard_task_def_mit(mp, LOOK_PERIOD);
	hard_task_def_usemath(mp);
	pid = task_create("Taken", look, &mp, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <Taken>\n");
		exit(1);
	} else
		task_activate(pid);

	incr = ASTRO_GRAD_INC;
	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_arg(mp, (void *)incr);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, ASTRO_MOVE_PERIOD);
	hard_task_def_aperiodic(mp);
	hard_task_def_usemath(mp);
	pid_TL = task_create("TurnLeft", turn, &mp, NULL);
	if (pid_TL == NIL) {
		sys_shutdown_message("Could not create task <Turn L>\n");
		exit(1);
	}

	incr = - ASTRO_GRAD_INC;
	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_arg(mp, (void *)incr);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, ASTRO_MOVE_PERIOD);
	hard_task_def_aperiodic(mp);
	hard_task_def_usemath(mp);
	pid_TR = task_create("TurnRight", turn, &mp, NULL);
	if (pid_TR == NIL) {
		sys_shutdown_message("Could not create task <Turn R>\n");
		exit(1);
	}

	incr = ASTRO_VEL_INC;
	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_arg(mp, (void *)incr);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, ASTRO_PERIOD);
	hard_task_def_aperiodic(mp);
	hard_task_def_usemath(mp);
	pid_SU = task_create("SpeedUP", speed, &mp, NULL);
	if (pid_SU == NIL) {
		sys_shutdown_message("Could not create task <Speed UP>\n");
		exit(1);
	}

	incr = - ASTRO_VEL_INC;
	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_arg(mp, (void *)incr);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, ASTRO_PERIOD);
	hard_task_def_aperiodic(mp);
	hard_task_def_usemath(mp);
	pid_SD = task_create("SpeedDOWN", speed, &mp, NULL);
	if (pid_SD == NIL) {
		sys_shutdown_message("Could not create task <Speed DOWN>\n");
		exit(1);
	}

	incr = 0;
	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_arg(mp, (void *)incr);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, ASTRO_PERIOD);
	hard_task_def_aperiodic(mp);
	hard_task_def_usemath(mp);
	pid_SZ = task_create("SpeedZERO", speed, &mp, NULL);
	if (pid_SZ == NIL) {
		sys_shutdown_message("Could not create task <Speed ZERO>\n");
		exit(1);
	}

	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, 6*ASTRO_PERIOD);
	hard_task_def_usemath(mp);
	pid = task_create("MoveAstro", move, &mp, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <MoveAstro>\n");
		exit(1);
	} else
		task_activate(pid);

#ifdef ASTRO_MOVE
	incr = 0;
	hard_task_default_model(mp);
	hard_task_def_ctrl_jet(mp);
	hard_task_def_arg(mp, (void *)incr);
	hard_task_def_wcet(mp,ASTRO_WCET);
	hard_task_def_mit(mp, 3*ASTRO_PERIOD);
	hard_task_def_aperiodic(mp);
	hard_task_def_usemath(mp);
	pid_FL = task_create("FlipAstro", turn, &mp, NULL);
	if (pid_FL == NIL) {
		sys_shutdown_message("Could not create task <Flip Astro>\n");
		exit(1);
	}
#endif
}

void frame_astro()
{
	grx_text("Game", 10, 45, RGB_BLUE, RGB_BLACK);
	grx_line(GB_XMIN-2,55,GB_XMAX+2,55,RGB_RED);

	grx_rect(GB_XMIN-3, GB_YMIN-3, GB_XMAX+3, GB_YMAX+3, RGB_GREEN);

#ifndef ASTRO_MOVE
	grx_rect(GB_XMIN-1, GB_YMAX-1, GB_XMAX+1, GB_YMAX+1, RGB_CYAN);
#endif
}

void init_astro()
{
	KEY_EVT k;

	crash = 0;
	freez_astro = 1;
	kill_shot = 1;
	astro_vel  = 0;
	astro_grad = 0;
	astro_x = ASTRO_X;
	astro_y = ASTRO_Y;

	create_astro_task();

	k.flag = 0;
	k.scan = KEY_SPC;
	k.ascii = ' ';
	k.status = KEY_PRESSED | KEY_REPEATED;
	keyb_hook(k,new_shot,FALSE);

	k.flag = 0;
	k.scan = KEY_O;
	k.ascii = 'o';
	k.status = KEY_PRESSED | KEY_REPEATED;
	keyb_hook(k,pad,FALSE);

	k.flag = 0;
	k.scan = KEY_P;
	k.ascii = 'p';
	k.status = KEY_PRESSED | KEY_REPEATED;
	keyb_hook(k,pad,FALSE);

	k.flag = 0;
	k.scan = KEY_S;
	k.ascii = 's';
	k.status = KEY_PRESSED | KEY_REPEATED;
	keyb_hook(k,pad,FALSE);

	k.flag = 0;
	k.scan = KEY_Z;
	k.ascii = 'z';
	k.status = KEY_PRESSED | KEY_REPEATED;
	keyb_hook(k,pad,FALSE);

#ifdef ASTRO_MOVE
	k.flag = 0;
	k.scan = KEY_A;
	k.ascii = 'a';
	k.status = KEY_PRESSED;
	keyb_hook(k,pad,FALSE);

	k.flag = 0;
	k.scan = KEY_F;
	k.ascii = 'f';
	k.status = KEY_PRESSED;
	keyb_hook(k,pad,FALSE);
#else
	k.flag = 0;
	k.scan = KEY_X;
	k.ascii = 'x';
	k.status = KEY_PRESSED;
	keyb_hook(k,pad,FALSE);

#endif
}
