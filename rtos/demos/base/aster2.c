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
 *
 * CVS :        $Id: aster2.c,v 1.6 2005/02/25 11:10:46 pj Exp $

 this is a part of the classic Hartik demo Aster.

 It checks:
 - jet functions
 - The EDF level with many task, with almost full bandwidth used

*/


#include "kernel/kern.h"
#include "edf/edf/edf.h"

int num_aster = 0;
#define ASTER_LIM	60
#define DISPLAY_MAX     15
#define ASTER_MAX       70
#define STAT_Y           9

// first numbers for wcet and periods are for a 486/25, the others for a 
// celeron 366

#define PER_WCET     13000 /*6200*/
#define CLOCK_WCET    1200 /* 100*/
#define ASTER_WCET    1200 /* 100*/

#define ASTER_MEAN_PERIOD 64 /*64*/

#define END_TEST_TIME  60

TASK asteroide(void)
{
  int i;
  int y = rand() % 7 + 1;

  int load1,j;

  char s[2];

  s[0] = '*'; s[1] = 0;

  /*for (;;)*/ {
    i = 1;
    while (i < ASTER_LIM) {
      load1 = 1000; //10000; // 5000 + rand()%5000;
      for (j=0; j<load1; j++) {
	s[0] = '*' + rand() % 100;
	puts_xy(i,y,rand()%15+1,s);
      }

      task_endcycle();

      puts_xy(i,y,WHITE," ");
      i++;
    }
  }
  num_aster--;
  return 0;
}

TASK aster()
{
  PID p;

  HARD_TASK_MODEL m;
  int r;
  int x; // adaptive bandwidth...

  hard_task_default_model(m);
  hard_task_def_wcet(m,PER_WCET);
  hard_task_def_ctrl_jet(m);

  x = ASTER_MEAN_PERIOD;

  srand(7);
  while (1) {
    if (num_aster < ASTER_MAX) {
      r = (rand() % 200);

      hard_task_def_arg(m,(void *)((rand() % 7)+1));
      hard_task_def_mit(m, (x+r)*1000);
      p = task_create("aaa",asteroide,&m,NULL);
      if (p == -1)
	{
	  if (x < 500 && errno != ENO_AVAIL_TASK)  x += 1;
	  printf_xy(62,3,WHITE,"adapt=%3u err=%d",
		    iq_query_first(&freedesc),errno);
	}
      else {
	num_aster++;
	printf_xy(62,3,WHITE,"adapt=%3u           ",x);//,errno);
	task_activate(p);
	x /= 2;
	if (x<50) x = 50;
      }
    }
    task_endcycle();
  }
}

TASK clock()
{
  int s = 0, m = 0;

  while(1) {
    printf_xy(62,1,WHITE,"%2d:%2d ast=%d",m,s, num_aster);
    printf_xy(62,2,WHITE,"U=%12u",EDF_usedbandwidth(1));
    task_endcycle();

    if (++s > 59) {
      s = 0;
      m++;
    }
    printf_xy(62,1,WHITE,"%2d:%2d ast=%d",m,s, num_aster);
    printf_xy(62,2,WHITE,"U=%12u",EDF_usedbandwidth(1));
    task_endcycle();
  }
}



/* we consider the first ASTER_MAX + 2 tasks from the PID 2
   and plot on the screen the elapsed times... */
TASK jetcontrol()
{
  int i;  /* a counter */
  TIME sum, max, curr, last[5];
  int nact;
  int j; /* the elements set by jet_gettable */
  PID p;


  kern_cli();
  printf_xy(0,STAT_Y,WHITE,"PID ³ Mean T.³ Max T. ³ N.A. ³ Curr.   ³ Last1 ³ Last2 ³ Last3 ³ Last4 ³ Last5");
  kern_sti();

  for (;;) {
    for (i=0,p=0; i<DISPLAY_MAX+5 && p<MAX_PROC; p++) {
      if (jet_getstat(p, &sum, &max, &nact, &curr) == -1) continue;

      for (j=0; j<5; j++) last[j] = 0;
      jet_gettable(p, &last[0], 5);
      kern_cli();
      printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
		p, (int)sum/(nact==0 ? 1 : nact), (int)max, nact, (int)curr, (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
      kern_sti();
      i++;
    }
  }
}

int main(int argc, char **argv)
{
  PID p1,p2,p3; //,p4,p5,p6;
  HARD_TASK_MODEL m;
  NRT_TASK_MODEL m_nrt;
  struct timespec t;

  clear();

  hard_task_default_model(m);
  hard_task_def_wcet(m,ASTER_WCET);
  hard_task_def_mit(m,10000);
  hard_task_def_group(m,1);
  hard_task_def_ctrl_jet(m);

  nrt_task_default_model(m_nrt);
  nrt_task_def_group(m_nrt,1);
  nrt_task_def_ctrl_jet(m_nrt);
    
  p1 = task_create("Aster",aster,&m,NULL);
  if (p1 == -1) {
    sys_shutdown_message("test7.c(main): Could not create task <aster> ...");
    exit(1);
  }

  hard_task_def_mit(m,500000);
  hard_task_def_wcet(m,CLOCK_WCET);
  p2 = task_create("Clock",clock,&m,NULL);
  if (p2 == -1) {
    sys_shutdown_message("test7.c(main): Could not create task <Clock> ...");
    exit(1);
  }

  p3 = task_create("JetControl",jetcontrol,&m_nrt,NULL);
  if (p2 == -1) {
    sys_shutdown_message("test7.c(main): Could not create task <JetControl> ...");
    exit(1);
  }

  group_activate(1);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < END_TEST_TIME);

  exit(0);

  return 0;
}




