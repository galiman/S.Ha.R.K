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
#include <kernel/kern.h>

sem_t  mx_mat, mx_grf, mx_pos, mx_vel, mx_xy, mx_rk, mx_rn; /* mutex semaphores */
sem_t  mx_st_nrg, mx_st_scr, mx_st_kil, mx_st_liv;          /* mutex semaphores */

double dist_xy(int x1, int y1, int x2, int y2)
{
	double dst;
	int dx, dy;

	dx = x2 - x1;
	dy = y2 - y1;
	sem_wait(&mx_mat);
	dst = sqrt(dx*dx + dy*dy);
	sem_post(&mx_mat);

	return dst;

}

void frame_main()
{

	grx_clear(RGB_BLACK);

#ifdef ASTRO_MOVE
	grx_text("S.Ha.R.K. - Asteroid 2002 RC1", 0, 0, RGB_GREEN, RGB_BLACK );
#else
	grx_text("S.Ha.R.K. - Defender 2002 RC1", 0, 0, RGB_GREEN, RGB_BLACK );
#endif
	grx_text("by Marinoni Mauro"            ,24,16, RGB_GREEN, RGB_BLACK );
	grx_text("   Scaricabarozzi Mattia"     ,24,24, RGB_GREEN, RGB_BLACK );

	grx_text("Ctrl-C, Ctrr-C: exit"  ,300, 0, RGB_GRAY, RGB_BLACK );
	grx_text("O-P : turn left/right" ,300,16, RGB_GRAY, RGB_BLACK );
#ifdef ASTRO_MOVE
	grx_text("A-Z : speed up/down"   ,300,24, RGB_GRAY, RGB_BLACK );
	grx_text("S   : stop engine"     ,300,32, RGB_GRAY, RGB_BLACK );
	grx_text("F   : flip astro"      ,300,40, RGB_GRAY, RGB_BLACK );
#else
	grx_text("Z-X : move left/right" ,300,24, RGB_GRAY, RGB_BLACK );
#endif
	grx_text("Space : fire"          ,485,16, RGB_GRAY, RGB_BLACK );
	grx_text("N     : new game"      ,485,24, RGB_GRAY, RGB_BLACK );
	grx_text("B     : begin game"    ,485,32, RGB_GRAY, RGB_BLACK );

	frame_stat();
	frame_astro();
}

void end_func(KEY_EVT *k) {

	exit(0);
}

int main(int argc, char **argv)
{
	KEY_EVT k;
	TIME seme;

	k.flag = CNTR_BIT;
	k.scan = KEY_C;
	k.ascii = 'c';
	k.status = KEY_PRESSED;
	keyb_hook(k,end_func,FALSE);

	k.flag = CNTL_BIT;
	k.scan = KEY_C;
	k.ascii = 'c';
	k.status = KEY_PRESSED;
	keyb_hook(k,end_func,FALSE);

	sem_init(&mx_mat,0,1);
	sem_init(&mx_grf,0,1);
	sem_init(&mx_pos,0,1);
	sem_init(&mx_vel,0,1);
	sem_init(&mx_xy ,0,1);
	sem_init(&mx_rk ,0,1);
	sem_init(&mx_rn ,0,1);
	sem_init(&mx_st_scr,0,1);
	sem_init(&mx_st_nrg,0,1);
	sem_init(&mx_st_kil,0,1);
	sem_init(&mx_st_liv,0,1);

	seme = sys_gettime(NULL);
	srand(seme);

	frame_main();

	init_stat();
	init_astro();
	init_rock();

	return 0;

}
