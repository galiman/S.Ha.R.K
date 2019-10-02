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
 CVS :        $Id: jetctrl.c,v 1.5 2005/01/08 14:35:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2005/01/08 14:35:17 $
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


// JetControl

#include "demo.h"
#include "kernel/func.h"

TASK jetdummy_task(void *arg)
{
  TIME   now_dummy, last_dummy, diff_dummy, slice;
  struct timespec now, last, diff;
  int x = 0;
  int height;

  NULL_TIMESPEC(&last);
  last_dummy = 0;
  for (;;) {
    task_nopreempt();
    jet_getstat(DUMMY_PID, NULL, NULL, NULL, &now_dummy);
    sys_gettime(&now);
    task_preempt();

    SUBTIMESPEC(&now, &last, &diff);
    slice = diff.tv_sec * 1000000 + diff.tv_nsec/1000;
    diff_dummy = now_dummy - last_dummy;

    height = (int)(JET_DUMMY_HEIGHT*((float)diff_dummy)/((float)slice));

    TIMESPEC_ASSIGN(&last, &now);
    last_dummy = now_dummy;

    grx_line(JET_DUMMY_X+x,JET_DUMMY_Y,
             JET_DUMMY_X+x,JET_DUMMY_Y+height          ,black);
    grx_line(JET_DUMMY_X+x,JET_DUMMY_Y+height,
             JET_DUMMY_X+x,JET_DUMMY_Y+JET_DUMMY_HEIGHT,white);
    grx_line(JET_DUMMY_X+(x+1)%JET_DUMMY_WIDTH,JET_DUMMY_Y,
             JET_DUMMY_X+(x+1)%JET_DUMMY_WIDTH,JET_DUMMY_Y+JET_DUMMY_HEIGHT,255);

    x = (x+1)%JET_DUMMY_WIDTH;

    task_endcycle();
  }
}


TASK jetctrl_task(void *arg)
{
  char st[50];
  TIME sum, max;
  int n;

  PID i;
  int printed = 0;

  for (;;) {
    for (i=2, printed=0; i<MAX_PROC && printed<JET_NTASK; i++) {
      if (jet_getstat(i, &sum, &max, &n, NULL) != -1) {
        if (!n) n=1;
        sprintf(st, "%6d %6d %10s", (int)sum/n, (int)max, proc_table[i].name);
        grx_text(st, 384, JET_Y_NAME+16+printed*8, gray, black);
        printed++;
      }
    }
    while (printed<JET_NTASK) {
      grx_text("                        ",
               384, JET_Y_NAME+16+printed*8, gray, black);
      printed++;
    }
    task_endcycle();
  }
}

TASK jetslide_task(void *arg)
{
  TIME sum, curr, max;

  TIME total[JET_NTASK];
  int slides[JET_NTASK];

  PID i;
  int printed = 0;

  for (;;) {
    // Fill the total array in a nonpreemptive section
    task_nopreempt();
    for (i=2, printed=0; i<MAX_PROC && printed<JET_NTASK; i++) {
      if (jet_getstat(i, &sum, NULL, NULL, &curr) != -1) {
        total[printed] = sum+curr;
        printed++;
      }
    }
    task_preempt();

    while (printed < JET_NTASK)
      total[printed++] = 0;

    // Compute the Max elapsed time
    max = 0;
    for (i=0; i<JET_NTASK; i++)
      if (total[i] > max) max = total[i];
    if (!max) max = 1;

    // Compute the slides width
    for (i=0; i<JET_NTASK; i++)
      slides[i] = (int)( (((float)total[i])/max) * JET_SLIDE_WIDTH);

    // print the data
    for (i=0; i<JET_NTASK; i++) {
      grx_box(JET_SLIDE_X,                 JET_Y_NAME+16+i*8,
              JET_SLIDE_X+slides[i],       JET_Y_NAME+23+i*8, gray);
      grx_box(JET_SLIDE_X+slides[i],       JET_Y_NAME+16+i*8,
              JET_SLIDE_X+JET_SLIDE_WIDTH, JET_Y_NAME+23+i*8, black);
    }

    while (i<JET_NTASK) {
      grx_box(JET_SLIDE_X,                 JET_Y_NAME+16+i*8,
              JET_SLIDE_X+JET_SLIDE_WIDTH, JET_Y_NAME+20+i*8, black);
      i++;
    }
    task_endcycle();
  }
}


void scenario_jetcontrol(void)
{
  grx_text("System load"         , 384, 45, rgb16(0,0,255), black);
  grx_line(384,55,639,55,red);

  grx_text("  Mean    Max Name  Slide", 384, JET_Y_NAME, gray, black);
  grx_line(384,JET_Y_NAME+10,639,JET_Y_NAME+10,gray);

  grx_rect(JET_DUMMY_X-1,               JET_DUMMY_Y-1,
           JET_DUMMY_X+JET_DUMMY_WIDTH, JET_DUMMY_Y+JET_DUMMY_HEIGHT+1, gray);

  grx_text("100%", JET_DUMMY_X-40, JET_DUMMY_Y, gray, black);
  grx_text("  0%", JET_DUMMY_X-40, JET_DUMMY_Y+JET_DUMMY_HEIGHT-8, gray, black);

  grx_line(JET_DUMMY_X-1, JET_DUMMY_Y, JET_DUMMY_X-5, JET_DUMMY_Y, gray);
  grx_line(JET_DUMMY_X-1, JET_DUMMY_Y+JET_DUMMY_HEIGHT, JET_DUMMY_X-5, JET_DUMMY_Y+JET_DUMMY_HEIGHT, gray);
}

void init_jetcontrol(void)
{
    SOFT_TASK_MODEL m3, m4, m5;

    PID p3, p4, p5;

    soft_task_default_model(m3);
    soft_task_def_level(m3,2);
    soft_task_def_period(m3, PERIOD_JETCTRL);
    soft_task_def_met(m3, WCET_JETCTRL);
    soft_task_def_ctrl_jet(m3);
    soft_task_def_group(m3, 1);
    p3 = task_create("jctrl", jetctrl_task, &m3, NULL);
    if (p3 == -1) {
	sys_shutdown_message("Could not create task <jetctrl> errno=%d", 
			     errno);
    	exit(1);
    }

    soft_task_default_model(m4);
    soft_task_def_level(m4,2);
    soft_task_def_period(m4, PERIOD_JETDUMMY);
    soft_task_def_met(m4, WCET_JETDUMMY);
    soft_task_def_group(m4, 1);
    soft_task_def_usemath(m4);
    soft_task_def_ctrl_jet(m4);
    p4 = task_create("jdmy", jetdummy_task, &m4, NULL);
    if (p4 == -1) {
	sys_shutdown_message("Could not create task <jetdummy> errno=%d", 
			     errno);
    	exit(1);
    }

    soft_task_default_model(m5);
    soft_task_def_level(m5,2);
    soft_task_def_period(m5, PERIOD_JETSLIDE);
    soft_task_def_met(m5, WCET_JETSLIDE);
    soft_task_def_group(m5, 1);
    soft_task_def_usemath(m5);
    soft_task_def_ctrl_jet(m5);
    p5 = task_create("jsli", jetslide_task, &m5, NULL);
    if (p5 == -1) {
	sys_shutdown_message("Could not create task <jetslide> errno=%d", 
			     errno);
    	exit(1);
    }
}

