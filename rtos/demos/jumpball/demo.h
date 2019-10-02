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
 CVS :        $Id: demo.h,v 1.8 2006/06/27 09:27:15 tullio Exp $

 File:        $File$
 Revision:    $Revision: 1.8 $
 Last update: $Date: 2006/06/27 09:27:15 $
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


#include <ll/ll.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <math.h>

#include <drivers/shark_linuxc26.h>
#include <drivers/shark_pci26.h>
#include <drivers/shark_input26.h>
#include <drivers/shark_keyb26.h>
#include <drivers/shark_fb26.h>

#define FRAME_BUFFER_DEVICE 0 

/* Subparts */
#define JET_ON
#define BALL_ON


/* CPU Speed Selection
   --------------------------------------------------------------------
   1700 - Pentium 4 1.7 GHz
   1300 - Centrino 1.3 GHz
   400  - Pentium 2 400 MHz
   133  - Pentium 1 133 MHz
*/
#define CPU 400

/*
 *
 * WCET, Periods and Models
 *
 */

#if CPU==133
#define WCET_JETCTRL     10000
#define WCET_JETDUMMY      300
#define WCET_JETSLIDE     2000
#define WCET_BALL          250
#define WCET_HARD_BALL    1000
#define BALL_DELAY       10000
#endif

#if CPU==400
#define WCET_JETCTRL      7500
#define WCET_JETDUMMY      400
#define WCET_JETSLIDE     2100
#define WCET_BALL          250
#define WCET_HARD_BALL     480
#define BALL_DELAY       10000
#endif

#if CPU==1300
#define WCET_JETCTRL      4500
#define WCET_JETDUMMY      200
#define WCET_JETSLIDE     1300
#define WCET_BALL          250
#define WCET_HARD_BALL     650
#define BALL_DELAY      100000
#endif

#if CPU==1700
#define WCET_JETCTRL      4500
#define WCET_JETDUMMY      100
#define WCET_JETSLIDE     1300
#define WCET_BALL          250
#define WCET_HARD_BALL     410
#define BALL_DELAY      100000
#endif

#define PERIOD_JETCTRL  100000
#define PERIOD_JETDUMMY 100000
#define PERIOD_JETSLIDE 100000
#define PERIOD_BALL      10000

/*
 *
 * Global Stuffs
 *
 */

extern PID shutdown_task_PID;

/* graphic mutex... */
extern mutex_t mutex;

/* useful colors... */
extern int white;
extern int black;
extern int red;
extern int gray;

void init_jetcontrol();
void init_ball(void);
void scenario_jetcontrol();
void scenario_ball();
char *itoa(int n, char *s);
int myrand(int x);

/*
 *
 * JETCONTROL stuffs
 *
 */

#define JET_NTASK   35
#define JET_Y_NAME 170

/* 
 *
 * ATTENTION: the following PID MUST match
 * with the one assigned by Shark to the 
 * Dummy task during dummy module creation.
 *
 */
#define DUMMY_PID    2

#define JET_DUMMY_WIDTH    210
#define JET_DUMMY_HEIGHT    80

/* the point (x, y) is the top left corner */
#define JET_DUMMY_X        428
#define JET_DUMMY_Y        65

#define JET_SLIDE_WIDTH     50
#define JET_SLIDE_X        576

/*
 *
 * BALL stuffs
 *
 */

// x and y corners are specified whithout consider a border of 3 pixels
#define BALL_Y      450         /* position of the floor        */
#define BALL_HEIGHT 385         /* initial height of the ball   */
#define BALL_XMIN    10         /* min position X of the ball   */
#define BALL_XMAX   370         /* max position X of the ball   */
#define BALL_VELX     5.        /* horizontal ball velocity     */
#define BALL_VYMIN   11.        /* min ground speed             */
#define BALL_MAX_P   60         /* max number of balls          */

#define BALL_GROUP    2          /* task group of the balls */
