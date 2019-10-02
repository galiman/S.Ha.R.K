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
 CVS :        $Id: ball.c,v 1.8 2005/05/10 17:21:17 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.8 $
 Last update: $Date: 2005/05/10 17:21:17 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai
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
/*              SIMULATION OF JUMPING BALLS                     */
/*--------------------------------------------------------------*/

#include "demo.h"
#include <kernel/func.h>
#include <stdlib.h>

#define R       8               /* dimension of a ball          */
#define G       9.8             /* acceleration of gravity      */

static int ballexit = 0;
static int      npc = 0;        /* number of tasks created      */

/*--------------------------------------------------------------*/
/*      Delay function for jumping balls                        */
/*--------------------------------------------------------------*/

void my_delay(void)
{
  int xxx;
  for (xxx=0; xxx<BALL_DELAY; xxx++);
}

/*--------------------------------------------------------------*/
/*      Periodic task for ball simulation                       */
/*--------------------------------------------------------------*/

TASK    palla(int i)
{
int     x, y;           /* coordinate grafiche pallina  */
int     ox, oy;         /* vecchia posizione pallina    */
int     x0, y0;         /* posizione iniziale X pallina */
float   vx, vy;         /* velocit… della pallina       */
float   vy0;            /* velocita' pallina al primo rimbalzo */
float   ty, tx;         /* variabile temporale          */
float   dt;             /* incremento temporale         */

    y = oy = y0 = BALL_HEIGHT;
    x = ox = x0 = BALL_XMIN;

    vy0= sqrt(2. * G * (float)BALL_HEIGHT);
    vy = 0;
    vx = BALL_VELX + myrand(9);
    tx = 0;
    ty = 0;
    dt = ((float)PERIOD_BALL)/100000;

    while (1) {
	y = y0 + vy*ty - .5*G*ty*ty;
	x = x0 + vx * tx;

	if (y < 0) {
		y = 0;

		if (vy == 0.0)
		  vy = vy0;
		else if (vy < BALL_VYMIN)
		  vy = vy0 * (1.0 - myrand(50)/100.0);
		else
		  vy = 0.9 * vy;

		ty = 0.0;
		y0 = 0;
	}

	if (x > BALL_XMAX) {
		tx = 0.0;
		x0 = BALL_XMAX;
		vx = -vx;
		x = x0 + vx * tx;
	}

	if (x < BALL_XMIN) {
		tx = 0.0;
		x0 = BALL_XMIN;
		vx = -vx;
		x = x0 + vx * tx;
	}

	mutex_lock(&mutex);
		grx_disc(ox, oy, R, 0);
		ox = x;
		oy = BALL_Y - y;
	mutex_unlock(&mutex);

	if (ballexit && i!=0xFFFF) {
	  npc--;
	  return 0;
	}

	mutex_lock(&mutex);
	 	grx_disc(ox, oy, R, i);
	mutex_unlock(&mutex);

	my_delay();

	ty += dt;
	tx += dt;
	task_endcycle();
    }
}

void killball(KEY_EVT *k)
{
  ballexit = 1;
}

void ballfun(KEY_EVT *k)
{
  SOFT_TASK_MODEL mp;
  int r,g,b;
  PID pid;
  char palla_str[]="palla  ";

  if (npc == BALL_MAX_P) return;

  ballexit = 0;

  r = 64 + myrand(190);
  g = 64 + myrand(190);
  b = 64 + myrand(190);

  itoa(npc,palla_str+5);

  soft_task_default_model(mp);
  soft_task_def_level(mp,2);
  soft_task_def_ctrl_jet(mp);
  soft_task_def_arg(mp, (void *)rgb16(r,g,b));
  soft_task_def_group(mp, BALL_GROUP);
  soft_task_def_met(mp, WCET_BALL);
  soft_task_def_period(mp,PERIOD_BALL);
  soft_task_def_usemath(mp);
  pid = task_create(palla_str, palla, &mp, NULL);

  if (pid != NIL) {
    task_activate(pid);
    npc++;
  }
}

void hardball()
{
  HARD_TASK_MODEL mp;
  int r,g,b;
  PID pid;

  r = 255;
  g = 255;
  b = 255;

  hard_task_default_model(mp);
  hard_task_def_ctrl_jet(mp);
  hard_task_def_arg(mp, (void *)rgb16(r,g,b));
  hard_task_def_wcet(mp, WCET_HARD_BALL);
  hard_task_def_mit(mp,PERIOD_BALL);
  hard_task_def_usemath(mp);
  pid = task_create("pallaEDF", palla, &mp, NULL);
  if (pid == NIL) {
	sys_shutdown_message("Could not create task <pallaEDF>");
	exit(1);
  }
  else
    task_activate(pid);
}


/*--------------------------------------------------------------*/
/*                      MAIN process                            */
/*--------------------------------------------------------------*/

void scenario_ball()
{
  grx_text("Noise", 0, 45 /*BALL_Y-BALL_HEIGHT-15*/, rgb16(0,0,255), black);
  grx_line(0,55,383,55,red);
  grx_rect(BALL_XMIN-R-1, BALL_Y-BALL_HEIGHT-R-1,
	   BALL_XMAX+R+1, BALL_Y+R+1, rgb16(0,200,0));
}

void init_ball(void)
{
    KEY_EVT k;

    hardball();

    k.flag = 0;
    k.scan = KEY_SPC;
    k.ascii = ' ';
    k.status = KEY_PRESSED;
    keyb_hook(k,ballfun,FALSE);

    k.flag = 0;
    k.scan = KEY_BKS;
    k.ascii = 0;
    k.status = KEY_PRESSED;
    keyb_hook(k,killball,FALSE);
}

/*--------------------------------------------------------------*/
