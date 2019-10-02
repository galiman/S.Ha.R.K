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
 CVS :        $Id: pinit.c,v 1.4 2005/02/25 11:05:44 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/02/25 11:05:44 $
 ------------

 This is a minimal initialization file for the PSE51 profile.

 It initializes the POSIX scheduler, the Hartik Ports and the Keyboard driver.

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



#include "kernel/kern.h"
#include "posix/posix/posix.h"
#include "dummy/dummy/dummy.h"

#include "pi/pi/pi.h"
#include "pc/pc/pc.h"

#include "sem/sem/sem.h"
#include "hartport/hartport/hartport.h"

#include "pthread.h"
#include "time.h"

/*+ sysyem tick in us +*/
#define TICK 0

/*+ RR tick in us +*/
#define RRTICK 10000

TIME __kernel_register_levels__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  POSIX_register_level(RRTICK, POSIX_MAIN_YES, mb, 32);
  dummy_register_level();

  PI_register_module();
  PC_register_module();

  SEM_register_module();

  /* for the Pthread library */
  PTHREAD_register_module(0, 0, 1);

  /* for the real time clock extensions */
  TIMER_register_module();

  return TICK;
}

TASK __init__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  HARTPORT_init();

  __call_main__(mb);

  return (void *)0;
}
