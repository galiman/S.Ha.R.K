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
 CVS :        $Id: var.h,v 1.5 2005/01/08 14:50:58 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2005/01/08 14:50:58 $
 ------------

Kernel global variables

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

#ifndef __KERNEL_VAR_H__
#define __KERNEL_VAR_H__

#include <ll/ll.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*---------------------------------------------------------------------*/
/* Kernel global variables: system queues & clock counters ...         */
/*---------------------------------------------------------------------*/

extern proc_des      proc_table[];     /*+ Process descriptor table   +*/

extern level_des    *level_table[];    /*+ Level descriptor table     +*/
/* for a description of the following fields, look in kernel/kern.c */
extern size_t level_size[];
extern int level_used[];
extern int level_first;
extern int level_last;
extern int level_free;
extern int level_next[];
extern int level_prev[];

extern resource_des *resource_table[]; /*+ Resource descriptor table  +*/


extern PID exec;              /*+ task suggested by the scheduler     +*/
extern PID exec_shadow;       /*+ task really executed                +*/

extern IQUEUE freedesc;        /*+ Free descriptor handled as a queue  +*/

extern TIME  sys_tick;	      /*+ System tick (in usec)	              +*/
extern struct timespec schedule_time;
                              /*+ Time at witch the last scheduler()
                                  was called                          +*/

extern int   cap_timer;       /*+ the capacity event posted when the
                                  task starts                         +*/
extern struct timespec cap_lasttime;
                              /*+ the time at whitch the capacity
                                  event is posted. Normally, it is
                                  equal to schedule_time              +*/

extern DWORD res_levels;      /*+ Resource levels active in the system +*/

extern int task_counter;      /*+ Application task counter. It represent
                                  the number of Application tasks in the
                                  system. When all Application Tasks end,
                                  also the system ends.                +*/

extern int system_counter;    /*+ System task counter. It represent
                                  the number of System tasks in the
                                  system with the NO_KILL flag reset.
                                  When all Application Tasks end,
                                  the system waits for the end of the
                                  system tasks and then it ends.       +*/


extern int calling_runlevel_func; /*+ this variable is set to 1 into
                                  call_runlevel_func (look at init.c)
                                  ad it is used because the task_activate
                                  (look at activate.c) must work in a
                                  different way when the system is in the
                                  global_context +*/

#define EXIT_CALLED   1
#define _EXIT_CALLED  2
extern int _exit_has_been_called; /*+ this variable is set when _exit is
				  called. in this case, the atexit
				  functions will not be called.
				  Values: - 0 neither exit or _exit
				  have been called - 1 exit has been
				  called - 2 _exit has been called +*/

extern CONTEXT global_context; /*+ Context used during initialization;
				   It references also a safe stack      +*/

extern int runlevel;           /*+ this is the system runlevel... it
				   may be from 0 to 4: 0 - init 1 -
				   running 2 - shutdown 3 - before
				   halting 4 - halting +*/

extern int event_noreschedule; /*+ This controls if the system needed
				   to be rescheduled at the end of an
				   IRQ/event or if must not because
				   exit, _exit, or sys_abort_shutdown
				   was called +*/


__END_DECLS
#endif /* __VAR_H__ */
