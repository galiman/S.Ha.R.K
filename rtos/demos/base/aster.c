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
 * CVS :        $Id: aster.c,v 1.6 2005/02/25 11:10:46 pj Exp $

 Author:      Gerardo Lamastra
 Giuseppe Lipari
 Date:        1/10/96

 File:        Aster.C
 Revision:    1.6

*/

/*
   Well, this is only a stupid demo which intend to show many
   HARTIK+ capabilities; the application is structured in the followig
   way: there is an ASTER task wich randomly creates some ASTEROID tasks
   which are displayed into the first window; each task is HARD/PERIODIC
   and auto-kills itself when it reaches the window end!
   An other couple of tasks, TITLE & PUT give an example of port
   communication facility; the server task creates the port, the client
   task connect to it and uses the server to accomplish some stuff.
   Port can be declared READ/WRITE and can model ONE-TO-ONE communication
   or MANY-TO-ONE communication.
   Finally a second couple of tasks realizes a communiation through CABs;
   each time a key is pressed, the ascii code is posted into the CAB by the
   CCC task while the second task, WRITE, displays it on the screen and
   perform other silly actions.
   Finally a CLOCK task is implemented to test system clock.
   Please note that usually the HARTIK+ application is made up of a task
   group which interacts among them, while the main() function, which
   became a task itself when the kernel is activated, is suspended until
   the system is ready to terminate; the MAIN task can also be used to make
   other background activities, but it should not be killed; when the
   application terminates, the control is passed to MAIN which kills
   everybody, shut down the system and can handle other operations using
   the services available with the previou operating system (I.E. the DOS).
   If you need to manage sudden abort/exception you should install your own
   exception handler and raise it through the exc_raise() primitive to
   make the system abort safely!
   Remember that the exit functions posted through sys_atexit() will be
   executed in both cases, to allow clean system shutdown.
*/

#include <kernel/kern.h>
#include <sem/sem/sem.h>
#include <hartport/hartport/hartport.h>
#include <cabs/cabs/cabs.h>
#include <string.h>

#include <drivers/shark_keyb26.h>

//#define __VPAGING__

#include <drivers/crtwin.h>

int num_aster = 0;
#define ASTER_LIM	67

CAB cc;
BYTE esc = FALSE;

TASK asteroide(void)
{
  int i = 1;
  int y = rand() % 7 + 1;
  while (i < ASTER_LIM) {
    puts_xy(i,y,WHITE,"*");
    task_endcycle();

    puts_xy(i,y,WHITE," ");
    i++;
  }
  num_aster--;
  return 0;
}

DWORD taskCreated = 0;

TASK aster(void)
{
  PID p;
  SOFT_TASK_MODEL m_soft;
  int r;
  WIN w;

  win_init(&w,0,0,ASTER_LIM,8);
  win_frame(&w,BLACK,WHITE,"Asteroids",2);

  soft_task_default_model(m_soft);
  soft_task_def_met(m_soft,2000);
  soft_task_def_ctrl_jet(m_soft);

  srand(7);
  while (1) {
    if (num_aster < 5) {
      r = (rand() % 50) - 25;
      soft_task_def_arg(m_soft,(void *)((rand() % 7)+1));
      soft_task_def_period(m_soft,(50 + r)*1000);
      p = task_create("aaa",asteroide,(TASK_MODEL *)&m_soft,NULL);
      taskCreated++;
      task_activate(p);
      num_aster++;
    }

    task_endcycle();
  }
}

TASK clock()
{
  WIN w;
  int s = 0, m = 0;

  win_init(&w,68,0,11,2);
  win_frame(&w,BLACK,WHITE,"Clk",1);

  while(1) {
    printf_xy(70,1,WHITE,"%2d : %2d",m,s);
    task_endcycle();

    if (++s > 59) {
      s = 0;
      m++;
    }
    printf_xy(70,1,WHITE,"%2d : %2d",m,s);
    task_endcycle();
  }
}

TASK title()
{
  PORT t;
  WIN w;
  int i,pos = 77;
  char msg[85],tmp[85],ss[2];
  BYTE c;
    
  win_init(&w,0,9,79,2);
  win_frame(&w,BLACK,WHITE,"Title",2);

  for (i=0; i < 77; i++) msg[i] = ' ';
  msg[77] = 0;

  t = port_connect("title",1,STREAM,READ);

  while (1) {
    port_receive(t,&c,BLOCK);
    ss[0] = c;
    ss[1] = 0;
    strcat(msg,ss);
    puts_xy(1,10,WHITE,msg);
    pos++;
    if (pos > 77) {
      strcpy(tmp,&(msg[1]));
      tmp[pos-1] = 0;
      pos -= 1;
      strcpy(msg,tmp);
    }
    task_endcycle();
  }
}

#define STR "..................... S.Ha.R.K. ....................."\
	    "              Guarantees hard tasks                "\
	    "          Includes soft periodic tasks             "\
	    "TB server for decrementing the aperiodic response time       "\
	    "SRP for both hard & soft aperiodic tasks        "\
	    "Portability toward other compilers/system       "\
	    "                                                             "\
	    "Programmers :    Gerardo Lamastra (lamastra@sssup2.sssup.it)    "\
	    "    Giuseppe Lipari (lipari@sssup2.sssup.it)        "\
	    "Research coordinator: Giorgio Buttazzo (giorgio@sssup1.sssup.it)"\
	    "                                                   "\
	    "                                                   "\
	    "                                                   "

static char GreetMsg[1600];

TASK put(void)
{
  PORT p;

  strcpy(GreetMsg,STR);
    
  p = port_create("title",strlen(GreetMsg),1,STREAM,WRITE);
  while(1) {
    port_send(p,GreetMsg,BLOCK);
    task_endcycle();
  }
}

TASK ccc(void)
{
  WIN w;
  char *m;

  win_init(&w,68,3,10,3);
  win_frame(&w,BLACK,WHITE,"CCC",2);
  puts_xy(70,4,WHITE,"Cab");

  while(1) {
    m = cab_getmes(cc);
    puts_xy(72,5,WHITE,m);
    cab_unget(cc,m);
    task_endcycle();
  }
}
    
    
TASK write_keyb()
{
  BYTE c;
  char *msg;

  while (1) {
    c = keyb_getchar();
    if (c == ESC) {
      esc = TRUE;
      task_endcycle();
    }
    else {
#ifdef __VPAGING__
      if (c == 's') {
	if (get_visual_page() == 0) set_visual_page(1);
	else if (get_visual_page() == 1) set_visual_page(0);
      }
#endif
      msg = cab_reserve(cc);
      msg[0] = c;
      msg[1] = 0;
      cab_putmes(cc,msg);
    }
  }
}

#define DELTA 200000.0
double carico(double rif,BYTE init)
{
  double i;
  DWORD t1 = 0,t2 = 1000;
  double u;

  i = 0.0;
  do {
    i += 1;
  } while (i <= DELTA);

  u = i / ((double) (t2 - t1));

  if (init) return u;
  else return (1.0 - u/rif);
}

int main(int argc, char **argv)
{
  PID p1,p2,p3,p4,p5,p6;

  HARD_TASK_MODEL m_per;
  SOFT_TASK_MODEL m_soft;
  NRT_TASK_MODEL m_nrt;

  struct timespec t;
    
#ifdef __VPAGING__
  set_active_page(1);
  set_visual_page(1);
#endif
        
  CRSR_OFF();
  clear();
  puts_xy(0,20,WHITE,"Press ESC to exit demo.");
  cc = cab_create("Cab",2,2);

  soft_task_default_model(m_soft);
  soft_task_def_period(m_soft,500000);
  soft_task_def_met(m_soft,1000);
  soft_task_def_group(m_soft, 1);
  p1 = task_create("Aster",aster,&m_soft,NULL);
  if (p1 == -1) {
    perror("Aster.C(main): Could not create task <aster>");
    exit(-1);
  }

  hard_task_default_model(m_per);
  hard_task_def_mit(m_per,500000);
  hard_task_def_wcet(m_per,1000);
  hard_task_def_group(m_per, 1);
  p2 = task_create("Clock",clock,&m_per,NULL);
  if (p2 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <Clock>");
    exit(1);
  }

  soft_task_def_period(m_soft, 50000);
  p3 = task_create("Title",title,&m_soft, NULL);
  if (p3 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <Title>");
    exit(1);
  }

  soft_task_def_period(m_soft, 1000000);
  p4 = task_create("Put",put,&m_soft, NULL);
  if (p4 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <Put>");
    exit(1);
  }

  nrt_task_default_model(m_nrt);
  nrt_task_def_group(m_nrt, 1);
  p5 = task_create("Write",write_keyb,&m_nrt,NULL);
  if (p5 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <Write>");
    exit(1);
  }

  hard_task_def_mit(m_per, 50000);
  p6 = task_create("CabTask",ccc,&m_per,NULL);
  if (p6 == -1) {
    sys_shutdown_message("Aster.C(main): Could not create task <CabTask>\n");
    exit(1);
  }

  group_activate(1);

  while (!esc) {
    sys_gettime(&t);
    printf_xy(0,21,WHITE,"Clock : %-9ds %-9dns",(int)t.tv_sec, (int)t.tv_nsec);
  }

  group_kill(1);
  clear();
  CRSR_STD();
#ifdef __VPAGING__
  set_active_page(0);
  set_visual_page(0);
#endif

  exit(0);

  return 0;

}

