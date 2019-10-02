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
#include <string.h>

int score;             /* current player score */
int energy;            /* current player energy */
int enemy;             /* current player killed enemy */
int lives;             /* current player lives */
int crash;             /* astro vs. rock */

TASK stat_write()
{
	char st[20];
	int l;

	while (1) {

		sem_wait(&mx_st_scr);
		sprintf(st, "%6d", score);
		sem_post(&mx_st_scr);
		sem_wait(&mx_grf);
		grx_text(st, GB_XMAX+80, GB_YMIN+ 8, RGB_YELLOW, RGB_BLACK);
		sem_post(&mx_grf);

		sem_wait(&mx_st_nrg);
		sprintf(st, "%6d", energy);
		if (energy <= 0) crash = 1;
		sem_post(&mx_st_nrg);
		sem_wait(&mx_grf);
		grx_text(st, GB_XMAX+80, GB_YMIN+24, RGB_YELLOW, RGB_BLACK);
		sem_post(&mx_grf);

		sem_wait(&mx_st_kil);
		sprintf(st, "%6d", enemy);
		sem_post(&mx_st_kil);
		sem_wait(&mx_grf);
		grx_text(st, GB_XMAX+80, GB_YMIN+40, RGB_YELLOW, RGB_BLACK);
		sem_post(&mx_grf);

		strcpy(st,LIVE_X);
		sem_wait(&mx_st_liv);
		l = lives;
		sem_post(&mx_st_liv);
		if (l == 0) strcpy(st,LIVE_0);
		if (l == 1) strcpy(st,LIVE_1);
		if (l == 2) strcpy(st,LIVE_2);
		if (l == 3) strcpy(st,LIVE_3);
		if (l == 4) strcpy(st,LIVE_4);
		if (l == 5) strcpy(st,LIVE_5);
		if (l == 6) strcpy(st,LIVE_6);
		sem_wait(&mx_grf);
		grx_text(st, GB_XMAX+80, GB_YMIN+56, RGB_YELLOW, RGB_BLACK);
		sem_post(&mx_grf);

		task_endcycle();
	}
}

void reset_game()
{
	sem_wait(&mx_st_liv);
	if (lives > 0) {
		lives--;
		sem_post(&mx_st_liv);
	} else {
		lives = LIVES_INIT;
		sem_post(&mx_st_liv);

		sem_wait(&mx_st_scr);
		score = 0;
		sem_post(&mx_st_scr);

		sem_wait(&mx_st_kil);
		enemy = 0;
		sem_post(&mx_st_kil);
	}

	sem_wait(&mx_st_nrg);
	energy = ENERGY_INIT;
	sem_post(&mx_st_nrg);
}

void new_game(KEY_EVT *k)
{
	reset_rock();
	reset_astro();
	reset_game();
}

void start_game(KEY_EVT *k)
{
	start_astro();
	start_rock();
}

void frame_stat()
{
	grx_text("Statistics", GB_XMAX+10, 45, RGB_BLUE, RGB_BLACK);
	grx_line(GB_XMAX+8,55,640-8,55,RGB_RED);

	grx_rect(GB_XMAX+7, GB_YMIN-3, 640-6, GB_YMIN+70, RGB_GREEN);
	grx_text("Score  : ", GB_XMAX+15, GB_YMIN+ 8, RGB_CYAN, RGB_BLACK);
	grx_text("Energy : ", GB_XMAX+15, GB_YMIN+24, RGB_CYAN, RGB_BLACK);
	grx_text("Enemy  : ", GB_XMAX+15, GB_YMIN+40, RGB_CYAN, RGB_BLACK);
	grx_text("Lives  : ", GB_XMAX+15, GB_YMIN+56, RGB_CYAN, RGB_BLACK);
}

void create_stat_task()
{
	SOFT_TASK_MODEL ms;
	PID pid;

	soft_task_default_model(ms);
	soft_task_def_ctrl_jet(ms);
	soft_task_def_met(ms, STAT_WCET);
	soft_task_def_period(ms,STAT_PERIOD);
	soft_task_def_usemath(ms);
	pid = task_create("StatWrite", stat_write, &ms, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <StatWrite>\n");
		exit(1);
	} else
		task_activate(pid);
}

void init_stat()
{
	KEY_EVT k;

	score  = 0;
	enemy  = 0;
	energy = ENERGY_INIT;
	lives  = LIVES_INIT;

	create_stat_task();

	k.flag = 0;
	k.scan = KEY_N;
	k.ascii = 'n';
	k.status = KEY_PRESSED;
	keyb_hook(k,new_game,FALSE);

	k.flag = 0;
	k.scan = KEY_B;
	k.ascii = 'b';
	k.status = KEY_PRESSED;
	keyb_hook(k,start_game,FALSE);
}

