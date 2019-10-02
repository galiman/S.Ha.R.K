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
 CVS :        $Id: initfile.c,v 1.2 2005/02/25 11:10:46 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2005/02/25 11:10:46 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai and Gabriele Bolognini
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

/*+ sysyem tick in us +*/
#define TICK 0

/*+ RR tick in us +*/
#define RRTICK 10000

void read_myfile(void);

TIME __kernel_register_levels__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  RR_register_level(RRTICK, RR_MAIN_YES, mb);
  dummy_register_level();

  /* If you need to interact with the DOS Filesystem and you use
     the X eXtender, this is the better place where you can use
     the DOS_fXXX functions...

     WARNING: You can call these functions only when you are in
              real mode!!!

     The system is in real mode:
     - into __kernel_register_levels__()
     - into sys_atrunlevel functions posted with RUNLEVEL_AFTER_EXIT
  */
  read_myfile();

  return TICK;
}

TASK __init__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  __call_main__(mb);

  return (void *)0;
}

