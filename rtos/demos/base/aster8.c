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
 * Copyright (C) 2000 Giorgio Buttazzo, Paolo Gai, Massimiliano Giorgi
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
 * CVS :        $Id: aster8.c,v 1.7 2006/07/03 12:53:13 tullio Exp $

 Test for Sporadic Server (ss): 

 this is a part of the classic Hartik demo Aster.

 it is based on test 17 (h), and the JobControl Task uses an
 SOFT_TASK_MODEL served by a sporadic server
 There are two "dummy" tasks that increment a counter and print
 the value. One uses a SOFT_TASK_MODEL served by sporadic server,
 the other uses a NRT_TASK_MODEL handled by RR module.

*/

#include "kernel/kern.h"
#include "edf/edf/edf.h"
#include "cbs/cbs/cbs.h"
#include "ss/ss/ss.h"

#include <drivers/shark_keyb26.h>

int num_aster = 0;
#define EDF_LEV		1
#define CBS_LEV		2
#define SS_LEV		3

#define ASTER_LIM	60
#define DISPLAY_MAX     8
#define ASTER_MAX       70
#define STAT_Y          9

#define PER_MAX          5
#define APER_MAX         8

#define PER_WCET     16000
#define APER_WCET    22000
#define JET_WCET     20000

#define APER_REP     22000

PID aper_table[APER_MAX];

mutex_t m1;


#define PIMUTEX
//#define PCMUTEX
//#define NPPMUTEX
//#define NOPMUTEX

#define LONGSC

#ifdef LONGSC
#define SOFT_MET      12000 /* 12000 */
#define CLOCK_WCET    300 /* 300*/
#define ASTER_WCET    300 /* 300*/
#else
#define SOFT_MET      5000 /* 4500 */
#define CLOCK_WCET    2000 /* 200*/
#define ASTER_WCET    2000 /* 200*/
#endif

PID p1,p2,p3,p4,p5;

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
          mutex_unlock(&m1);
        #endif
        mutex_lock(&m1);
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
        //mutex_lock(&m1);
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
			iq_query_first(&freedesc), errno);
              mutex_unlock(&m1);
            }
            else {
              num_aster++;
              mutex_lock(&m1);
              printf_xy(62,3,WHITE,"adapt=%3u           ",x);
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
	printf_xy(62,2,WHITE,"Uedf=%12u",EDF_usedbandwidth(EDF_LEV));
	printf_xy(62,4,WHITE,"Ucbs=%12u",CBS_usedbandwidth(CBS_LEV));
	printf_xy(62,5,WHITE,"CSss=%6d",SS_availCs(SS_LEV));
        mutex_unlock(&m1);

	task_endcycle();

	if (++s > 59) {
	    s = 0;
	    m++;
	}
        mutex_lock(&m1);
	printf_xy(62,1,WHITE,"%2d:%2d ast=%d",m,s, num_aster);
	printf_xy(62,2,WHITE,"Uedf=%12u",EDF_usedbandwidth(EDF_LEV));
	printf_xy(62,4,WHITE,"Ucbs=%12u",CBS_usedbandwidth(CBS_LEV));
	printf_xy(62,5,WHITE,"CSss=%6d",SS_availCs(SS_LEV));
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
       if (proc_table[p].task_level == 2)
         printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
                   p, (int)sum/(nact==0 ? 1 : nact), (int)max, nact, (int)CBS_get_nact(2,p), (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
//                   p, sum/(nact==0 ? 1 : nact), max, proc_table[p].avail_time, proc_table[p].status, proc_table[p].shadow, proc_table[p].timespec_priority.tv_sec,proc_table[p].timespec_priority.tv_nsec/1000 , CBS_get_nact(2,p), last[4]);
       else
         printf_xy(0,STAT_Y+i+1,WHITE,"%-3d ³ %-6d ³ %-6d ³ %-4d ³ %-7d ³ %-5d ³ %-5d ³ %-5d ³ %-5d ³ %-5d",
                   p, (int)sum/(nact==0 ? 1 : nact), (int)max, (int)nact, (int)curr, (int)last[0], (int)last[1], (int)last[2], (int)last[3], (int)last[4]);
//                   p, sum/(nact==0 ? 1 : nact), max, nact, proc_table[p].status, proc_table[p].shadow, proc_table[p].timespec_priority.tv_sec,proc_table[p].timespec_priority.tv_nsec/1000 , last[3], last[4]);
       mutex_unlock(&m1);
       i++;
       task_activate(p3);
       task_endcycle();
    }
  }
}

void fine(KEY_EVT *e)
{

  exit(0);

}

void mydummyaper(void) {
	int i=0;
	while(1) {
		mutex_lock(&m1);
        	printf_xy(1,24,RED,"dummyAPER pid=%d: %d",p4,i++);
		mutex_unlock(&m1);
		task_activate(p4);
		task_endcycle();
	}
}

void mydummynrt(void) {
	int i=0;
	while(1) {
		mutex_lock(&m1);
		printf_xy(40,24,RED,"dummyNRT pid=%d: %d",p5,i++);
		mutex_unlock(&m1);
	}
}

int main(int argc, char **argv)
{
//    PID p1,p2,p5;
    HARD_TASK_MODEL m;
    NRT_TASK_MODEL m_nrt;
    SOFT_TASK_MODEL m_aper;
    SOFT_TASK_MODEL m_soft;

    #ifdef PIMUTEX
    PI_mutexattr_t a;
    #endif

    #ifdef PCMUTEX
    PC_mutexattr_t a;
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

    cprintf("Press Ctrl-C to end the demo...");

    hard_task_default_model(m);
    hard_task_def_mit(m,100000);
    hard_task_def_wcet(m,ASTER_WCET);
    hard_task_def_group(m,1);
    hard_task_def_ctrl_jet(m);

    nrt_task_default_model(m_nrt);
    nrt_task_def_group(m_nrt,1);
    nrt_task_def_ctrl_jet(m_nrt);

    soft_task_default_model(m_aper);
    soft_task_def_group(m_aper,1);
    soft_task_def_ctrl_jet(m_aper);
    soft_task_def_aperiodic(m_aper);

    soft_task_default_model(m_soft);
    soft_task_def_period(m_soft,10000);
    soft_task_def_met(m_soft,JET_WCET);
    soft_task_def_group(m_soft,1);
    soft_task_def_ctrl_jet(m_soft);
    soft_task_def_aperiodic(m_soft);

    p1 = task_create("Aster",aster,&m,NULL);
    if (p1 == -1) {
	sys_shutdown_message("test7.c(main): Could not create task <aster> ...");
	exit(1);
	return 0;
    }

    hard_task_def_mit(m,50000);
    hard_task_def_wcet(m,CLOCK_WCET);
    p2 = task_create("Clock",clock,&m,NULL);
    if (p2 == -1) {
	sys_shutdown_message("test7.c(main): Could not create task <Clock> ...");
	exit(1);
	return 0;
    }

    p3 = task_create("JetControl",jetcontrol,&m_aper,NULL);
    if (p3 == -1) {
	sys_shutdown_message("test7.c(main): Could not create task <JetControl> ...");
	exit(1);
	return 0;
    }

    p4 = task_create("MyDummyAper",(void *(*)(void*))mydummyaper,&m_aper,NULL);
    if (p4 == -1) {
	sys_shutdown_message("Could not create task <MyDummyAper> ...");
	exit(1);
	return 0;
    }

    p5 = task_create("MyDummyNRT",(void *(*)(void*))mydummynrt,&m_nrt,NULL);
    if (p5 == -1) {
	sys_shutdown_message("Could not create task <MyDummyNRT> ...");
	exit(1);
	return 0;
    }

    #ifdef PIMUTEX
    PI_mutexattr_default(a);
    #endif

    #ifdef PCMUTEX
    PC_mutexattr_default(a);
    #endif

    #ifdef NPPMUTEX
    NPP_mutexattr_default(a);
    #endif

    #ifdef NOPMUTEX
    NOP_mutexattr_default(a);
    #endif

    mutex_init(&m1, &a);

    group_activate(1);
    return 0;
}

