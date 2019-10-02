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

/* CVS :        $Id: aereo.c,v 1.3 2006/07/03 15:10:15 tullio Exp $ */

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

TASK aereo_creator(void * arg)
{ int             i, index, last;
  WORD            count;
  PID             pid=-1;
  COLOR           col;
  double          vel;
  int             dir;
  WORD            h;
  double          l;
  WORD            x, y;

  while(1){

    count=0;
    index=-1;
    last=-1;

    for (i=0; i<AEREO_N_MAX; i++)  // Conteggio aerei attivi
      if (aereo_table[i].status) {
         count++;
         last=i;          //...ultima posizione occupata in tabella...
         }
      else {
         if (index==-1) index=i;   //...prima posizione libera in tabella...
         }

    if (count<aereo_count && index!=-1) { // Richiesta nuovo aereo
      aereo_table[index].status =  1;     //...occupa posizione...
      aereo_table[index].killing = 0;     //...disattiva richiesta di kill...
      col = FAB_rgb(FAB_int_rand(50,255), //...nuovo colore...
                    FAB_int_rand(50,255),
                    FAB_int_rand(50,255));
      vel = FAB_double_rand(AEREO_V_MIN,AEREO_V_MAX); //...velocita'...
      dir = FAB_sign_rand();                          //...direzione...
      h   = FAB_int_rand(AEREO_H_MIN,AEREO_H_MAX);    //...altezza...
      y   = Y0 + 500 - h*500/BASE_H;
      l   = (dir>0) ? AEREO_L_MIN : AEREO_L_MAX;      //...posizione iniz...
      x   = (dir>0) ? AEREO_X_MIN : AEREO_X_MAX;
      // ... settaggio valori in tabella ...
      aereo_table[index].color = col;
      if (index>0) {
        if (dir>0) FAB_image_copy(image_aereo[0],aereo_table[index].image);
        else       FAB_image_copy(image_aereo[1],aereo_table[index].image);
        }
      else {
        if (dir>0) FAB_image_copy(image_aereo[2],aereo_table[index].image);
        else       FAB_image_copy(image_aereo[3],aereo_table[index].image);
        }
      FAB_image_color_change(aereo_table[index].image,AEREO_BASE_COL,col);
      aereo_table[index].vel = vel;
      aereo_table[index].dir = dir;
      aereo_table[index].h   = h;
      aereo_table[index].l   = l;
      aereo_table[index].x   = x;
      aereo_table[index].y   = y;
      if (index==0) pid = crea_hard_aereo(index); // sempre il 1ø hard
      else          pid = crea_soft_aereo(index); // tutti gli altri soft
      aereo_table[index].pid = pid;
      if (pid!=NIL) task_activate(pid);
      else {           //ripristino il posto libero in tabella;
        aereo_table[index].status =  0;
        aereo_table[index].killing = 0;
        }
      }

    else if (count>aereo_count) {       // Richiesta cancellazione aereo
      aereo_table[last].killing = 1;
      }

    task_endcycle();
    }
  return NULL;
}

TASK aereo(void *arg)
{
  int        index;    // indice nella aereo_table
  FAB_IMAGE* image;
  int        xx0,yy0,xx1,yy1;
  int        y;
  double     x;
  double     old_x;    // [pixel]: conserva valore x prima di aggiornare
  double     dx;       // [pixel/us]: spostamento periodico
  int        first_time;

  index = (int)arg;
  image =      aereo_table[index].image;
  y = aereo_table[index].y;
  x = aereo_table[index].x;
  dx = aereo_table[index].vel * (AEREO_X_MAX-AEREO_X_MIN) / (double)BASE_L
       * (AEREO_PERIOD / (60*60*1000000.0))
       * aereo_table[index].dir;
  first_time = 1;

  while(1){

   old_x = x;
   x += dx;

   xx0 = old_x - AEREO_LX/2;
   yy0 = y - AEREO_LY/2;
   xx1 = xx0   + AEREO_LX - 1;
   yy1 = yy0   + AEREO_LY - 1;

   aereo_table[index].x = x;   //...aggiornamento posizione in tabella...

   if (x<AEREO_X_MIN || x>AEREO_X_MAX) {
      aereo_table[index].killing = 1;
      }

   if (aereo_table[index].killing) {
      if (!first_time) {
          mutex_lock(&grx_mutex);
          FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,yy1);
          mutex_unlock(&grx_mutex);
          }
      aereo_table[index].status=0;
      aereo_table[index].pid=-2;
      aereo_table[index].killing=0;
      //il task aereo_creator si accorge che c'Š un aereo in meno
      // quindi ne ricreer… uno nuovo!!!
      return NULL;
      }

   if(aereo_table[index].x!=(int)old_x) {//...se c'Š lo spostamento reale
                                         //        di almeno un pixel...
     mutex_lock(&grx_mutex);   //...aggiorna disegno...
     if (first_time) {
         first_time=0;
         }
     else {
         FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,yy1);
         }

     FAB_image_put_within(image, x - AEREO_LX/2, y - AEREO_LY/2,
                                 X0, Y0, X1, Y1);
     mutex_unlock(&grx_mutex);
     }

   task_endcycle();
 }

return NULL;
}

PID crea_soft_aereo_creator()
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_periodic(m);
  soft_task_def_period(m,AEREO_CREATOR_PERIOD);
  soft_task_def_wcet(m,AEREO_CREATOR_WCET);
  soft_task_def_met(m,AEREO_CREATOR_MET);

  pid = task_create("aereo_creator", aereo_creator, &m, NULL);
  return pid;
}

PID crea_hard_aereo(int index)
{
  HARD_TASK_MODEL m;
  PID pid;

  hard_task_default_model(m);
  hard_task_def_level(m,1);
  hard_task_def_arg(m,(void*)index);
  hard_task_def_periodic(m);
  hard_task_def_wcet(m, AEREO_WCET);
  hard_task_def_mit(m,AEREO_PERIOD);
  hard_task_def_usemath(m);

  pid = task_create("hard_aereo", aereo, &m, NULL);
  return pid;
}

PID crea_soft_aereo(int index)
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_level(m,2);
  soft_task_def_arg(m,(void*)index);
  soft_task_def_periodic(m);
  soft_task_def_period(m,AEREO_PERIOD);
  soft_task_def_wcet(m, AEREO_WCET);
  soft_task_def_met(m,AEREO_MET);
  soft_task_def_usemath(m);

  pid = task_create("soft aereo", aereo, &m, NULL);
  return pid;
}

