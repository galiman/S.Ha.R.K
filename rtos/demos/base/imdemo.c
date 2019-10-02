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
 *
 * CVS :        $Id: imdemo.c,v 1.5 2005/05/10 17:21:17 mauro Exp $
**/

#include "kernel/kern.h"

#include "intdrive/intdrive/intdrive.h"
#include "edf/edf/edf.h"
#include "cbs/cbs/cbs.h"
#include "rr/rr/rr.h"
#include "dummy/dummy/dummy.h"

#include "sem/sem/sem.h"
#include "hartport/hartport/hartport.h"
#include "cabs/cabs/cabs.h"
#include "pi/pi/pi.h"
#include "pc/pc/pc.h"
#include "srp/srp/srp.h"
#include "npp/npp/npp.h"
#include "nop/nop/nop.h"

/*+ sysyem tick in us +*/
#define TICK 0

/*+ RR tick in us +*/
#define RRTICK 10000

/*+ Interrupt Server +*/
#define INTDRIVE_Q 1000
#define INTDRIVE_U 1000
#define INTDRIVE_FLAG 0

TIME __kernel_register_levels__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  INTDRIVE_register_level(INTDRIVE_Q, INTDRIVE_Q, INTDRIVE_U, INTDRIVE_FLAG); 
  EDF_register_level(EDF_ENABLE_ALL);
  RR_register_level(RRTICK, RR_MAIN_YES, mb);
  CBS_register_level(CBS_ENABLE_ALL, 1);
  dummy_register_level();

  SEM_register_module();

  CABS_register_module();

  PI_register_module();
  PC_register_module();
  NPP_register_module();
  SRP_register_module();
  NOP_register_module();

  return TICK;
}

TASK __init__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  HARTPORT_init();

  __call_main__(mb);

  return (void *)0;

}

