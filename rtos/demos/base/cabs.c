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
 * Copyright (C) 2000 Giorgio Buttazzo, Paolo Gai
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
 * CVS :        $Id: cabs.c,v 1.9 2005/02/25 11:10:46 pj Exp $
 */

/*--------------------------------------------------------------*/
/*		      TEST ON CABS				*/
/*--------------------------------------------------------------*/

#include <kernel/kern.h>
#include <cabs/cabs/cabs.h>
#include <string.h>

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>

#define	NCAB	4		/* max number of CABs	*/
#define	NCAR	26		/* generated characters	*/

#define	YP	32		/* level of arrows	*/
#define	R	20		/* task radius		*/
#define	YY	(YP+R+32)	/* level of writing	*/
#define	DELTA	(2*R+72)	/* total channel hight	*/
#define	X1	120		/* start column for P1	*/
#define	X2	360		/* start column for P2	*/

#define	XP1	(X1+64)		/* X position of task 1	*/
#define	XP2	(X2+64)		/* X position of task 2	*/
#define	XC	(XP1+96)	/* X position of CAB	*/
#define	L	52		/* CAB rectangle length	*/

void	my_exit(KEY_EVT *k);
void	draw_channel(int i);
void	create_channel(int i);
void	get_data();

TASK	producer(void *arg);
TASK	consumer(void *arg);

char	*cname[NCAB] = {"cab1", "cab2", "cab3", "cab4"};
char	*pname1[NCAB] = {"wr1", "wr2", "wr3", "wr4"};
char	*pname2[NCAB] = {"rd1", "rd2", "rd3", "rd4"};

CAB	cid[NCAB];		/* CAB identifiers	*/
PID	p1[NCAB], p2[NCAB];	/* task identifiers	*/

/* Task Periods */
TIME	t1[NCAB] = {200000, 100000, 300000, 800000};
TIME    t2[NCAB] = {400000, 400000, 150000, 200000};

/* Task WCETS */
TIME	w1[NCAB] = {10000, 10000, 10000, 10000};
TIME    w2[NCAB] = {10000, 10000, 10000, 10000};

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

/*--------------------------------------------------------------*/
/* Main task							*/
/*--------------------------------------------------------------*/

/****************************** MAIN ******************************/

int main(int argc, char **argv)
{
    char c = 0;		/* character from keyboard	*/

    grx_clear(BLACK);

    grx_text("Press a key [1-4]", 10, 16, rgb16(255,255,255), 0);
    grx_text("to create a pair",  10, 24, rgb16(255,255,255), 0);
    grx_text("ESC to exit demo",  10, 48, rgb16(255,255,255), 0);

    while (c != 27) {
      c = keyb_getch(BLOCK);
      if ((c >= '1') && (c <= '1'+NCAB-1))
        create_channel(c-'1');
    }
  
    exit(0);

    return 0;
}


/*--------------------------------------------------------------*/
/* write data in a cab						*/
/*--------------------------------------------------------------*/

TASK	producer(void *arg)
{
int     i = (int)arg;
char	c;			/* message character		*/
char	*p;			/* pointer to a cab buffer	*/
char	s[2];			/* string to display		*/
int	k = 0;
int	x, y;
int	col = rgb16(0,0,255);
int	ybase = YY + i*DELTA;

	x = X1;
	y = ybase;
	s[1] = 0;

	k = 0;
	while (1) {
		c = 'A' + k;
		p = cab_reserve(cid[i]);
		*p = c;
		cab_putmes(cid[i], p);

		s[0] = c;
		k = (k + 1) % NCAR;
		grx_text(s,x,y,col,0);

		x += 8;
		if (x >= (X1 + NCAR*8)) {
			x = X1;
			y = y + 8;
			if (y >= ybase+16) {
				y = ybase;
			}
		}

		task_endcycle();
	}
}

/*--------------------------------------------------------------*/
/* read data from a cab						*/
/*--------------------------------------------------------------*/

TASK	consumer(void *arg)
{
int     i = (int)arg;
char	*p;
char	s[2];
int	x, y;
int	col = 13;
int	ybase = YY + i*DELTA;

	x = X2;
	y = ybase;
	s[1] = 0;

	while (1) {
		p = cab_getmes(cid[i]);
		s[0] = *p - 'A' + 'a';
		cab_unget(cid[i], p);

		grx_text(s,x,y,col,0);
		x += 8;

		if (x >= (X2 + NCAR*8)) {
			x = X2;
			y = y + 8;
			if (y >= ybase+16) {
				y = ybase;
				col = col % 15 + 1;
			}
		}
		task_endcycle();
	}
}

/*--------------------------------------------------------------*/
/* create the two tasks and a channel				*/
/*--------------------------------------------------------------*/

void	create_channel(int i)
{
        HARD_TASK_MODEL m;

	draw_channel(i);
	cid[i] = cab_create(cname[i], 1, 2);

        hard_task_default_model(m);
        hard_task_def_ctrl_jet (m);
        hard_task_def_arg      (m, (void *)i);
        hard_task_def_wcet     (m, w1[i]);
        hard_task_def_mit      (m, t1[i]);
        hard_task_def_usemath  (m);
        p1[i] = task_create(pname1[i], producer, &m, NULL);
        if (p1[i] == NIL) {
          sys_shutdown_message("Could not create task <producer>");
          exit(1);
	  return;
        }
        task_activate(p1[i]);

        hard_task_default_model(m);
        hard_task_def_ctrl_jet (m);
        hard_task_def_arg      (m, (void *)i);
        hard_task_def_wcet     (m, w2[i]);
        hard_task_def_mit      (m, t2[i]);
        hard_task_def_usemath  (m);
        p2[i] = task_create(pname2[i], consumer, &m, NULL);
        if (p2[i] == NIL) {
          sys_shutdown_message("Could not create task <consumer>");
          exit(1);
          return;
        }
        task_activate(p2[i]);
}

/*--------------------------------------------------------------*/
/* Disegna i processi e il canale di comunicazione		*/
/*--------------------------------------------------------------*/

void	draw_channel(int i)
{
char	buffer[32];			/* buffer per sprintf	*/
int	yc = YP + i*DELTA;		/* altezza del canale	*/

	grx_circle(XP1,yc,R,rgb16(255,0,0));
	grx_text("P1",XP1-8,yc-4,rgb16(255,255,255),0);

	grx_circle(XP2,yc,R,rgb16(255,0,0));
	grx_text("P2",XP2-8,yc-4,rgb16(255,255,255),0);

	grx_rect(XC,yc-R,XC+L,yc+R,rgb16(255,255,255));
	grx_text("CAB",XC+16,yc-4,rgb16(255,255,255),0);

	grx_line(XP1+R,yc,XC,yc,rgb16(255,255,255));
	grx_line(XC+L,yc,XP2-R,yc,rgb16(255,255,255));

	grx_text("T1 =          ms",X1+40,yc+R+16,rgb16(255,255,255),0);
	sprintf(buffer,"%ld", t1[i]);
	grx_text(buffer,X1+88,yc+R+16,rgb16(255,255,255),0);

	grx_text("T2 =          ms",X2+40,yc+R+16,rgb16(255,255,255),0);
	sprintf(buffer,"%ld", t2[i]);
	grx_text(buffer,X2+88,yc+R+16,rgb16(255,255,255),0);
}
