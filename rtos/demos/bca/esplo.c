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

/* CVS :        $Id: esplo.c,v 1.6 2006/07/03 15:10:15 tullio Exp $ */

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

#include <time.h>
#include "bca.h"

PID crea_soft_esplo(int index)
{
  SOFT_TASK_MODEL m;
  PID pid;

  soft_task_default_model(m);
  soft_task_def_arg(m,(void*)index);
  soft_task_def_period(m,ESPLO_PERIOD*2);
  soft_task_def_wcet(m,ESPLO_WCET*10);
  soft_task_def_met(m,ESPLO_MET*10);

  pid = task_create("esplo", esplo, &m, NULL);
  return pid;
}

TASK esplo(void *argv)
{
  int i;
  int index;
  WORD xx0, yy0, xx1, yy1;
  struct timespec delay;


  delay.tv_sec = 0;
  delay.tv_nsec = 20000000;
  i=0;
  index = (int)argv;
  xx0 = aereo_table[index].x - ESPLO_LX/2;
  yy0 = aereo_table[index].y - ESPLO_LY/2;
  xx1 = xx0   + ESPLO_LX-1;
  yy1 = yy0   + ESPLO_LY-1;

  for(i=0;i<5;i++) {
     mutex_lock(&grx_mutex);
     FAB_image_put_within(image_esplo[i],xx0,yy0,X0,Y0,X1,Y1);
     mutex_unlock(&grx_mutex);
     task_endcycle();
  }

  mutex_lock(&grx_mutex);
  FAB_image_put_within(image_bca,X0,Y0,xx0,yy0,xx1,yy1);
  mutex_unlock(&grx_mutex);

  return 0;

}
