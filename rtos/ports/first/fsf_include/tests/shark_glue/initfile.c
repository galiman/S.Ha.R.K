/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi	 <giacomo@gandalf.sssup.it>
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
 */

#include "kernel/kern.h"
#include "modules/edf.h"
#include "modules/cbs.h"
#include "modules/posix.h"
#include "pthread.h"
#include "modules/sem.h"
#include "modules/dummy.h"
#include "modules/hartport.h"
#include "grubstar.h"
#include "fsf.h"

#include "modules/pi.h"
#include "modules/pc.h"
#include "modules/intdrive.h"

#include <drivers/shark_linuxc26.h>
#include <drivers/shark_input26.h>
#include <drivers/shark_keyb26.h>
#include <drivers/shark_pci26.h>

#define TICK 0

#define RRTICK 10000

/*+ Interrupt Server +*/
#define INTDRIVE_Q 1000
#define INTDRIVE_T 10000
#define INTDRIVE_FLAG 0

void call_shutdown_task(void *arg);
int device_drivers_init();
int device_drivers_close();
void set_shutdown_task();
TASK shutdown_task_body(void *arg);

PID shutdown_task_PID = 1;

// void load_file();

int device_drivers_close() {

  //KEYB26_close();
  //INPUT26_close();
  
  return 0;
                                                                           
}

int device_drivers_init() {
  KEYB_PARMS  kparms = BASE_KEYB;
	
  LINUXC26_register_module();
  PCI26_init();
  //INPUT26_init();

        /*keyb_def_map(kparms, KEYMAP_IT);*/
  //keyb_def_ctrlC(kparms, NULL);
  //KEYB26_init(&kparms);


  return 0;
}

TASK shutdown_task_body(void *arg) {

	device_drivers_close();
	sys_shutdown_message("-- S.Ha.R.K. Closed --\n");
	sys_abort_shutdown(0);

	return NULL;
}
#define SHUTDOWN_TIMEOUT_SEC 3

void set_shutdown_task() {

	NRT_TASK_MODEL nrt;

	nrt_task_default_model(nrt);
	nrt_task_def_system(nrt);

	shutdown_task_PID = task_create("Shutdown Task", shutdown_task_body, &nrt, NULL);
	if (shutdown_task_PID == NIL) {
		sys_shutdown_message("Error: Cannot create shutdown task\n");
		exit(-1);
	}

}

void call_shutdown_task(void *arg) {

	struct timespec t;

	sys_gettime(&t);
	t.tv_sec += SHUTDOWN_TIMEOUT_SEC;

	/* Emergency timeout to exit from RUNLEVEL_SHUTDOWN */
	kern_event_post(&t,(void *)((void *)sys_abort_shutdown),(void *)0);

	task_activate(shutdown_task_PID);
}

TIME __kernel_register_levels__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;
  int edf_level;
  int grubstar_level;
  int posix_level;
  int pi_level;
  int pc_level;

  INTDRIVE_register_level(INTDRIVE_Q, INTDRIVE_T, INTDRIVE_FLAG);
  edf_level=EDF_register_level(EDF_ENABLE_ALL);
  posix_level=POSIX_register_level(RRTICK, POSIX_MAIN_YES, mb, 32);
  grubstar_level = GRUBSTAR_register_level(FSF_MAX_N_SERVERS, edf_level);

  FSF_register_module(posix_level,grubstar_level, (int)(MAX_BANDWIDTH*0.8));
  dummy_register_level();

  CBS_register_level(CBS_ENABLE_ALL,1);

  SEM_register_module();

  pi_level=PI_register_module();
  pc_level=PC_register_module();

  PTHREAD_register_module(posix_level, pi_level, pc_level);

  return TICK;

}

TASK __init__(void *arg)
{
  struct multiboot_info *mb = (struct multiboot_info *)arg;

  HARTPORT_init();
  
  set_shutdown_task();
  
  device_drivers_init();
  
  sys_atrunlevel(call_shutdown_task, NULL, RUNLEVEL_SHUTDOWN);

  __call_main__(mb);

  return (void *)0;
}


