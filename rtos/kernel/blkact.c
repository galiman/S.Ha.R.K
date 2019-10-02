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
 CVS :        $Id: blkact.c,v 1.2 2002/10/28 07:58:19 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2002/10/28 07:58:19 $
 ------------

 block_activations & co.

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
  It blocks all explicit activation of a task made with task_activate and
  group_activate. These activations are registered in an internal counter,
  returned by task_unblock_activation.
  it returns 0 if all ok, or -1 otherwise. errno is set accordingly.
+*/
int task_block_activation(PID p)
{
    SYS_FLAGS f;

    /* some controls on the task p */
    if (p<0 || p>=MAX_PROC) {
	errno = EINVALID_TASK_ID;
	return -1;
    }
    if (proc_table[p].status == FREE) {
	errno = EINVALID_TASK_ID;
	return -1;
    }

    f = kern_fsave();
    if (!(proc_table[p].control & FREEZE_ACTIVATION)) {
      proc_table[p].control |= FREEZE_ACTIVATION;
      proc_table[p].frozen_activations = 0;
    }
    kern_frestore(f);
    return 0;
}

/*+
  It unblocks all explicit activations of a task, and returns the number of
  "frozen" activations. It not call the task_activate!!!!
  it returns -1 if an error occurs. errno is set accordingly.
+*/
int task_unblock_activation(PID p)
{
    int result;
    SYS_FLAGS f;

    /* some controls on the task p */
    if (p<0 || p>=MAX_PROC) {
	errno = EINVALID_TASK_ID;
	return -1;
    }
    if (proc_table[p].status == FREE) {
	errno = EINVALID_TASK_ID;
	return -1;
    }

    f = kern_fsave();

    result = 0;

    if (proc_table[p].control & FREEZE_ACTIVATION) {
      proc_table[p].control &= ~FREEZE_ACTIVATION;
      result = proc_table[p].frozen_activations;
      proc_table[p].frozen_activations = 0;
    }
    kern_frestore(f);

    return result;
}
