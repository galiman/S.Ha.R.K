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

/* CVS :        $Id: dummy.c,v 1.2 2006/07/03 15:10:15 tullio Exp $ */

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

PID crea_soft_dummy_radar()
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_level(m,0);
  soft_task_def_periodic(m);
  soft_task_def_period(m,DUMMY_RADAR_PERIOD);
  soft_task_def_wcet(m,DUMMY_RADAR_WCET);
  soft_task_def_met(m,DUMMY_RADAR_MET);

  pid = task_create("dummy_radar", dummy_radar, &m, NULL);
  return pid;
}

TASK dummy_radar(void *argv)
{
  WORD i,r,r_max,pos;
  BYTE red[3]={250, 150,50};
  int  d[3]=  {+10,+10,+10};

  r =     0;
  r_max = ballframe_radar->r+1;

  while(1){
    for(i=0; i<3; i++) {
      if (red[i]<50)  d[i]=+10;
      if (red[i]>250) d[i]=-50;
      red[i] += d[i];
      grx_disc(ballframe_lucerossa[i]->cx,
               ballframe_lucerossa[i]->cy,
               ballframe_lucerossa[i]->r,
               FAB_rgb(red[i],50,50));
      }
    grx_disc(ballframe_radar->cx,
             ballframe_radar->cy,
             r_max, FAB_blue);
    grx_circle(ballframe_radar->cx,
               ballframe_radar->cy,
               (r+9)%r_max, FAB_white);
    grx_circle(ballframe_radar->cx,
               ballframe_radar->cy,
               (r+5)%r_max, FAB_rgb(200,200,255));
    grx_circle(ballframe_radar->cx,
               ballframe_radar->cy,
               r%r_max, FAB_rgb(100,100,255));
    for (i=0; i<AEREO_N_MAX; i++) {
      if (aereo_table[i].status
         &&aereo_table[i].x>X0
         &&aereo_table[i].y>Y0) {
            pos = ballframe_radar->cx - ballframe_radar->r
                + (aereo_table[i].x-X0)*2*ballframe_radar->r/500;
            grx_plot(pos,ballframe_radar->cy,FAB_yellow);
            }
      }
    r++;

  task_endcycle();
  }
}
