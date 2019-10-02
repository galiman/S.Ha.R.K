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
 CVS :        $Id: mix.c,v 1.7 2005/01/08 14:33:10 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.7 $
 Last update: $Date: 2005/01/08 14:33:10 $
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

/*--------------------------------------------------------------*/
/*		DEMO with 9 INDEPENDENT TASKS			*/
/*--------------------------------------------------------------*/

#include <kernel/kern.h>

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>

#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define	PIG	3.1415
#define	DURATA	10000		/* counter duration in tick	*/

#define	LW	200		/* window length		*/
#define	HW	150		/* window height		*/
#define	HLOAD	30		/* Y level for the max load	*/
#define	LLOAD	(HW-HLOAD-5)	/* length for the max load	*/

#define	XWL	10		/* left X of LEFT window	*/
#define	XWM	220		/* left X of MIDDLE window	*/
#define	XWR	430		/* left X RIGHT window		*/

#define	YWH	5		/* top Y of HIGH window		*/
#define	YWM	165		/* top Y of MIDDLE window	*/
#define	YWL	325		/* top Y of LOW window		*/

int	flen = 0;		/* file length			*/
int	fine = 0;		/* ending flag			*/

sem_t	mx_mat, mx_grf;		/* mutex semaphores		*/

int	wcet[10];		/* array of task wcets		*/
int	period[10];		/* array of task periods	*/
double	load(long);		/* load evaluation function	*/

PID	ptas;
char	talk[5][25] =  {" SHARK Demonstration ",
			" RETIS Lab -- Scuola ",
			" Superiore   S. Anna ",
			" HARD REAL-TIME DEMO ",
			"    June 5,  2001    "};

char 	fbuf[1000] = "\
TASK    NAME    PERIOD          WCET\n\
------------------------------------------\n\
task1   watch:  1000000         200\n\
task2   tasto:  2000            200\n\
task3   palla:  2000            200\n\
task4   mosca:  20000           200\n\
task5   infor:  20000           300\n\
task6   ruota:  5000            400\n\
task7   color:  2000            200\n\
task8   pendo:  5000            400\n\
------------------------------------------\n";

extern int vga16color[16];

/*------------------------------------------------------*/
/*		file reading				*/
/*------------------------------------------------------*/

void	read_file(void)
{
  int	err;
  DOS_FILE *fp;

  fp = DOS_fopen("mix.dat","r");

  if (!fp) {
    err = DOS_error();
    cprintf("Error %d opening myfile.txt...\n", err);
    flen = 0;
    return;
  }

  flen = DOS_fread(&fbuf, 1, 1000, fp);
  cprintf("Read %d bytes from file\n", flen);
  DOS_fclose(fp);
}

/*------------------------------------------------------*/
/*		get data from buffer 			*/
/*------------------------------------------------------*/

void	get_par(void)
{
  int	x = 0;
  int	i;

  flen = strlen(fbuf);

  for (i=1; i<=8; i++) {
    while ((fbuf[x] != ':') && (x < flen)) x++;
    x++;
    sscanf(&fbuf[x], "%d %d", &period[i], &wcet[i]);
  }
}

/*--------------------------------------------------------------*/

void	finish1()
{
  exit(1);
}

/*--------------------------------------------------------------*/

void	finish2()
{
  fine = 1;
}

/****************************************************************/
/*			PROCESSO OROLOGIO			*/
/****************************************************************/

#define	LLAN	40		/* length of watch stick	*/

TASK	watch()
{
  int	x0 = XWL + LW/2;
  int	y0 = YWH + HW/2;
  int	grad;
  int	xg, yg;
  int	xt, yt, d;
  int	sec, min;
  char	s[5];
  double	rad, x, y;

  xg = x0;
  yg = y0 - LLAN;
  xt = XWL + 78;
  yt = YWH + 12;
  sec = min = 0;

  while (1) {
    sec = (sec + 1) % 60;
    if (sec == 0) min++;
    grad = 90 - sec * 6;
    rad = (double)grad * PIG / 180.;

    sem_wait(&mx_mat);
    x = (double)x0 + (double)LLAN * cos(rad);
    y = (double)y0 - (double)LLAN * sin(rad);
    sem_post(&mx_mat);

    sem_wait(&mx_grf);
    grx_line(x0, y0, xg, yg, vga16color[0]);
    sem_post(&mx_grf);

    xg = x;
    yg = y;

    sem_wait(&mx_grf);
    grx_line(x0, y0, xg, yg, vga16color[14]);
    sem_post(&mx_grf);

    sem_wait(&mx_grf);
    grx_text("0 :0 ", xt, yt, vga16color[14], vga16color[0]);
    sprintf(s, "%d", min);
    grx_text(s, xt+8, yt, vga16color[14], vga16color[0]);
    sprintf(s, "%d", sec);
    if (sec > 9) d = 24; else d = 32;
    grx_text(s, xt+d, yt, vga16color[14], vga16color[0]);
    sem_post(&mx_grf);

    task_endcycle();
  }
}

/****************************************************************/
/*		 PROCESSO DI RIEMPIMENTO			*/
/****************************************************************/

#define	CIMA	(YWH+5)		/* fondo del recipiente		*/
#define	FONDO	(YWH+HW-5)	/* cima  del recipiente		*/
#define	LREC	(XWM+75)	/* lato sinistro recipiente	*/
#define	DREC	50		/* diametro del recipiente	*/

TASK	tasto()
{
  int	x, y;
  int	x0;			/* coord. sinistra recipiente	*/
  int	col, cliq, bkg;
  int	i;
  int	liv;			/* livello del liquido		*/

  cliq = 9;
  bkg = 14;
  x0 = LREC;
  x = x0 + DREC/2;
  y = CIMA;
  liv = FONDO;

  while (1) {

    col = cliq;
    for (i=0; i<2; i++) {		/* disegna goccia */
      while (y < liv) {
	sem_wait(&mx_grf);
	grx_plot(x,y,vga16color[col]);
	sem_post(&mx_grf);
	y++;
      }
      y = CIMA;
      col = bkg;
    }

    liv--;
    sem_wait(&mx_grf);
    grx_line(x0+1, liv, x0+DREC-1, liv, vga16color[cliq]);
    sem_post(&mx_grf);

    if (liv <= CIMA+1) {		/* swap colors */
      i = bkg; bkg = cliq; cliq = i;
      liv = FONDO;
    }

    task_endcycle();
  }
}

/****************************************************************/

void	kboar()
{
  task_activate(ptas);
}

/****************************************************************/
/*			PROCESSO PALLA				*/
/****************************************************************/

#define	VMIN	11.		/* velocit… minima per suono	*/
#define	LP	3		/* lato della pallina		*/

TASK	palla()
{
  int	ox, oy;			/* vecchia posizione pallina	*/
  int	x0;			/* posizione iniziale pallina	*/
  int	xmin, xmax;
  int	base, top;
  int	xg, yg;			/* coordinate grafiche pallina	*/
  double	x, y;			/* coordinate pallina	*/
  double	G = 9.8;
  double	vx, vy, v0;		/* velocit… della pallina	*/
  double	t, tx;			/* variabile temporale		*/
  double	dt;			/* incremento temporale		*/
  double	arg;			/* variabile di appoggio	*/

  xmin = XWR+LP+1;
  xmax = XWR+LW-LP-1;
  base = YWH+HW-LP-1;
  top  = HW-10-LP;
  x = ox = x0 = xmin;
  y = oy = top;
  arg = 2.*G*(double)top;
  vy = v0 = sqrt(arg);
  vx = 15.;
  tx = 0.0;
  t = vy / G;
  dt = .02;

  while (1) {
    x = x0 + vx*tx;
    y = base - vy*t + .5*G*t*t;
    if (y >= base) {
      t = 0.0;
      vy = v0;
      y = base - vy*t + .5*G*t*t;
    }
    if (x >= xmax) {
      tx = 0.0;
      x0 = xmax;
      vx = -vx;
      x = x0 + vx*tx;
    }
    if (x <= xmin) {
      tx = 0.0;
      x0 = xmin;
      vx = -vx;
      x = x0 + vx*tx;
    }
    xg = x; yg = y;
    sem_wait(&mx_grf);
    grx_disc(ox,oy,LP,vga16color[0]);
    grx_disc(xg,yg,LP,vga16color[10]);
    sem_post(&mx_grf);
    oy = yg; ox = xg;
    t += dt;
    tx += dt;
    task_endcycle();
  }
}

/****************************************************************/
/*			PROCESSO MOSCA				*/
/****************************************************************/

TASK	mosca()
{
  int	x, y, Ax, Ay, h;
  int	x0, y0, tet;
  int	xmax,ymax;
  double	A, B;
  double	r;
  double	rnd;

  xmax = LW/2-1; ymax = HW/2-1;
  x = 0; y = 0; tet = 0;
  x0 = XWL+LW/2; y0 = YWM+HW/2;
  A = 5.; B = 30.;

  while (1) {

    rnd = (rand()%100)/100.;	/* rnd = [0,1] */
    h = (2. * B * rnd) - B;		/*  h = [-B,B] */
    tet = tet + h;

    if (tet > 360) tet = tet - 360;
    if (tet < 0) tet = tet + 360;
    r = tet * PIG / 180.;

    sem_wait(&mx_mat);
    Ax = (double)(A * cos(r));
    Ay = (double)(A * sin(r));
    sem_post(&mx_mat);
    x = x + Ax;
    y = y + Ay;

    if ((x >= xmax) || (x <= -xmax) ||
	(y >= ymax) || (y <= -ymax)) {
      x = x - Ax;
      y = y - Ay;
      tet = tet - 180;
      if (tet > 360) tet = tet - 360;
      if (tet < 0) tet = tet + 360;
      r = tet * PIG / 180.;
      sem_wait(&mx_mat);
      Ax = (double)(A * cos(r));
      Ay = (double)(A * sin(r));
      sem_post(&mx_mat);
      x = x + Ax;
      y = y + Ay;
    }
    sem_wait(&mx_grf);
    grx_plot(x+x0, y+y0, vga16color[10]);
    sem_post(&mx_grf);
    task_endcycle();
  }
}

/****************************************************************/
/*			PROCESSO INFORMAZIONI			*/
/****************************************************************/

TASK	infor()
{
  char	s[2];
  int	x, y;
  int	r;
  int	i = 0;
  int	leng;
  int	col = 0;

  r = 0;
  x = XWM + 16;
  y = YWM + 40;
  s[1] = 0;

  leng = 0;
  while (talk[0][leng] != 0) leng++;

  while (1) {
    s[0] = talk[r][i];
    sem_wait(&mx_grf);
    grx_text(s,x+i*8,y+r*8,vga16color[col+10],vga16color[1]);
    sem_post(&mx_grf);
    i++;
    if (i == leng) {
      i = 0;
      r = (r + 1) % 5;
      if (r == 0) col = (col + 1) % 6;
    }
    task_endcycle();
  }
}

/****************************************************************/
/*			PROCESSO RUOTA				*/
/****************************************************************/

TASK	ruota()
{
  int	x0 = XWR + LW/2;
  int	y0 = YWM + HW/2;
  int	grad = 90;
  int	xg, yg;
  double	rad, x, y;

  xg = x0;
  yg = y0 + LLAN;

  while (1) {

    rad = (double)grad * PIG / 180.;

    sem_wait(&mx_mat);
    x = (double)x0 + (double)LLAN * cos(rad);
    y = (double)y0 + (double)LLAN * sin(rad);
    sem_post(&mx_mat);

    sem_wait(&mx_grf);
    grx_disc(xg, yg, 4, vga16color[0]);
    sem_post(&mx_grf);

    xg = x; yg = y;

    sem_wait(&mx_grf);
    grx_disc(xg, yg, 4, vga16color[13]);
    sem_post(&mx_grf);

    grad = (grad + 1) % 360;

    task_endcycle();
  }
}

/****************************************************************/
/*			PROCESSO COLORI				*/
/****************************************************************/

TASK	color()
{
  int	xx0 = XWL+5;
  int	yy0 = YWL+5;
  int	n, col;
  int	x, y;

  x = 0; y = 0;

  while (1) {
    n = 19. * ((rand()%100)/100.);
    x = xx0 + n * 10;
    n = 14. * ((rand()%100)/100.);
    y = yy0 + n * 10;
    col = 16. * ((rand()%100)/100.);

    /*		xg = xx0 + x;
		yg = yy0 + y;
		x = (x + 10)%(LW-10);
		y = (y + 10)%(HW-10);
    */
    sem_wait(&mx_grf);
    grx_box(x, y, x+9, y+9, vga16color[col]);
    sem_post(&mx_grf);

    task_endcycle();
  }
}

/****************************************************************/
/*			PROCESSO PENDOLO			*/
/****************************************************************/

TASK	pendo()
{
  int	x0 = XWM+LW/2;
  int	y0 = YWL+10;
  int	xg, yg;
  int	col = 11;
  double	x, y, teta;
  double	v, a, dt;
  double	g, l;

  g = 9.8;
  l = 80.;
  dt = 0.1;
  teta = 40. * PIG / 180.;
  v = 0.;
  sem_wait(&mx_mat);
  x = l * sin((double)teta);
  y = l * cos((double)teta);
  a = -(g/l) * sin((double)teta);
  sem_post(&mx_mat);
  xg = x0 + x;
  yg = y0 + y;

  while (1) {

    v += a * dt;
    teta += v * dt;
    sem_wait(&mx_mat);
    x = l * sin((double)teta);
    y = l * cos((double)teta);
    a = -(g/l) * sin((double)teta);
    sem_post(&mx_mat);

    sem_wait(&mx_grf);
    grx_line(x0, y0, xg, yg, vga16color[0]);
    grx_circle(xg, yg, 5, vga16color[0]);
    grx_disc(xg, yg, 4, vga16color[0]);
    sem_post(&mx_grf);

    xg = x0+x; yg = y0+y;

    sem_wait(&mx_grf);
    grx_line(x0, y0, xg, yg, vga16color[col]);
    grx_circle(xg, yg, 5, vga16color[col+2]);
    grx_disc(xg, yg, 4, vga16color[col+1]);
    sem_post(&mx_grf);

    task_endcycle();
  }
}

/****************************** gener ******************************/

TASK	gener()
{
  HARD_TASK_MODEL m;
  SOFT_TASK_MODEL am;
  PID	pid;

  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[1]);
  hard_task_def_mit      (m, period[1]);
  hard_task_def_usemath  (m);
  pid = task_create("watch", watch, &m, NULL);
  task_activate(pid);
  keyb_getch(BLOCK);
  //---------------------------------------------
  soft_task_default_model(am);
  soft_task_def_met      (am, wcet[2]);
  soft_task_def_period   (am, period[2]);
  soft_task_def_aperiodic(am);
  soft_task_def_usemath  (am);
  ptas = task_create("tasto", tasto, &am, NULL);
  task_activate(ptas);
  keyb_getch(BLOCK);
  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[3]);
  hard_task_def_mit      (m, period[3]);
  hard_task_def_usemath  (m);
  pid = task_create("palla", palla, &m, NULL);
  task_activate(pid);
  keyb_getch(BLOCK);
  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[4]);
  hard_task_def_mit      (m, period[4]);
  hard_task_def_usemath  (m);
  pid = task_create("mosca", mosca, &m, NULL);
  task_activate(pid);
  keyb_getch(BLOCK);
  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[5]);
  hard_task_def_mit      (m, period[5]);
  hard_task_def_usemath  (m);
  pid = task_create("infor", infor, &m, NULL);
  task_activate(pid);
  keyb_getch(BLOCK);
  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[6]);
  hard_task_def_mit      (m, period[6]);
  hard_task_def_usemath  (m);
  pid = task_create("ruota", ruota, &m, NULL);
  task_activate(pid);
  keyb_getch(BLOCK);
  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[7]);
  hard_task_def_mit      (m, period[7]);
  hard_task_def_usemath  (m);
  pid = task_create("color", color, &m, NULL);
  task_activate(pid);
  keyb_getch(BLOCK);
  //---------------------------------------------
  hard_task_default_model(m);
  hard_task_def_wcet     (m, wcet[8]);
  hard_task_def_mit      (m, period[8]);
  hard_task_def_usemath  (m);
  pid = task_create("pendo", pendo, &m, NULL);
  task_activate(pid);
  //---------------------------------------------

  return NULL;
}

/****************************** MAIN ******************************/

int main()
{
  char	s[20];		        /* carattere letto da tastiera	*/
  int	x0, y0;
  int	x, y;
  TIME	t1, count;		/* contatori valutazione carico	*/
  double	car;			/* valore del carico corrente	*/
  TIME	seme;
  PID	pid;
  NRT_TASK_MODEL m2;
  KEY_EVT	eva, evx, evs;

  /* set the keyboard handler */
  eva.ascii = 'a';
  eva.scan = KEY_A;
  eva.flag = 0;
  eva.status = KEY_PRESSED;
  keyb_hook(eva,kboar,FALSE);

  evx.ascii = 'x';
  evx.scan = KEY_X;
  evx.flag = ALTL_BIT;
  evx.status = KEY_PRESSED;
  keyb_hook(evx,finish1,FALSE);

  evs.ascii = ESC;
  evs.scan = KEY_ESC;
  evs.flag = 0;
  evs.status = KEY_PRESSED;
  keyb_hook(evs,finish2,FALSE);

  sem_init(&mx_mat,0,1);
  sem_init(&mx_grf,0,1);

  seme = sys_gettime(NULL);
  srand(seme);

  get_par();

  grx_rect(XWL,YWH,XWL+LW,YWH+HW,vga16color[14]);
  grx_rect(XWM,YWH,XWM+LW,YWH+HW,vga16color[14]);
  grx_rect(XWR,YWH,XWR+LW,YWH+HW,vga16color[14]);

  grx_rect(XWL,YWM,XWL+LW,YWM+HW,vga16color[14]);
  grx_rect(XWM,YWM,XWM+LW,YWM+HW,vga16color[14]);
  grx_rect(XWR,YWM,XWR+LW,YWM+HW,vga16color[14]);

  grx_rect(XWL,YWL,XWL+LW,YWL+HW,vga16color[14]);
  grx_rect(XWM,YWL,XWM+LW,YWL+HW,vga16color[14]);
  grx_rect(XWR,YWL,XWR+LW,YWL+HW,vga16color[14]);

  x0 = XWL + LW/2;
  y0 = YWH + HW/2;
  grx_circle(x0, y0, LLAN+3, vga16color[12]);
  grx_rect(XWL+74, YWH+7, XWL+120, YWH+22, vga16color[12]);

  x0 = LREC;
  grx_line(x0, CIMA, x0, FONDO, vga16color[15]);
  grx_line(x0+DREC, CIMA, x0+DREC, FONDO, vga16color[15]);
  grx_line(x0, FONDO, x0+DREC, FONDO, vga16color[15]);
  grx_box(x0+1, CIMA, x0+DREC-1, FONDO-1, vga16color[14]);
  grx_text("Press A", XWM+16, YWH+48, vga16color[10], vga16color[0]);
  grx_text("to fill", XWM+16, YWH+64, vga16color[10], vga16color[0]);

  grx_text("Press:", XWM+18, YWM+HW-50, vga16color[10], vga16color[0]);
  grx_text("ESC to exit", XWM+18, YWM+HW-40, vga16color[10], vga16color[0]);
  grx_text("SPACE to create", XWM+18, YWM+HW-30, vga16color[10], vga16color[0]);

  x0 = XWR + LW/2;
  y0 = YWM + HW/2;
  grx_circle(x0, y0, LLAN/3, vga16color[14]);
  grx_disc(x0, y0, LLAN/3-1, vga16color[12]);

  x0 = XWR+5;
  y0 = YWL+HW-5;
  grx_line(x0, YWL+HLOAD, x0+LW-10, YWL+HLOAD, vga16color[12]);
  grx_text("SYSTEM WORKLOAD:", x0+5, YWL+HLOAD-10, vga16color[10], vga16color[0]);

  count = 0;
  t1 = sys_gettime(NULL);
  do count++; while (sys_gettime(NULL) < (t1 + DURATA));

  nrt_task_default_model(m2);
  pid = task_create("gener", gener, &m2, NULL);
  task_activate(pid);

  x = 0;
  while (!fine) {
    car = load(count);
    y = (double)LLOAD*car;
    sem_wait(&mx_grf);
    grx_line(x0+x, y0-LLOAD+1, x0+x, y0, vga16color[0]);
    grx_line(x0+x, y0-y, x0+x, y0, vga16color[15]);
    grx_text("    ", x0+LW-60, YWL+HLOAD-10, vga16color[0], vga16color[0]);
    sprintf(s, "%.3f", car);
    grx_text(s, x0+LW-50, YWL+HLOAD-10, vga16color[15], vga16color[0]);
    sem_post(&mx_grf);
    x = (x + 1) % (LW-10);
  }

  exit(1);

  return 0;
}

/****************************************************************/

double	load(long n)
{
  TIME	i, t1;
  double	carico;

  i = 0;
  t1 = sys_gettime(NULL);
  do i++; while (sys_gettime(NULL) < (t1 + DURATA));
  carico = 1. - (double)i / (double)n;
  return(carico);
}

/****************************************************************/
