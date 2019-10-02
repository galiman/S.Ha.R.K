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
 CVS :        $Id: cancel.c,v 1.4 2005/01/08 14:44:06 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:44:06 $
 ------------

 This file contains:

 - the cancellation point function
 - the setcancelstate and setcanceltype functions

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

#include <stdarg.h>
#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>
/*+
  This primitive creates a cancellation point in the calling task
+*/
void task_testcancel(void)
{
  SYS_FLAGS f;  // testcancel may be called from primitives, i.e. task_join

  f = kern_fsave();

  if (proc_table[exec_shadow].control & KILL_ENABLED &&
      proc_table[exec_shadow].control & KILL_REQUEST ) {
    task_makefree(TASK_CANCELED);
    scheduler();
    ll_context_to(proc_table[exec_shadow].context);
  }
  kern_frestore(f);
}

/*+ This primitive set the cancellation state of the task +*/
int task_setcancelstate(int state, int *oldstate)
{
  SYS_FLAGS f;

  f = kern_fsave();
  if (state != TASK_CANCEL_ENABLE &&
      state != TASK_CANCEL_DISABLE) {
    kern_frestore(f);
    return -1;
  }

  *oldstate = (proc_table[exec_shadow].control & KILL_ENABLED) != 0;
  proc_table[exec_shadow].control &= ~KILL_ENABLED;
  proc_table[exec_shadow].control |= state;

  kern_frestore(f);
  return 0;
}

/*+ This primitive set the cancellation type of the task +*/
int task_setcanceltype(int type, int *oldtype)
{
  SYS_FLAGS f;

  f = kern_fsave();
  if (type != TASK_CANCEL_DEFERRED &&
      type != TASK_CANCEL_ASYNCHRONOUS) {
    kern_frestore(f);
    return -1;
  }

  *oldtype = (proc_table[exec_shadow].control & KILL_DEFERRED) != 0;
  proc_table[exec_shadow].control &= ~KILL_DEFERRED;
  proc_table[exec_shadow].control |= type;

  kern_frestore(f);
  return 0;
}
