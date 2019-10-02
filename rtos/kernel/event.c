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
 CVS :        $Id: event.c,v 1.3 2005/01/08 14:45:23 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2005/01/08 14:45:23 $
 ------------

 This file contains the functions to be used into events:

 event_need_reschedule
 event_resetepilogue
 reschedule

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

static void reschedule(void);

#ifdef __PERF_TEST2__
static void perftest_reschedule(void);
#endif

/*+ called in an event to force the system to execute the scheduler at
    the end of an event list. An event shall NEVER call directly
    the sequence {scheduler(); kern_context_load(...);} !!! +*/
void event_need_reschedule()
{
  #ifdef __PERF_TEST2__
  event_setepilogue(perftest_reschedule);
  #else
  event_setepilogue(reschedule);
  #endif
}


/* some static functions ...*/

int event_noreschedule = 0;

/*+ used to call the scheduler at the end of an event list +*/
static void reschedule(void)
{
  if (!event_noreschedule) {
    scheduler();
    ll_context_to(proc_table[exec_shadow].context);
  }
}


/*+ called in the events to force the event handler to reset the event
    epilogue; called in __kernel_init__ +*/
void event_resetepilogue()
{
  #ifdef __PERF_TEST2__
  extern TIME perftime_prol[10001];
  extern int perftime_count;
  extern void perftest_epilogue(void);

  if (perftime_count < 10000) {
    perftime_prol[perftime_count] = kern_gettime(NULL);
  }
  event_setepilogue(perftest_epilogue);
  #else
  event_setepilogue(NULL);
  #endif

}

#ifdef __PERF_TEST2__
static void perftest_reschedule(void)
{
  extern TIME perftime_epil[10001];
  extern int perftime_count;
  reschedule();

  if (perftime_count < 10000){
    perftime_epil[perftime_count] = kern_gettime(NULL);
    perftime_count++;
  }
}

static void perftest_epilogue(void)
{
  extern TIME perftime_epil[10001];
  extern int perftime_count;


  if (perftime_count < 10000) {
    perftime_epil[perftime_count] = kern_gettime(NULL);
    perftime_count++;
  }
}
#endif

