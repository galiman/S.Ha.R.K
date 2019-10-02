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

/* CVS :        $Id: cannone.c,v 1.4 2006/07/03 15:10:15 tullio Exp $ */

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

TASK cannone_creator(void * arg)
{ int             i, index, last;
  WORD            count;
  PID             pid=-1;

  while(1){

    count=0;
    index=-1;
    last=-1;

    for (i=0; i<CANNONE_N_MAX; i++)  // Conteggio aerei attivi
      if (cannone_table[i].status) {
         count++;
         last=i;          //...ultima posizione occupata in tabella...
         }
      else {
         if (index==-1) index=i;   //...prima posizione libera in tabella...
         }

    if (count<cannone_count && index!=-1) { // Richiesta nuovo cannone
      cannone_table[index].status =  1;     //...occupa posizione...
      cannone_table[index].killing = 0;     //...disattiva richiesta di kill...
      if (index==0) pid = crea_hard_cannone(index); // sempre il 1ø hard
      else          pid = crea_soft_cannone(index); // tutti gli altri soft
      cannone_table[index].pid = pid;
      if (pid!=NIL) task_activate(pid);
      else {           //ripristino il posto libero in tabella;
        cannone_table[index].status =  -2;
        cannone_table[index].killing = -2;
        }
      }

    else if (count>cannone_count) {       // Richiesta cancellazione cannone
      cannone_table[last].killing = 1;
      }

     task_endcycle();
    }
  return NULL;
}

PID crea_soft_cannone_creator();
PID crea_hard_cannone(int index);
PID crea_hard_cannone(int index);

PID crea_hard_missile(int index);
PID crea_soft_missile(int index);

TASK cannone(void *arg)
{
  PID        missile_pid=-2;
  int        i, index;    // indice nella aereo_table e cannone_table
  int        d, dir;
  int        xx0,yy0,xx1,yy1;
  int        first_time;

  index = (int)arg;
  first_time = 1;
  if (index%2) {
     //indice dispari
     xx0 = X0 + 500/2 - CANNONE_LX/2 + 500/(CANNONE_N_MAX+1)*(index+1)/2;
     }
  else {
     //indice pari
     xx0 = X0 + 500/2 - CANNONE_LX/2 - 500/(CANNONE_N_MAX+1)*(index/2);
     }
  xx1 = xx0 + CANNONE_LX;
  yy0= Y1 - CANNONE_LY;
  yy1= Y1;

  cannone_table[index].x = xx0 + CANNONE_LX/2;

  while(1){

   if (cannone_table[index].killing) {
      if (!first_time) {
          mutex_lock(&grx_mutex);
          FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,yy1);
          mutex_unlock(&grx_mutex);
          }
      cannone_table[index].status=0;
      cannone_table[index].pid=-2;
      cannone_table[index].killing=0;
      //il task aereo_creator si accorge che c'Š un aereo in meno
      // quindi ne ricreer… uno nuovo!!!
      return NULL;
      }

   if (first_time) {
      mutex_lock(&grx_mutex);   //...aggiorna disegno...
      if (index==0) FAB_image_put(image_cannone[1], xx0, yy0);
      else          FAB_image_put(image_cannone[0], xx0, yy0);
      mutex_unlock(&grx_mutex);
      first_time=0;
      }

    if (!cannone_table[index].fire) {
      for (i=0; i<AEREO_N_MAX; i++) {
         if (aereo_table[i].status) {
             d = aereo_table[i].x - (xx0+CANNONE_LX/2);
             dir = aereo_table[i].dir;
             if (d*dir<0 && FAB_ABS(d)<=CANNONE_SENSIBILITA/1000.0*500/BASE_L) {
                  // FA   F U O C O
                  if (index==0) missile_pid=crea_hard_missile(index);
                  else          missile_pid=crea_soft_missile(index);
                  if (missile_pid!=NIL) {
                     cannone_table[index].fire = 1;
                     task_activate(missile_pid);
                     break;
                     }
                  }
             }
         }
      }

   task_endcycle();
 }

return NULL;
}



PID crea_soft_cannone_creator()
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_periodic(m);
  soft_task_def_period(m,CANNONE_CREATOR_PERIOD);
  soft_task_def_wcet(m,CANNONE_CREATOR_WCET);
  soft_task_def_met(m,CANNONE_CREATOR_MET);

  pid = task_create("cannone_creator", cannone_creator, &m, NULL);
  return pid;
}

PID crea_hard_cannone(int index)
{
  HARD_TASK_MODEL m;
  PID pid;

  hard_task_default_model(m);
  hard_task_def_level(m,1);
  hard_task_def_arg(m,(void*)index);
  hard_task_def_periodic(m);
  hard_task_def_wcet(m, CANNONE_WCET);
  hard_task_def_mit(m,CANNONE_PERIOD);

  pid = task_create("hard_cannone", cannone, &m, NULL);
  return pid;
}

PID crea_soft_cannone(int index)
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_level(m,2);
  soft_task_def_arg(m,(void*)index);
  soft_task_def_periodic(m);
  soft_task_def_period(m,CANNONE_PERIOD);
  soft_task_def_wcet(m, CANNONE_WCET);
  soft_task_def_met(m,CANNONE_MET);

  pid = task_create("soft_cannone", cannone, &m, NULL);
  return pid;
}

