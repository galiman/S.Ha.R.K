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
 CVS :        $Id: tpreempt.c,v 1.3 2003/11/05 15:05:12 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2003/11/05 15:05:12 $
 ------------

 task_preempt and task_nopreempt

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


/*+ This primitive set the non-preemption flag in the task
    descriptor control field                               +*/
void task_nopreempt(void)
{
   SYS_FLAGS f;

    if ((~proc_table[exec_shadow].control) & NO_PREEMPT) {
      f = kern_fsave();
      proc_table[exec_shadow].control |= NO_PREEMPT;
      if (cap_timer != NIL) {
        kern_event_delete(cap_timer);
        cap_timer = NIL;
      }
      kern_frestore(f);
    }
}

/*+ This primitive reset the non-preemption flag in the task
    descriptor control field and check if there is a preemption
    to make...
+*/
void task_preempt(void)
{
    if (proc_table[exec_shadow].control & NO_PREEMPT)
    {
      proc_table[exec_shadow].context = kern_context_save();

      proc_table[exec_shadow].control &= ~NO_PREEMPT;
      scheduler();
      kern_context_load(proc_table[exec_shadow].context);
    }
}
