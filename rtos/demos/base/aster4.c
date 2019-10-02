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
 * CVS :        $Id: aster4.c,v 1.6 2006/07/03 13:08:29 tullio Exp $

 Test Number 13 (D):

 this is a part of the classic Hartik demo Aster, and it is based on aster 3.

 The demo creates:
 - a set of TBS tasks assigned to 2 TBS servers initialized with different bandwidth.

 - a set of periodic tasks, just to make noise (function asteroide)

 - a set of CBS tasks that are created to fill the available free
   bandwidth (function soft_aster)

 - a few service task (the one that creates the CBS tasks (aster), a clock, 
   JET info visualization

 - a set of never ending "system tasks" that simulate a device driver
   task that will end only at shutdown (function aper_asteroid)

 - a keyboard task that will execute an hook to terminate the system

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
 */

#include "kernel/kern.h"
#include "edf/edf/edf.h"
#include "cbs/cbs/cbs.h"

#include <drivers/shark_linuxc26.h>
#include <drivers/shark_input26.h>
#include <drivers/shark_keyb26.h>

int num_aster = 0;
#define ASTER_LIM	60
#define DISPLAY_MAX     15
#define ASTER_MAX       70
#define STAT_Y           9

#define PER_MAX          5
#define APER_MAX         8

// These numbers works on a Pentium 133 */
#define PER_WCET     25000
#define APER_WCET    53000
#define CLOCK_WCET    1000
#define ASTER_WCET    1000
#define SOFT_MET      6300

#define APER_REP     22000

PID aper_table[APER_MAX];

int shutting_down = 0;

TASK asteroide(void)
{
  int i;
  int y = rand() % 7 + 1;

  int load1,j;

  char s[2];

  s[0] = '*'; s[1] = 0;

  for (;;) {
    i = 1;
    while (i < ASTER_LIM) {
      load1 = 10000; //8000 + rand()%2000;
      for (j=0; j<load1; j++) {
	s[0] = '*' + rand() % 100;
	puts_xy(i,y,rand()%15+1,s);
      }

      task_activate(aper_table[rand()%APER_MAX]);
      task_endcycle();

      puts_xy(i,y,WHITE," ");
      i++;
    }
  }
  //num_aster--;
}

TASK aper_asteroid(void *a)
{
  int i;
  int y = rand() % 7 + 1;

  int load1,j;
  int c;

  char s[2];

  c = (int)a;
  s[0] = '*'; s[1] = 0;

  for (;;) {
    i = 1;
    while (i < ASTER_LIM) {
      load1 = APER_REP; //8000 + rand()%2000;
      for (j=0; j<load1; j++) {
	s[0] = '*' + rand() % 100;
	puts_xy(i,y,rand()%15+1,s);
      }
      s[0] = c;
      puts_xy(i,y,rand()%15+1,s);

      if (shutting_down) {
	cprintf("Ending System Task %d\n",exec_shadow);
	return 0;
      }

      task_endcycle();

      puts_xy(i,y,WHITE," ");
      i++;
    }
  }
}

TASK soft_aster(void)
{
  int i;
  int y = rand() % 7 + 1;

  int load1,j;

  char s[2];

  s[0] = '*'; s[1] = 0;

  i = 1;
  while (i < ASTER_LIM) {
    load1 = 1000 + rand()%9000;
    for (j=0; j<load1; j++) {
      s[0] = '*' + rand() % 100;
      puts_xy(i,y,rand()%15+1,s);
    }
    s[0] = 1;
    puts_xy(i,y,rand()%15+1,s);

    task_activate(aper_table[rand()%APER_MAX]);
    task_endcycle();

    puts_xy(i,y,WHITE," ");
    i++;
  }
  num_aster--;
  return 0;
}

TASK aster()
{
  PID p;

  HARD_TASK_MODEL m;
  SOFT_TASK_MODEL m_soft;
  int r;
  int x; // adaptive bandwidth...

  srand(7);

  /* create a set of periodic tasks, just to make noise */
  hard_task_default_model(m);
  hard_task_def_wcet(m,PER_WCET);
  hard_task_def_ctrl_jet(m);
  for (x=0; x<PER_MAX; x++) {
    r = (rand() % 200);
    hard_task_def_mit(m, (64+r)*1000);
    p = task_create("per",asteroide,&m,NULL);
    if (p!=-1) task_activate(p);
  }

  soft_task_default_model(m_soft);
  soft_task_def_met(m_soft,SOFT_MET);
  soft_task_def_ctrl_jet(m_soft);

  x = 64;

  while (1) {
    if (num_aster < ASTER_MAX) {
      r = (rand() % 200);

      soft_task_def_period(m_soft, (x+r)*1000);
      p = task_create("aaa",soft_aster,&m_soft,NULL);
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
    printf_xy(62,2,WHITE,"Uedf=%12u",EDF_usedbandwidth(1));
    printf_xy(62,4,WHITE,"Ucbs=%12u",CBS_usedbandwidth(5));
    task_endcycle();

    if (++s > 59) {
      s = 0;
      m++;
    }
    printf_xy(62,1,WHITE,"%2d:%2d ast=%d",m,s, num_aster);
    printf_xy(62,2,WHITE,"Uedf=%12u",EDF_usedbandwidth(1));
    printf_xy(62,4,WHITE,"Ucbs=%12u",CBS_usedbandwidth(5));
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
      if (jet_getstat(p, &sum, &max, &nact, &curr) == -1 ||
	  (proc_table[p].pclass & 0xFF00) == HARD_PCLASS) continue;

      for (j=0; j<5; j++) last[j] = 0;
      jet_gettable(p, &last[0], 5);
      kern_cli();
      if (proc_table[p].task_level == 5)
	printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
		  p, (int)sum/(nact==0 ? 1 : nact), (int)max, nact, (int)CBS_get_nact(4,p), (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
      else
	printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
		  p, (int)sum/(nact==0 ? 1 : nact), (int)max, nact, (int)curr, (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
      kern_sti();
      i++;
    }
  }
}

void endfun(KEY_EVT *k)
{
  exit(0);
}

void exiting(void *arg)
{
  cprintf("System shut down...\n");
  shutting_down = 1;
}

int main(int argc, char **argv)
{
  KEY_EVT k;

  PID p1,p2,p3;
  HARD_TASK_MODEL m;
  SOFT_TASK_MODEL m_aper;
  SOFT_TASK_MODEL m_soft;
  int i;

	k.ascii = 'c';
  k.scan = KEY_C;
  k.flag = CNTL_BIT;
  k.status = KEY_PRESSED;
  keyb_hook(k,endfun,FALSE);
	
	k.flag = CNTR_BIT;
  k.status = KEY_PRESSED;
  keyb_hook(k,endfun,FALSE);

  clear();
  cprintf("Press Ctrl-C to end the demo...");

  sys_atrunlevel(exiting, NULL, RUNLEVEL_SHUTDOWN);
    
  hard_task_default_model(m);
  hard_task_def_wcet(m,ASTER_WCET);
  hard_task_def_mit(m,10000);
  hard_task_def_group(m,1);
  hard_task_def_ctrl_jet(m);

  soft_task_default_model(m_soft);
  soft_task_def_met(m_soft,1000);
  soft_task_def_period(m_soft,100000);
  soft_task_def_group(m_soft,1);
  soft_task_def_ctrl_jet(m_soft);
  soft_task_def_aperiodic(m_soft);

  p1 = task_create("Aster",aster,&m,NULL);
  if (p1 == -1) {
    sys_shutdown_message("aster4.c(main): Could not create task <aster> ...");
    exit(0);
  }

  hard_task_def_mit(m,500000);
  hard_task_def_wcet(m,CLOCK_WCET);
  p2 = task_create("Clock",clock,&m,NULL);
  if (p2 == -1) {
    sys_shutdown_message("aster4.c(main): Could not create task <Clock> ...");
    exit(0);
  }

  p3 = task_create("JetControl",jetcontrol,&m_soft,NULL);
  if (p3 == -1) {
    sys_shutdown_message("aster4.c(main): Could not create task <JetControl> ...");
    exit(0);
  }

  soft_task_default_model(m_aper);
  soft_task_def_wcet(m_aper,APER_WCET);
  soft_task_def_ctrl_jet(m_aper);
  soft_task_def_system(m_aper);
  soft_task_def_aperiodic(m_aper);

  for (i=0; i<APER_MAX; i++) {
    soft_task_def_level(m_aper, i/4 + 3);
    soft_task_def_arg(m_aper, (void *)(i/4 ? 'Û' : '±'));
    aper_table[i] = task_create("aper",aper_asteroid,&m_aper,NULL);
    if (aper_table[i] == -1) {
      sys_shutdown_message("aster4.c(main): Could not create task <aper> ...");
      exit(0);
    }
  }

  group_activate(1);
  return 0;
}

