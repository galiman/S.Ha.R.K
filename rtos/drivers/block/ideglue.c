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


/***************************************

  CVS :        $Id: ideglue.c,v 1.3 2005/05/10 17:20:09 mauro Exp $
 
  Revision:    $Revision: 1.3 $

  Last update: $Date: 2005/05/10 17:20:09 $

  This module is used to link the low-level IDE module with
  some kernel specific and particular functionality (it is included 
  into idelow.c).

***************************************/

/*
 * Copyright (C) 1999,2000 Massimiliano Giorgi
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

#include <ll/ll.h>
#include <kernel/func.h>
#include <kernel/model.h>

#include "glue.h"
#include "ide.h"

/* --- */

/*+ the default values for the server task +*/
#define DEFAULT_WCET     200
//#define DEFAULT_MIT      1000
#define DEFAULT_DL       7000 
//#define DEFAULT_WCET     200
////#define DEFAULT_MIT      1000
//#define DEFAULT_DL       2000 

/* --- */

void ide_service_request(int ideif);

/*++++++++++++++++++++++++++++++++++++++
  
  This is the ide server (a task is created for every interface)

  TASK ide_server
    never return

  int ideif
    interface number (index into ide[])
  ++++++++++++++++++++++++++++++++++++++*/

TASK ide_server(int ideif)
{
  for (;;) {
    task_endcycle();
    ide_service_request(ideif);
  }
  return NULL;
}

/*++++++++++++++++++++++++++++++++++++++
  
  A request is made to the server (the server is activated).

  int ideif
    interface that own the server
  ++++++++++++++++++++++++++++++++++++++*/

void ide_glue_send_request(int ideif)
{
  task_activate(ide[ideif].server);
}

/*++++++++++++++++++++++++++++++++++++++
  
  This function activate an interface: create a server task and set the
  irq handler to wake up this server.

  int ide_glue_activate_interface
    return 0 on success, other value on error

  int ideif
    interface to activate
  ++++++++++++++++++++++++++++++++++++++*/


TASK ide_dummy(int x)
{
  for (;;) {
    task_endcycle();
  }
  return NULL;
}



int ide_glue_activate_interface(int ideif)
{
  struct ide_server_model *parms=
    (struct ide_server_model *)ide_parm_initserver;
  char name[32];
  SOFT_TASK_MODEL model;
  TIME dl=DEFAULT_DL;
  //TIME mit=DEFAULT_MIT;
  TIME wcet=DEFAULT_WCET;
  
  soft_task_default_model(model);    
  if (parms!=NULL) {
    dl=parms->dl;
    //mit=parms->mit;
    wcet=parms->wcet;
  }
  soft_task_def_system(model);
  //hard_task_def_mit(model,mit);
  //hard_task_def_drel(model,dl);
  //hard_task_def_wcet(model,wcet);
  soft_task_def_met(model,wcet);
  soft_task_def_wcet(model,wcet);
  soft_task_def_period(model,dl);
  soft_task_def_system(model);
  soft_task_def_nokill(model);
  
  soft_task_def_arg(model,(void*)ideif);
  soft_task_def_aperiodic(model);
  /* forse un NO_PREEMPT e' meglio */

  sprintf(name,"ide%i-server",ideif);
  ide[ideif].server=task_create(name,ide_server,&model,NULL);

  if (ide[ideif].server==NIL) {
    if (ide_showinfo_flag)
      printk(IDELOG "ide glue: can't create task");
    return -1;
  }

  /* activate */
  task_activate(ide[ideif].server);

  /* associate an IRQ handler */
  handler_set(ide[ideif].irq,NULL,FALSE,ide[ideif].server, NULL);
  
  return 0;
}


/*++++++++++++++++++++++++++++++++++++++
  
  This function unactivate??? an interface: release the irq and
  kill the server task.

  int ideif
    interface to release
  ++++++++++++++++++++++++++++++++++++++*/

void __inline__ ide_glue_unactivate_interface(int ideif)
{
  handler_remove(ide[ideif].irq);
  task_kill(ide[ideif].server);
}

