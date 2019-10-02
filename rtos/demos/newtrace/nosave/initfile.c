/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   Tullio Facchinetti  <tullio.facchinetti@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
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
#include "edf/edf/edf.h"
#include "cbs/cbs/cbs.h"
#include "rr/rr/rr.h"
#include "dummy/dummy/dummy.h"

#include "sem/sem/sem.h"
#include "hartport/hartport/hartport.h"
#include "cabs/cabs/cabs.h"

/*+ sysyem tick in us +*/
#define TICK 0

/*+ RR tick in us +*/
#define RRTICK 10000

TIME __kernel_register_levels__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;
  LEVEL EDF_level;

  EDF_level = EDF_register_level(EDF_ENABLE_ALL);
  CBS_register_level(CBS_ENABLE_ALL, EDF_level);
  RR_register_level(RRTICK, RR_MAIN_YES, mb);
  dummy_register_level();

  SEM_register_module();

  CABS_register_module();

  return TICK;
}

TASK __init__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  HARTPORT_init();

  __call_main__(mb);

  return (void *)0;
}

