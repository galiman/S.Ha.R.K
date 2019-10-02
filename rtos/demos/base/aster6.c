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
 * Copyright (C) 2000 Paolo Gai, Gerardo Lamastra and Giuseppe Lipari
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
 * CVS :        $Id: aster6.c,v 1.8 2006/07/03 13:11:52 tullio Exp $

 this is a part of the classic Hartik demo Aster.

 it is based on test 17 (h), and the JobControl Task uses an
 SOFT_TASK_MODEL served by a polling server

*/

#include "kernel/kern.h"
#include "edf/edf/edf.h"
#include "cbs/cbs/cbs.h"

#include <drivers/shark_keyb26.h>

int num_aster = 0;
#define ASTER_LIM	60
#define DISPLAY_MAX     15
#define ASTER_MAX       70
#define STAT_Y           9

#define PER_MAX          5
#define APER_MAX         8

#define PER_WCET      6200
#define APER_WCET    18400
#define JET_WCET     10000

#define APER_REP     22000

PID aper_table[APER_MAX];

mutex_t m1;


#define PIMUTEX
//#define NPPMUTEX
//#define NOPMUTEX

//#define LONGSC

#ifdef LONGSC
#define SOFT_MET      3000 /* 12000 */
#define CLOCK_WCET     200 /* 300*/
#define ASTER_WCET     200 /* 300*/
#else
#define SOFT_MET     80000 /* 4500 */
#define CLOCK_WCET    2000 /* 200*/
#define ASTER_WCET    2000 /* 200*/
#endif

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
#ifdef LONGSC
      mutex_lock(&m1);
#endif
      for (j=0; j<load1; j++) {
	s[0] = '*' + rand() % 100;
#ifndef LONGSC
	mutex_lock(&m1);
#endif
	puts_xy(i,y,rand()%15+1,s);
#ifndef LONGSC
	mutex_unlock(&m1);
#endif
      }
#ifdef LONGSC
      mutex_unlock(&m1);
#endif

      task_activate(aper_table[rand()%APER_MAX]);
      task_endcycle();

      mutex_lock(&m1);
      puts_xy(i,y,WHITE," ");
      mutex_unlock(&m1);
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
#ifdef LONGSC
      mutex_lock(&m1);
#endif
      for (j=0; j<load1; j++) {
	s[0] = '*' + rand() % 100;
#ifndef LONGSC
	mutex_lock(&m1);
#endif
	puts_xy(i,y,rand()%15+1,s);
#ifndef LONGSC
	mutex_unlock(&m1);
#endif
      }
      s[0] = c;
#ifndef LONGSC
      mutex_lock(&m1);
#endif
      puts_xy(i,y,rand()%15+1,s);
      mutex_unlock(&m1);

      task_endcycle();

      mutex_lock(&m1);
      puts_xy(i,y,WHITE," ");
      mutex_unlock(&m1);
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

  /*for (;;)*/ {
    i = 1;
    while (i < ASTER_LIM) {
      load1 = 1000 + rand()%9000;
#ifdef LONGSC
      mutex_lock(&m1);
#endif
      for (j=0; j<load1; j++) {
	s[0] = '*' + rand() % 100;
#ifndef LONGSC
	mutex_lock(&m1);
#endif
	puts_xy(i,y,rand()%15+1,s);
#ifndef LONGSC
	mutex_unlock(&m1);
#endif
      }
      s[0] = 1;
#ifndef LONGSC
      mutex_lock(&m1);
#endif
      puts_xy(i,y,rand()%15+1,s);
      mutex_unlock(&m1);

      task_activate(aper_table[rand()%APER_MAX]);
      task_endcycle();

      mutex_lock(&m1);
      puts_xy(i,y,WHITE," ");
      mutex_unlock(&m1);
      i++;
    }
  }
  num_aster--;
  return 0;
}

TASK aster()
{
  PID p;

  //    HARD_TASK_MODEL m;
  SOFT_TASK_MODEL m_soft;
  int r;
  int x; // adaptive bandwidth...

  srand(7);

  soft_task_default_model(m_soft);
  soft_task_def_met(m_soft,SOFT_MET);
  soft_task_def_ctrl_jet(m_soft);

  x = 128; //64;

  while (1) {
    if (num_aster < ASTER_MAX) {
      r = (rand() % 200);

      soft_task_def_period(m_soft, (x+r)*1000);
      p = task_create("aaa",soft_aster,&m_soft,NULL);
      if (p == -1)
	{
	  if (x < 500 && errno != ENO_AVAIL_TASK)  x += 1;
	  mutex_lock(&m1);
	  printf_xy(62,3,WHITE,"adapt=%3u err=%d",
		    iq_query_first(&freedesc),errno);
	  mutex_unlock(&m1);
	}
      else {
	num_aster++;
	mutex_lock(&m1);
	printf_xy(62,3,WHITE,"adapt=%3u           ",x);//,errno);
	mutex_unlock(&m1);
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
    mutex_lock(&m1);
    printf_xy(62,1,WHITE,"%2d:%2d ast=%d",m,s, num_aster);
    printf_xy(62,2,WHITE,"Uedf=%12u",EDF_usedbandwidth(1));
    printf_xy(62,4,WHITE,"Ucbs=%12u",CBS_usedbandwidth(3));

    mutex_unlock(&m1);

    task_endcycle();

    if (++s > 59) {
      s = 0;
      m++;
    }
    mutex_lock(&m1);
    printf_xy(62,1,WHITE,"%2d:%2d ast=%d",m,s, num_aster);
    printf_xy(62,2,WHITE,"Uedf=%12u",EDF_usedbandwidth(1));
    printf_xy(62,4,WHITE,"Ucbs=%12u",CBS_usedbandwidth(3));
    mutex_unlock(&m1);
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

  mutex_lock(&m1);
  printf_xy(0,STAT_Y,WHITE,"PID ³ Mean T.³ Max T. ³ N.A. ³ Curr.   ³ Last1 ³ Last2 ³ Last3 ³ Last4 ³ Last5");
  mutex_unlock(&m1);

  for (;;) {
    for (i=0,p=0; i<DISPLAY_MAX+5 && p<MAX_PROC; p++) {
      if (jet_getstat(p, &sum, &max, &nact, &curr) == -1 /*||
							   (proc_table[p].pclass & 0xFF00) == APERIODIC_PCLASS ||
							   (proc_table[p].pclass & 0xFF00) == PERIODIC_PCLASS*/ ) continue;

      for (j=0; j<5; j++) last[j] = 0;
      jet_gettable(p, &last[0], 5);
      mutex_lock(&m1);
      if (proc_table[p].task_level == 3)
	printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
		  p, (int)sum/(nact==0 ? 1 : nact), (int)max, nact, (int)CBS_get_nact(2,p), (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
      //                   p, sum/(nact==0 ? 1 : nact), max, proc_table[p].avail_time, proc_table[p].status, proc_table[p].shadow, proc_table[p].timespec_priority.tv_sec,proc_table[p].timespec_priority.tv_nsec/1000 , CBS_get_nact(2,p), last[4]);
      else
	printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
		  p, (int)sum/(nact==0 ? 1 : nact), (int)max, (int)nact, (int)curr, (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
      //                   p, sum/(nact==0 ? 1 : nact), max, nact, proc_table[p].status, proc_table[p].shadow, proc_table[p].timespec_priority.tv_sec,proc_table[p].timespec_priority.tv_nsec/1000 , last[3], last[4]);
      mutex_unlock(&m1);
      i++;
    }
  }
}

void fine(KEY_EVT *e)
{

  exit(0);

}

int main(int argc, char **argv)
{
  PID p1,p2,p3; //,p4,p5,p6;
  HARD_TASK_MODEL m;
  //    NRT_TASK_MODEL m_nrt;
  SOFT_TASK_MODEL m_aper;
  SOFT_TASK_MODEL m_soft;
  //    int i;
  struct timespec fineprg;

#ifdef PIMUTEX
  PI_mutexattr_t a;
#endif

#ifdef NPPMUTEX
  NPP_mutexattr_t a;
#endif

#ifdef NOPMUTEX
  NOP_mutexattr_t a;
#endif

  KEY_EVT emerg;
  
  emerg.ascii = 'c';
  emerg.scan = KEY_C;
  emerg.flag = CNTL_BIT;
  emerg.status = KEY_PRESSED;
  keyb_hook(emerg,fine,FALSE);
	
	emerg.flag = CNTR_BIT;
  emerg.status = KEY_PRESSED;
  keyb_hook(emerg,fine,FALSE);

  clear();

  hard_task_default_model(m);
  hard_task_def_mit(m,100000);
  hard_task_def_wcet(m,ASTER_WCET);
  hard_task_def_group(m,1);
  hard_task_def_ctrl_jet(m);

  //    nrt_task_default_model(m_nrt);
  //    nrt_task_def_group(m_nrt,1);
  //    nrt_task_def_ctrl_jet(m_nrt);


  soft_task_default_model(m_aper);
  soft_task_def_group(m_aper,1);
  soft_task_def_ctrl_jet(m_aper);
  soft_task_def_aperiodic(m_aper);

  soft_task_default_model(m_soft);
  soft_task_def_period(m_soft,100000);
  soft_task_def_met(m_soft,JET_WCET);
  soft_task_def_group(m_soft,1);
  soft_task_def_ctrl_jet(m_soft);
  soft_task_def_aperiodic(m_soft);

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

  //    p3 = task_create("JetControl",jetcontrol,&m_nrt,NULL);
  p3 = task_create("JetControl",jetcontrol,&m_aper,NULL);
  if (p3 == -1) {
    sys_shutdown_message("test7.c(main): Could not create task <JetControl> ...");
    exit(1);
  }

  task_nopreempt();

#ifdef PIMUTEX
  PI_mutexattr_default(a);
#endif

#ifdef NPPMUTEX
  NPP_mutexattr_default(a);
#endif

#ifdef NOPMUTEX
  NOP_mutexattr_default(a);
#endif

  mutex_init(&m1, &a);

  fineprg.tv_sec = 20;
  fineprg.tv_nsec = 0;
  kern_event_post(&fineprg,(void (*)(void *))fine,NULL);
  group_activate(1);

  return 0;
}

