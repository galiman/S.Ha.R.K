/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/* CVS :        $Id: control.c,v 1.3 2006/07/03 15:34:08 tullio Exp $ */

/*
 * Copyright (C) 2000 Fabio Calabrese <fabiocalabrese77@yahoo.it>
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

#include "bca.h"

PID crea_soft_control()
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_periodic(m);
  soft_task_def_period(m,CONTROL_PERIOD);
  soft_task_def_wcet(m,CONTROL_WCET);
  soft_task_def_met(m,CONTROL_MET);

  pid = task_create("control", control, &m, NULL);
  return pid;
}

TASK control(void *argv)
{
    int i, x0, y0;
    int rect1_x0, rect1_y0, rect1_x1, rect1_y1;
    int rect2_x0, rect2_y0, rect2_x1, rect2_y1;
    char * str = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

   x0=frame_control->x0;
   y0=frame_control->y0;
   rect1_x0=674;
   rect1_y0=y0+19;
   rect1_x1=758;
   rect1_y1=y0+24;
   rect2_x0=690;
   rect2_y0=frame_control->y1-81+3;
   rect2_x1=758;
   rect2_y1=frame_control->y1-81+2+6;

  while(1){

   sprintf(str,"PLANE:           %02d",aereo_count);
   grx_text(str,x0+2,y0+18,FAB_white,FAB_blue);
   grx_box(rect1_x0,rect1_y0,rect1_x1,rect1_y1,FRAME_CONTROL_BACK_COLOR);
   grx_box(rect1_x0,rect1_y0,
           rect1_x0+(rect1_x1-rect1_x0)*aereo_count/AEREO_N_MAX,rect1_y1,
           FAB_yellow);
   for (i=0; i<AEREO_N_MAX; i++){
     if (aereo_table[i].status)
         if (i==0) {
           sprintf(str,"HARD  vel=%04.1fkm/h",aereo_table[i].vel);
           grx_text(str,
                  x0+2,y0+28+8*i,
                  FAB_white,FRAME_CONTROL_BACK_COLOR);
           grx_box(x0+36,y0+29+8*i,
                 x0+45,y0+26+8*(i+1),
                 aereo_table[i].color);
           }
         else {
           sprintf(str,"Soft  vel=%04.1fkm/h",aereo_table[i].vel);
           grx_text(str,
                  x0+2,y0+28+8*i,
                  FAB_white,FRAME_CONTROL_BACK_COLOR);
           grx_box(x0+36,y0+29+8*i,
                 x0+45,y0+26+8*(i+1),
                 aereo_table[i].color);
           }
     else grx_text("                   ",
                  x0+2,y0+28+8*i,
                  FAB_white,FRAME_CONTROL_BACK_COLOR);
     }

   sprintf(str,"CANNONS:         %02d",cannone_count);
   grx_text(str,x0+2,frame_control->y1-81+2,FAB_white,FAB_blue);
   grx_box(rect2_x0,rect2_y0,rect2_x1,rect2_y1,FRAME_CONTROL_BACK_COLOR);
   grx_box(rect2_x0,rect2_y0,
           rect2_x0+(rect2_x1-rect2_x0)*cannone_count/CANNONE_N_MAX,rect2_y1,
           FAB_yellow);

   for (i=0; i<CANNONE_N_MAX; i++)
     if (cannone_table[i].status)  {
       if (cannone_table[i].fire) {
           if (i==0) {
             grx_text("                   ",x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);
             sprintf(str,"HARD vel=%-5.1fkm/h",cannone_table[i].missile_vel);
             grx_text(str,x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);
             }
           else {
             grx_text("                   ",x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);
             sprintf(str,"Soft vel=%-5.1fkm/h",cannone_table[i].missile_vel);
             grx_text(str,x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);
             }
           }
       else  {
           if (i==0) grx_text("HARD               ",x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);
           else      grx_text("Soft               ",x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);
           }
       }
     else grx_text("                   ",x0+2,frame_control->y1-81+12+8*i,FAB_white,FRAME_CONTROL_BACK_COLOR);

  task_endcycle();
  }
}
