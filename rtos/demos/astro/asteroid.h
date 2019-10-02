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

#include <kernel/func.h>
#include <stdlib.h>
#include <semaphore.h>
#include "sem/sem/sem.h"
#include "hartport/hartport/hartport.h"

#include <drivers/shark_keyb26.h>
#include <drivers/shark_fb26.h>

#include <math.h>

#define ASTRO_MOVE/* If defined then astro else defender */

#define FRAME_BUFFER_DEVICE 0

// DA CALCOLARE
#define RGB_BLACK     rgb16(  0,  0,  0)
#define RGB_GRAY      rgb16(127,127,127)
#define RGB_WHITE     rgb16(255,255,255)
#define RGB_RED       rgb16(255,  0,  0)
#define RGB_GREEN     rgb16(  0,255,  0)
#define RGB_BLUE      rgb16(  0,  0,255)
#define RGB_YELLOW    rgb16(255,255,  0)
#define RGB_MAGENTA   rgb16(255,  0,255)
#define RGB_CYAN      rgb16(  0,255,255)
#define RGB_D_RED     rgb16(127,  0,  0)
#define RGB_D_GREEN   rgb16(  0,127,  0)
#define RGB_D_BLUE    rgb16(  0,  0,127)
#define RGB_D_YELLOW  rgb16(127,127,  0)
#define RGB_D_MAGENTA rgb16(127,  0,127)
#define RGB_D_CYAN    rgb16(  0,127,127)

#define LIVE_0 "      "
#define LIVE_1 "     *"
#define LIVE_2 "    **"
#define LIVE_3 "   ***"
#define LIVE_4 "  ****"
#define LIVE_5 " *****"
#define LIVE_6 "******"
#define LIVE_X "------"

/* GameBoard constants */
#define GB_YMIN             65               /* position of the top */
#define GB_YMAX            460               /* position of the bottom */
#define GB_XMIN              5               /* min position X of the asteroid */
#define GB_XMAX            505               /* max position X of the asteroid */

/* Asteroid constants */
#define ASTEROID_RADIUS      4               /* radius of the asteroid */
#define ASTEROID_NMAX       60               /* max number of asteroids */

#define ASTEROID_GROUP       2               /* task group of asteroids */

/* Astro constants */
#ifdef ASTRO_MOVE
#define ASTRO_Y        (GB_YMAX+GB_YMIN)/2   /* Y position of the astro */
#else
#define ASTRO_Y        GB_YMAX-22            /* Y position of the astro */
#endif
#define ASTRO_X        (GB_XMAX+GB_XMIN)/2   /* X position of the astro */
#define ASTRO_MAX_VEL  ASTRO_VEL_INC*5       /* Max astro velocity */
#ifdef ASTRO_MOVE
#define ASTRO_MAX_GRAD     180               /* Max astro angle */
#else
#define ASTRO_MAX_GRAD      80               /* Max astro angle */
#endif
#define ASTRO_GRAD_INC      10               /* angular variation */
#define ASTRO_VEL_INC        3               /* velocity variation */
#define ASTRO_RADIUS        16               /* Dimension of the astro */
#define ASTRO_PERIOD      10000
#define ASTRO_MOVE_PERIOD 10000
#define ASTRO_WCET         1500

/* Shot constants */
#define SHOT_RADIUS          1               /* radius of the shot */
#define SHOT_VEL            35               /* speed of the shot */
#define SHOT_NMAX           30               /* max number of shots */

#define SHOT_GROUP           3               /* task group of shots */

#define SHOT_PERIOD      30000
#define SHOT_WCET         2000

/* Rock constants */
#define ROCK_RADIUS_I        8               /* initial radius of the rock */
#define ROCK_RADIUS_S        4               /* radius of the rock after one shot */
#define ROCK_VEL             6               /* speed of the rock */
#define ROCK_NMAX            8               /* max number of rocks */

#define ROCK_GROUP           4               /* task group of rocks */

#define ROCK_PERIOD      30000
#define ROCK_WCET         2000

/* Statistic constants */
#define ENERGY_INIT        200
#define ENERGY_SHOT          2
#define ENERGY_GOT          10
#define SCORE_GOT            2
#define LIVES_INIT           3

#define STAT_PERIOD     400000
#define STAT_WCET         2000

/* Statistic constants */
#define LOOK_PERIOD      30000
#define LOOK_WCET         4000

typedef struct {
	PID pid;
	int x, y, r;
} rock_pos;

typedef struct {
	int i;
	int x, y, r;
} rock_ini;

double dist_xy(int x1, int y1, int x2, int y2);
void draw_rock(int x, int y, int r, int c, int m);
void rock_create(rock_ini* ri);
void frame_astro();
void frame_stat();
void init_astro();
void init_stat();
void init_rock();
void reset_astro();
void reset_rock();
void start_astro();
void start_rock();
void reset_game();

extern sem_t  mx_mat, mx_grf;       /* mutex semaphores */
extern sem_t  mx_pos, mx_vel;       /* mutex semaphores */
extern sem_t  mx_xy, mx_rk, mx_rn;  /* mutex semaphores */
extern sem_t  mx_st_scr, mx_st_nrg; /* mutex semaphores */
extern sem_t  mx_st_kil, mx_st_liv; /* mutex semaphores */
extern int nshot;                   /* number of shot active */
extern int nrock;                   /* number of rock active */
extern int astro_x, astro_y;        /* astro position */
extern int astro_grad;              /* astro angolar position */
extern int astro_vel;               /* astro velocity */
extern int score;                   /* current player score */
extern int energy;                  /* current player energy */
extern int enemy;                   /* current player strikes */
extern int lives;                   /* current player lives*/
extern int kill_rock;               /* kill active rocks */
extern int kill_shot;               /* kill active shots */
extern int freez_astro;             /* turn of control pad */
extern int crash;                   /* astro vs. rock */
extern rock_pos rocks[ROCK_NMAX];   /* rocks position */
extern rock_ini rock_new;
