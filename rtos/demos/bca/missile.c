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

/* CVS :        $Id: missile.c,v 1.4 2006/07/03 15:10:15 tullio Exp $ */

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

PID crea_hard_missile(int index);
PID crea_soft_missile(int index);
PID crea_soft_esplo(int index);

TASK missile(void *arg)
{
  PID        esplo_pid;
  int        img;
  int        i, index;    // indice nella cannone_table
  int        distx, disty;
  int        xx0,yy0,xx1,yy1;
  double     vel, acc;
  double     a;
  double     y;
  double     x;
  double     old_x;    // [pixel]: conserva valore x prima di aggiornare
  double     old_y;
  double     dx;       // [pixel/us]: spostamento periodico
  double     dy;
  int        first_time;
  int killing;

  index = (int)arg;
  img=0;
  y = MISSILE_Y_MAX;
  x = cannone_table[index].x;
  a = FAB_rad(270);
  vel = MISSILE_V_MIN;
  acc = MISSILE_ACC_MIN;
  dx = 0;
  dy = 0;

  first_time = 1;
  killing = 0;

  while(1){

   old_x = x;
   old_y = y;

   if (vel<MISSILE_V_MAX) {
      vel += acc*MISSILE_PERIOD/(60*60*1000000.0);
      if (vel>MISSILE_V_MAX) vel = MISSILE_V_MAX;
      }
   if (acc<MISSILE_ACC_MAX) {
      acc += MISSILE_ACC_MIN;
      if (acc>MISSILE_ACC_MAX) acc = MISSILE_ACC_MAX;
      }

   cannone_table[index].missile_vel=vel;

   dx = cos(a)*vel * (X1-X0) / (double)BASE_L
        * (MISSILE_PERIOD / (60*60*1000000.0));
   dy = sin(a)*vel * (Y1-Y0) / ((double)BASE_H/1000)
        * (MISSILE_PERIOD / (60*60*1000000.0));

   x += dx;
   y += dy;

   xx0 = old_x - MISSILE_LX/2;
   yy0 = old_y - MISSILE_LY/2;
   xx1 = xx0   + MISSILE_LX-1;
   yy1 = yy0   + MISSILE_LY-1;

   if (x<MISSILE_X_MIN || x>MISSILE_X_MAX
     || y<MISSILE_Y_MIN || y>MISSILE_Y_MAX) {
      killing = 1;
      }

   if (killing) {
      if (!first_time) {
          mutex_lock(&grx_mutex);
          FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,yy1);
          mutex_unlock(&grx_mutex);
          }
      cannone_table[index].fire = 0;
      //il task cannone si accorge che il suo missile ha finito
      // quindi ne ricreer… uno nuovo in caso di bersaglio!!!
      return NULL;
      }

   if ( (int)x != (int)old_x
      ||(int)y != (int)old_y ) {//...se c'Š lo spostamento reale
                                //        di almeno un pixel...
     mutex_lock(&grx_mutex);   //...aggiorna disegno...
     if (first_time) {
         first_time=0;
         }
     else {
         if (yy1<Y1-CANNONE_LY) FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,yy1);
         else                   FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,Y1-CANNONE_LY);
         }

     FAB_image_put_within(image_missile[img++], x - MISSILE_LX/2, y - MISSILE_LY/2,
                                              X0, Y0,
                                              X1, Y1-CANNONE_LY);
     mutex_unlock(&grx_mutex);
     if (img==2) img = 0;
     }

   //Ha colpito un aereo?
   for (i=0; i<AEREO_N_MAX; i++)
       if (aereo_table[i].status){
          distx = aereo_table[i].x-x;
          disty = aereo_table[i].y-y;
          if (distx<0) distx *= -1;
          if (disty<0) disty *= -1;
          if ( distx<(AEREO_LX+MISSILE_LX)/2-1
             &&disty<(AEREO_LY+MISSILE_LY)/2-1 ) { // BERSAGLIO COLPITO!
               esplo_pid = crea_soft_esplo(i);
               task_activate(esplo_pid);
               killing=1;
               aereo_table[i].killing=1;
               break;
               }
          }

   task_endcycle();
 }

return NULL;
}


PID crea_hard_missile(int index)
{
  HARD_TASK_MODEL m;
  PID pid;

  hard_task_default_model(m);
  hard_task_def_level(m,1);
  hard_task_def_arg(m,(void*)index);
  hard_task_def_periodic(m);
  hard_task_def_wcet(m, MISSILE_WCET);
  hard_task_def_mit(m,MISSILE_PERIOD);

  pid = task_create("hard_missile", missile, &m, NULL);
  return pid;
}

PID crea_soft_missile(int index)
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_level(m,2);
  soft_task_def_arg(m,(void*)index);
  soft_task_def_periodic(m);
  soft_task_def_period(m,MISSILE_PERIOD);
  soft_task_def_wcet(m, MISSILE_WCET);
  soft_task_def_met(m,MISSILE_MET);

  pid = task_create("soft_missile", missile, &m, NULL);
  return pid;
}
