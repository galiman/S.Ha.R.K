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
 CVS :        $Id: isemdemo.c,v 1.2 2005/02/25 11:10:46 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2005/02/25 11:10:46 $
 ------------

 The simplest initialization file

 The tick is set to TICK ms.

 This file contains the 2 functions needed to initialize the system.

 These functions register the following levels:

 a RR (Round Robin) level
 a Dummy level

 It can accept these task models:

 NRT_TASK_MODEL at level 0

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
#include "rr/rr/rr.h"
#include "dummy/dummy/dummy.h"
#include "sem/sem/sem.h"


/*+ sysyem tick in us +*/
#define TICK 300

/*+ RR tick in us +*/
#define RRTICK 10000

TIME __kernel_register_levels__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  RR_register_level(RRTICK, RR_MAIN_YES, mb);
  dummy_register_level();

  SEM_register_module();

  return TICK;
}

TASK __init__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  __call_main__(mb);

  return (void *)0;
}

