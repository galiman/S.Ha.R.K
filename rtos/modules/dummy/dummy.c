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

/**
 ------------
 CVS :        $Id: dummy.c,v 1.1 2005/02/25 10:55:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:55:09 $
 ------------

 This file contains the Dummy scheduling module

 Read dummy.h for further details.

**/

/*
 * Copyright (C) 2000 Paolo Gai
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

#include <dummy/dummy/dummy.h>
#include <ll/ll.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>



/*+ the level redefinition for the Dummy level +*/
typedef struct {
  level_des l;     /*+ the standard level descriptor          +*/

  PID dummy;       /*+ the dummy task...                      +*/
} dummy_level_des;


static PID dummy_public_scheduler(LEVEL l)
{
  dummy_level_des *lev = (dummy_level_des *)(level_table[l]);
  //kern_printf("DUMMYsched!!! %d", lev->dummy);
  return lev->dummy;
}

static int dummy_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
  dummy_level_des *lev = (dummy_level_des *)(level_table[l]);

  if (m->pclass != DUMMY_PCLASS) return -1;
  if (m->level != 0 && m->level != l) return -1;
  if (lev->dummy != -1) return -1;

  /* the dummy level doesn't introduce any new field in the TASK_MODEL
     so, all initialization stuffs are done by the task_create.
     the task state is set at SLEEP by the general task_create */
  return 0; /* OK */
}

static void dummy_public_dispatch(LEVEL l, PID p, int nostop)
{
  /* nothing... the dummy hangs the cpu waiting for interrupts... */
}

static void dummy_public_epilogue(LEVEL l, PID p)
{
  proc_table[p].status = SLEEP; /* Paranoia */
}

/*+ Dummy task must be present & cannot be killed; +*/
static TASK dummy()
{
    /*
    It is possible to Halt the CPU & avoid consumption if idle
    cycle are intercepted with hlt instructions!
    It seems that some CPU have buggy hlt instruction or they
    have not it at all! So, if available, use the hlt facility!!
    */
    #ifdef __HLT_WORKS__
    for(;;) {
//       kern_printf("?");
	hlt();
    }
    #else
    for(;;);// kern_printf("?");
    #endif
}

/* Registration functions */

/*+ This init function install the dummy task +*/
static void dummy_create(void *l)
{
  LEVEL lev;
  PID p;
  DUMMY_TASK_MODEL m;

  lev = (LEVEL)l;

  dummy_task_default_model(m);
  dummy_task_def_level(m,lev);
  dummy_task_def_system(m);
  dummy_task_def_nokill(m);
  dummy_task_def_ctrl_jet(m);

  ((dummy_level_des *)level_table[lev])->dummy = p =
    task_create("Dummy", dummy, &m, NULL);

  if (p == NIL)
    printk("\nPanic!!! can't create dummy task...\n");

  /* dummy must block all signals... */
  proc_table[p].sigmask = 0xFFFFFFFF;
}


/*+ Registration function:
    TIME slice                the slice for the Round Robin queue
    int createmain            1 if the level creates the main task 0 otherwise
    struct multiboot_info *mb used if createmain specified   +*/
LEVEL dummy_register_level()
{
  LEVEL l;            /* the level that we register */
  dummy_level_des *lev;  /* for readableness only */

  printk("dummy_register_level\n");

  /* request an entry in the level_table */
  l = level_alloc_descriptor(sizeof(dummy_level_des));

  lev = (dummy_level_des *)level_table[l];

  /* fill the standard descriptor */
  lev->l.public_scheduler = dummy_public_scheduler;
  lev->l.public_guarantee = NULL;
  lev->l.public_create    = dummy_public_create;
  lev->l.public_dispatch  = dummy_public_dispatch;
  lev->l.public_epilogue  = dummy_public_epilogue;

  /* the dummy process will be created at init_time.
     see also dummy_level_accept_model,dummy_create   */
  lev->dummy = -1;

  sys_atrunlevel(dummy_create,(void *) l, RUNLEVEL_INIT);

  return l;
}
