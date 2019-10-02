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

/*
 ------------
 CVS :        $Id: orbit.c,v 1.9 2005/01/08 14:34:48 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.9 $
 Last update: $Date: 2005/01/08 14:34:48 $
 ------------
*/

/*
 * Copyright (C) 2000 Giorgio Buttazzo and Paolo Gai
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

/****************************************************************/
/*		SIMULAZIONE DI MASSE CHE ORBITANO		*/
/****************************************************************/

#include <kernel/kern.h>
#include <semaphore.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>

#define	MAX	11		/* numero massimo pianeti	*/
#define	XMAX	639		/* valore massimo coordinata X	*/
#define	YMAX	479		/* valore massimo coordinata Y	*/

extern int vga16color[16];

struct	coord {
  double	x;
  double	y;
};

int	flen;			// file length

double	mass[MAX];		/* vettore masse pianeti	*/
struct	coord pos[MAX];		/* vettore posizioni attuali	*/
struct	coord vel[MAX];		/* vettore velocita' iniziali	*/

int	XGS, YGS;		/* Coordinate centro spazio	*/
int	RP, RT;			/* raggio pianeta, raggio Terra	*/
int	np;			/* numero attuale di pianeti	*/
double	G;			/* Gravitazione Universale	*/
double	tick;			/* tick di sistema		*/
double	delta;			/* incremento temporale		*/
double	scala;			/* fattore grafico di scala	*/

char    fbuf[1000] ="\
----------------------------------------------------\n\
period: 10000   wcet: 500\n\
delta: 1.       scala: 150.\n\
G: 6.6e-15\n\
r_pianeta: 4    r_Terra: 8\n\
X_centro: 320   Y_centro: 240\n\
------------------- pianeti ------------------------\n\
        massa   pos.x   pos.y   vel.x   vel.y\n\
0:      \t 6.0e21  \t 0.      \t 0.      \t 0.      \t 0. \n\
1:      \t 1.0e21  \t 10000.  \t 8000.   \t -60.    \t 0. \n\
2:      \t 1.0e8   \t 5000.   \t 0.      \t 0.      \t 80. \n\
3:      \t 5.0e18  \t 10000.  \t 8000.   \t -50.    \t 0. \n\
4:      \t 1.0e9   \t 10000.  \t 8000.   \t -40.    \t 20. \n\
5:      \t 1.0e15  \t 1000.   \t 5000.   \t -80.    \t 0. \n\
6:      \t 1.0e5   \t 1000.   \t 5000.   \t -80.    \t 0. \n\
7:      \t 1.0e17  \t 1000.   \t 5000.   \t -80.    \t 0. \n\
8:      \t 1.0e5   \t 1000.   \t 5000.   \t -80.    \t 0. \n\
9:      \t 1.0e5   \t 1000.   \t 5000.   \t -80.    \t 0. \n\
10:     \t 1.0e5   \t 1000.   \t 5000.   \t -80.    \t 0. \n\
----------------------------------------------------\n";

// -------------------------------------------------------
// NOTA: %f o %lf significa double e %nf significa float
// -------------------------------------------------------

PID	pid;
int	period;			/* task period			*/
int     wcet;			/* task wcet                    */
sem_t   mutex;			/* semaforo di mutua esclusione	*/


void	get_par(void);

/*--------------------------------------------------------------*/

int	inside(int x, int y)
{
  return ((x > RP) && (x < XMAX-RP) &&
	  (y > RP) && (y < YMAX-RP));
}

/*--------------------------------------------------------------*/

TASK	massa(void *xxx)
{
  int	i = (int)xxx;		/* parametro del task */
  int	gx, gy;			/* coordinate grafiche pallina	*/
  int	ox, oy;			/* vecchia posizione pallina	*/
  int	j;
  int	r, col;			/* raggio e colore pianeta	*/
  double	dt;			/* incremento temporale		*/
  double	dist=0.0, dx, dy;	/* distanze pianeta-pianeta	*/
  double	dist0=0.0;		/* distanze pianeta-Terra	*/
  double	x, y;			/* posizione del pianeta	*/
  double	vx, vy;			/* velocita' del pianeta	*/
  double	ax, ay;			/* accelerazione del pianeta	*/
  double	k;			/* variabile ausiliaria		*/
  double	arg;			/* variabile di appoggio	*/

  x = pos[i].x;	y = pos[i].y;
  vx = vel[i].x;	vy = vel[i].y;
  ox = XGS + x / scala;
  oy = YGS + y / scala;
  dt = delta;

  do {
    x = pos[i].x;
    y = pos[i].y;
    ax = ay = 0.0;
    for (j=0; j<np; j++) {
      if (j != i) {
	dx = pos[j].x - x;
	dy = pos[j].y - y;
	arg = dx*dx + dy*dy;
	dist = sqrt(arg);
	if (dist < RP*scala) dist = RP*scala;
	k = G * mass[j] / (dist*dist*dist);
	ax += k * dx;
	ay += k * dy;
      }
      if (j == 0) dist0 = dist - (RP+RT)*scala;
    }
    x += vx*dt + 0.5*ax*dt*dt;
    y += vy*dt + 0.5*ay*dt*dt;
    vx += ax * dt;
    vy += ay * dt;

    gx = XGS + x / scala;
    gy = YGS + y / scala;

    r = RP;
    col = i + 1;

    sem_wait(&mutex);
    grx_disc(ox,oy,r,vga16color[0]);
    grx_disc(gx,gy,r,vga16color[col]);
    sem_post(&mutex);

    pos[i].x = x;	pos[i].y = y;
    ox = gx;	oy = gy;

    task_endcycle();

  } while ((dist0 > 0) && inside(gx,gy));

  sem_wait(&mutex);
  grx_disc(ox,oy,r,vga16color[0]);
  grx_disc(XGS,YGS,RT,vga16color[12]);
  grx_circle(XGS,YGS,RT,vga16color[14]);
  sem_post(&mutex);

  return NULL;
}

/*--------------------------------------------------------------*/
/*			MAIN					*/
/*--------------------------------------------------------------*/

int main()
{
  HARD_TASK_MODEL m;
  char	c;			/* carattere letto da tastiera	*/

  sem_init(&mutex,0,1);

  get_par();

  grx_disc(XGS,YGS,RT,vga16color[12]);
  grx_circle(XGS,YGS,RT,vga16color[14]);

  np = 0;

  do {
    if (np < MAX-1) {
      np++;
      hard_task_default_model(m);
      hard_task_def_arg      (m, (void *)np);
      hard_task_def_wcet     (m, wcet);
      hard_task_def_mit      (m, period);
      hard_task_def_usemath  (m);
      pid = task_create("massa", massa, &m, NULL);
      if (pid == NIL) {
	sys_shutdown_message("Could not create task");
	exit(1);
      }
      task_activate(pid);
    }
    c = keyb_getch(BLOCK);

  } while (c != ESC);

  exit(1);

  return 0;
}

/*------------------------------------------------------*/
/*		file reading				*/
/*------------------------------------------------------*/

void	read_file(void)
{
  int	err;
  DOS_FILE *fp;

  fp = DOS_fopen("orbit.dat","r");

  if (!fp) {
    err = DOS_error();
    cprintf("Error %d opening myfile.txt...\n", err);
    flen = 0;
    return;
  }

  flen = DOS_fread(&fbuf, 1, 1000, fp);
  cprintf("Read %d bytes from orbit.dat\n", flen);

  DOS_fclose(fp);
}

/*------------------------------------------------------*/
/*		get data from buffer 			*/
/*------------------------------------------------------*/

void	get_par(void)
{
  int	x = 0;
  int	i;
  double vx, vy, px, py;
 
  flen = strlen(fbuf);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%d", &period);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%d", &wcet);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%lf", &delta);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%lf", &scala);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%lf", &G);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%d", &RP);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%d", &RT);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%d", &XGS);

  while ((fbuf[x] != ':') && (x < flen)) x++;
  x++;
  sscanf(&fbuf[x], "%d", &YGS);

  for (i=0; i<MAX; i++) {

    mass[i] = 0.;
    px = 0.;
    py = 0.;
    vx = 0.;
    vy = 0.;

    while ((fbuf[x] != '\t') && (x < flen)) x++;
    x++;
    sscanf(&fbuf[x], "%lf", &mass[i]);

    while ((fbuf[x] != '\t') && (x < flen)) x++;
    x++;
    sscanf(&fbuf[x], "%lf", &px);

    while ((fbuf[x] != '\t') && (x < flen)) x++;
    x++;
    sscanf(&fbuf[x], "%lf", &py);

    while ((fbuf[x] != '\t') && (x < flen)) x++;
    x++;
    sscanf(&fbuf[x], "%lf", &vx);

    while ((fbuf[x] != '\t') && (x < flen)) x++;
    x++;
    sscanf(&fbuf[x], "%lf", &vy);

    pos[i].x = px; pos[i].y = py;

    vel[i].x = vx; vel[i].y = vy;

  }
}

/*--------------------------------------------------------------*/

