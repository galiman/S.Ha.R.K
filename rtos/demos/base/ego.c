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
 CVS :        $Id: ego.c,v 1.8 2006/07/03 12:47:33 tullio Exp $

 File:        $File$
 Revision:    $Revision: 1.8 $
 Last update: $Date: 2006/07/03 12:47:33 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai and Giorgio Buttazzo
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

/****************************************************************/
/*	PERIODIC PROCESS TEST                                   */
/****************************************************************/

#include <kernel/kern.h>

#include <drivers/shark_keyb26.h>
#include <drivers/shark_fb26.h>
#define X0	10

/* task periods */
#define PERIOD_T1 100000
#define PERIOD_T2 200000
#define PERIOD_T3 300000

/* X position of the text printed by each task */
int	y[3] = {100, 180, 260};

/* text printed by each task */
char	talk[3][50] = { "I am ego1 and I print a character every 100 ms",
                        "I am ego2 and I print a character every 200 ms",
                        "I am ego3 and I print a character every 300 ms"};

/***************************************************************/

TASK	ego(void *arg)
{
int     i = (int)arg;
int     leng;
char	s[2];
int	x;
int	j = 0;

        /* compute the length of the string to print */
	leng = 0;
	while (talk[i][leng] != 0) leng++;

	x = X0;
	s[1] = 0;
        task_endcycle();

	while (1) {
		s[0] = talk[i][j];
		grx_text(s,x,y[i],rgb16(255,255,255),0);
		x += 8;
		if (++j == leng) {
			j = 0;
			x = X0;
			y[i] += 8;
                        if (y[i]>340) y[i]=100;
		}
		task_endcycle();
	}
}


/****************************************************************/

/* This function is called when Alt-X is pressed.
*/
void my_end(KEY_EVT* e)
{
	exit(0);
}

/******************************************************************/

/* This function is called when the system exit correctly after Alt-X.
   It exits from the graphic mode and then it prints a small greeting.
   Note that:
   - The function calls grx_exit, so it must be registered using
     RUNLEVEL_BEFORE_EXIT (RUNLEVEL_AFTER_EXIT does not work because
     at that point the kernel is already returned in real mode!!!)
   - When an exception is raised, the exception handler is called.
     Since the exception handler already exits from the graphic mode,
     this funcion has not to be called. For this reason:
     . we registered byebye using the flag NO_AT_ABORT
     . the exception handler exits using exit; in that way byebye is
       NOT called
*/

/****************************** MAIN ******************************/

int main(int argc, char **argv)
{
  PID	          pid1, pid2, pid3;
  KEY_EVT	  emerg;
  HARD_TASK_MODEL m1, m2, m3;

        /* set the keyboard handler to exit correctly */
	emerg.ascii = 'c';
	emerg.scan = KEY_C;
	emerg.flag = CNTL_BIT;
	emerg.status = KEY_PRESSED;
	keyb_hook(emerg,my_end,FALSE);
	
	emerg.flag = CNTR_BIT;
  emerg.status = KEY_PRESSED;
  keyb_hook(emerg,my_end,FALSE);

        /* a small banner */
        grx_text("EGO Test",8,8,rgb16(255,255,255),0);
        grx_text("Press Ctrl-C to exit",8,16,rgb16(255,255,255),0);

        /* ego1 creation */
        hard_task_default_model(m1);
        hard_task_def_ctrl_jet (m1);
        hard_task_def_arg      (m1, (void *)0);
        hard_task_def_wcet     (m1, 5000);
        hard_task_def_mit      (m1, PERIOD_T1);
        hard_task_def_group    (m1,1);
        pid1 = task_create("ego1", ego, &m1, NULL);
        if (pid1 == NIL) {
          sys_shutdown_message("Could not create task <ego1>");
	  exit(1);
        }

        /* ego2 creation */
        hard_task_default_model(m2);
        hard_task_def_ctrl_jet (m2);
        hard_task_def_arg      (m2, (void *)1);
        hard_task_def_wcet     (m2, 5000);
        hard_task_def_mit      (m2, PERIOD_T2);
        hard_task_def_group    (m2,1);
        pid2 = task_create("ego2", ego, &m2, NULL);
        if (pid2 == NIL) {
          sys_shutdown_message("Could not create task <ego2>");
	  exit(1);
        }

        /* ego3 creation */
        hard_task_default_model(m3);
        hard_task_def_ctrl_jet (m3);
        hard_task_def_arg      (m3, (void *)2);
        hard_task_def_wcet     (m3, 5000);
        hard_task_def_mit      (m3, PERIOD_T3);
        hard_task_def_group    (m3,1);
        pid3 = task_create("ego3", ego, &m3, NULL);
        if (pid3 == NIL) {
          sys_shutdown_message("Could not create task <ego3>");
	  exit(1);
        }

        /* and finally we activate the three threads... */
	group_activate(1);

        /*
           now the task main ends, but the system does not shutdown because
           there are the three task ego1, ego2, and ego3 running.

           The demo will finish if a Alt-X key is pressed.
        */

        return 0;
}

/****************************************************************/
