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
 CVS :        $Id: tskmsg.c,v 1.3 2003/12/10 16:54:59 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2003/12/10 16:54:59 $
 ------------

**/

/*
 * Copyright (C) 2002 Paolo Gai
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

#include <tracer.h>


/*
  The running task (pointed by exec_shadow) sent a message m to the
  scheduling module that handle the task p.

  If the message has value NULL the behavior should be the
  task_endcycle primitive behavior, and an endcycle tracer event is
  generated.

*/
int task_message(void *m, PID p, int reschedule)
{
  LEVEL l;            /* for readableness only */

  int retvalue;
  
  if (p == NIL) p = exec_shadow;

  if (reschedule) {
    proc_table[exec_shadow].context = kern_context_save();

    kern_epilogue_macro();

    l = proc_table[p].task_level;
    retvalue = level_table[l]->public_message(l,p,m);

    exec = exec_shadow = -1;
    scheduler();

    kern_context_load(proc_table[exec_shadow].context);

  } else {
    SYS_FLAGS f;

    f = kern_fsave();
    l = proc_table[p].task_level;
    retvalue = level_table[l]->public_message(l,p,m);
    kern_frestore(f);
  }

  return retvalue;
}
