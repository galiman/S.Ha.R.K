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
 * CVS :        $Id: shutdown.c,v 1.2 2005/02/25 11:04:46 pj Exp $

  Shutdown demo, used to test system shutdown.

  - derived from demo aster4
  
  - the aper_asteroid task simulates some system tasks that have to
    finish prior system shutdown.

  - note that if you remove the nanosleep in aper_asteroid
    the system does not shut down correctly because the system
    tasks does not receive any CPU time!!!
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

#include <time.h>

#include <drivers/shark_linuxc26.h>
#include <drivers/shark_input26.h>
#include <drivers/shark_keyb26.h>

int num_aster = 0;
#define ASTER_LIM       60
#define DISPLAY_MAX     10
#define ASTER_MAX       70
#define STAT_Y          24

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
int nobandwidth;

TASK asteroide(void *arg)
{
  int i;

  for (;;) {
    i = 1;
    for (i=1; i < ASTER_LIM; i++) {
      puts_xy(i,20,WHITE,"*");
      task_endcycle();
      puts_xy(i,20,WHITE," ");
    }
  }
}


TASK system_asteroide(void *arg)
{
  int i;
  struct timespec t;
  t.tv_sec = 0;

  for (;;) {
    i = 1;
    for (i=1; i < ASTER_LIM; i++) {
      puts_xy(i,21,WHITE,"s");

      if (shutting_down) {
        cprintf("Ending System Task %d\n",(int)arg);
        return 0;
      }
      
      t.tv_nsec = 1000000*(100+rand()%200);
      nanosleep(&t,NULL);

      puts_xy(i,21,WHITE," ");
    }
  }
}


TASK aster()
{
  PID p;

  SOFT_TASK_MODEL m_soft;
  int r;
  int x; // adaptive bandwidth...


  soft_task_default_model(m_soft);
  soft_task_def_met(m_soft,SOFT_MET);
  soft_task_def_ctrl_jet(m_soft);

  x = 64;

  while (1) {
    if (num_aster < ASTER_MAX) {
      r = (rand() % 200);

      soft_task_def_period(m_soft, (x+r)*1000);
      p = task_create("aaa",asteroide,&m_soft,NULL);
      if (p == -1)
	{
	  if (x < 500 && errno != ENO_AVAIL_TASK)  x += 1;
	}
      else {
	num_aster++;
	task_activate(p);
	x /= 2;
	if (x<50) x = 50;
      }
    }
    task_endcycle();
  }
}


/* we consider the first ASTER_MAX + 2 tasks from the PID 2
   and plot on the screen the elapsed times... */
TASK jetcontrol()
{
  int i;  /* a counter */
  TIME curr;
  PID p;
  struct timespec t;
  t.tv_sec=0;
  t.tv_nsec=100000;


  kern_cli();
  printf_xy(0,STAT_Y,WHITE,"PID ³ Curr.   ³ status ³ name");
  kern_sti();

  for (;;) {
    for (i=0,p=0; i<DISPLAY_MAX+5 && p<MAX_PROC; p++) {
      if (proc_table[p].status==FREE) continue;

      curr = 0;
      jet_getstat(p, NULL, NULL, NULL, &curr);

      kern_cli();
	printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-7d ³ %-6d ³ %s                      ",
		  p, (int)curr, proc_table[p].status, proc_table[p].name);
      kern_sti();
      i++;
    }

    for (; i<DISPLAY_MAX+5;i++) {
      printf_xy(0,STAT_Y+i+1,WHITE,"    ³         ³        ³                     ");
    }

    if (!nobandwidth) nanosleep(&t, NULL);
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

  PID p1,p3;
  HARD_TASK_MODEL m;
  SOFT_TASK_MODEL m_soft;
  NRT_TASK_MODEL  m_nrt;
  int i;

  srand(7);

  clear();
  cprintf("REMEMBER: RUN THIS DEMO IN 80x50 TEXT MODE!!!\n");
  cprintf("use \"mode con lines=50\" to put the screen in 80x50.\n\n");
  cprintf("Press SPACE for a typical WRONG shutdown sequence\n");
  cprintf("   where the shutdown task does not receive bandwidth\n\n");
  cprintf("Press another key for a clear shutdown\n");
  nobandwidth = !(keyb_getch(BLOCK) - ' ');


  k.flag = 0;
  k.scan = KEY_ENT;
  k.ascii = 13;
  k.status = KEY_PRESSED;
  keyb_hook(k, endfun, FALSE);



  clear();
  if (nobandwidth)
    cprintf("ENTER to end the demo WITH A TIMEOUT...\n");
  else
    cprintf("ENTER to end the demo SHUTTING DOWN THE SYSTEM TASKS...\n");
  

  sys_atrunlevel(exiting, NULL, RUNLEVEL_SHUTDOWN);
    

  hard_task_default_model(m);
  hard_task_def_wcet(m,ASTER_WCET);
  hard_task_def_mit(m,10000);
  hard_task_def_group(m,1);
  hard_task_def_ctrl_jet(m);
  p1 = task_create("Aster",aster,&m,NULL);
  if (p1 == -1) {
    sys_shutdown_message("(main): Could not create task <aster> ...");
    exit(0);
  }


  soft_task_default_model(m_soft);
  soft_task_def_met(m_soft,1000);
  soft_task_def_period(m_soft,100000);
  soft_task_def_group(m_soft,1);
  soft_task_def_aperiodic(m_soft);
  soft_task_def_system(m_soft);
  soft_task_def_ctrl_jet(m_soft);
  soft_task_def_nokill(m_soft);
  p3 = task_create("JetControl",jetcontrol,&m_soft,NULL);
  if (p3 == -1) {
    sys_shutdown_message("(main): Could not create task <JetControl> ...");
    exit(0);
  }


  /* These are the tasks that simulate the system tasks */

  nrt_task_default_model(m_nrt);
  nrt_task_def_ctrl_jet(m_nrt);
  nrt_task_def_system(m_nrt);
  nrt_task_def_group(m_nrt,1);

  cprintf("creating system tasks: ");
  for (i=0; i<APER_MAX; i++) {
    char systemtaskname[]="mysystemtask0";
    nrt_task_def_arg(m_nrt,(void *)i);
    systemtaskname[12]='0'+i;
    aper_table[i] = task_create(systemtaskname,system_asteroide,&m_nrt,NULL);
    if (aper_table[i] == -1) {
      sys_shutdown_message("(main): Could not create task <mysystemtask> ...");      exit(0);
    }
    else
      cprintf(" %d",i);
  }
  cprintf("\n");



  group_activate(1);
  return 0;
}

