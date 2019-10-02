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
 CVS :        $Id: jet.c,v 1.3 2005/01/08 14:47:16 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2005/01/08 14:47:16 $
 ------------

 Kernel Job Execution Time functions

 This file contains:

 jet_delstat
 jet_getstat
 jet_gettable
 jet_update_endcycle
 jet_update_slice

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

/*---------------------------------------------------------------------*/
/* Jet management primitives                                           */
/*---------------------------------------------------------------------*/

/*+ This primitive returns the maximum execution time and the total
    execution time from the task_create or the last jet_delstat
    It returns also the number of instances to use to calculate the mean
    time and the current job execution time.
    The value returned is 0 if all ok, -1 if the PID is not correct or
    the task doesn't have the JET_ENABLE bit set.
+*/
int jet_getstat(PID p, TIME *sum, TIME *max, int *n, TIME *curr)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (p<0 || p>=MAX_PROC ||
      !(proc_table[p].control & JET_ENABLED) ||
      proc_table[p].status == FREE) {
    kern_frestore(f);
    return -1;
  }

  if (sum != NULL)  *sum = proc_table[p].jet_sum;
  if (max != NULL)  *max = proc_table[p].jet_max;
  if (n != NULL)    *n   = proc_table[p].jet_n;
  if (curr != NULL) *curr= proc_table[p].jet_table[proc_table[p].jet_curr];

  kern_frestore(f);
  return 0;
}

/*+ This primitive reset to 0 the maximum execution time and the mean
    execution time of the task p, and reset to 0 all the entries in
    jet_table.
    The value returned is 0 if all ok, -1 if the PID is not correct or
    the task doesn't have the JET_ENABLE bit set.                     +*/
int jet_delstat(PID p)
{
  SYS_FLAGS f;

  f = kern_fsave();

  if (p<0 || p>=MAX_PROC ||
      !(proc_table[p].control & JET_ENABLED) ||
      proc_table[p].status == FREE) {
    kern_frestore(f);
    return -1;
  }

  proc_table[p].jet_sum = 0;
  proc_table[p].jet_n   = 0;
  proc_table[p].jet_max = 0;

  kern_frestore(f);
  return 0;
}


/*+ This primitive returns the last n values of the task execution time
    recorded after the last call to jet_gettable or jet_delstat.
    If n is
    <0 it will be set only the last values inserted in the table
       since the last call of jet_gettable.
    >0 it will be set up to JET_TABLE_DIM datas.

    The value returned is -1 if the PID is not correct or
    the task doesn't have the JET_ENABLE bit set, otherwise it returns the
    number of values set in the parameter table.
    (can be from 0 to JET_TABLE_DIM-1)
+*/
int jet_gettable(PID p, TIME *table, int n)
{
  int i;
  SYS_FLAGS f;

  f = kern_fsave();

  if (p<0 || p>=MAX_PROC ||
      !(proc_table[p].control & JET_ENABLED) ||
      proc_table[p].status == FREE) {
    kern_frestore(f);
    return -1;
  }

  /* Copy all the info to the user */
  if (n < 0) {
    n = proc_table[p].jet_tvalid;
    proc_table[p].jet_tvalid = 0;
  }
  else if (n > JET_TABLE_DIM-1)
    n = JET_TABLE_DIM-1;

  for (i=(proc_table[p].jet_curr + JET_TABLE_DIM - n) % JET_TABLE_DIM;
       i!= proc_table[p].jet_curr;
       i= (i+1) % JET_TABLE_DIM)
     *table++ = proc_table[p].jet_table[i];

  kern_frestore(f);
  return n;
}

/*+ This function updates the jet information. +*/
void jet_update_slice(TIME t)
{
  if (proc_table[exec_shadow].control & JET_ENABLED)
    proc_table[exec_shadow].jet_table[ proc_table[exec_shadow].jet_curr ]
      += t;


  if (TIMESPEC_A_LT_B(&schedule_time, &cap_lasttime)) {
      kern_printf("scheduletime %lus %luns * caplasttime %lus %luns * exec=%d TIME = %lu \n",
                schedule_time.tv_sec,schedule_time.tv_nsec,
                cap_lasttime.tv_sec,cap_lasttime.tv_nsec, exec_shadow, t);
      kern_raise(XUNSPECIFIED_EXCEPTION,exec_shadow);
  }
}

/*+ This function updates the jet information at the task end period
    it is called in task_endcycle and task_sleep +*/
void jet_update_endcycle()
{
  int s,n;

  if (proc_table[exec_shadow].control & JET_ENABLED) {
    s = proc_table[exec_shadow].jet_table[proc_table[exec_shadow].jet_curr];
    n = proc_table[exec_shadow].jet_n;

    if (proc_table[exec_shadow].jet_max < s)
      proc_table[exec_shadow].jet_max = s;

    proc_table[exec_shadow].jet_sum += s;

    proc_table[exec_shadow].jet_n = n+1;

    proc_table[exec_shadow].jet_curr = (proc_table[exec_shadow].jet_curr + 1)
      % JET_TABLE_DIM;
    proc_table[exec_shadow].jet_table[proc_table[exec_shadow].jet_curr] = 0;

    /* we update the tvalid field... */
    if (proc_table[exec_shadow].jet_tvalid < JET_TABLE_DIM)
      proc_table[exec_shadow].jet_tvalid++;
  }
}


