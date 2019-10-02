/*
 * Project: S.Ha.R.K
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it
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
#include "hardcbs/hardcbs/hardcbs.h"
#include "rr/rr/rr.h"
#include "dummy/dummy/dummy.h"
#include "intdrive/intdrive/intdrive.h"

#include "sem/sem/sem.h"
#include "hartport/hartport/hartport.h"

#include "drivers/shark_linuxc26.h"
#include "drivers/shark_pci26.h"

/*+ sysyem tick in us +*/
#define TICK 0

/*+ RR tick in us +*/
#define RRTICK 10000

/*+ Interrupt Server +*/
#define INTDRIVE_Q 1000
#define INTDRIVE_U 1000
#define INTDRIVE_FLAG 0

void call_shutdown_task(void *arg);
int device_drivers_init();
int device_drivers_close();
void set_shutdown_task();
TASK shutdown_task_body(void *arg);

PID shutdown_task_PID = -1;

TIME __kernel_register_levels__(void *arg)
{
	struct multiboot_info *mb = (struct multiboot_info *)arg;

	INTDRIVE_register_level(INTDRIVE_Q, INTDRIVE_Q, INTDRIVE_U, INTDRIVE_FLAG);
	EDF_register_level(EDF_ENABLE_ALL);
	HCBS_register_level(HCBS_ENABLE_ALL, 1);
	RR_register_level(RRTICK, RR_MAIN_YES, mb);
	dummy_register_level();

	SEM_register_module();

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

void set_shutdown_task() {

	NRT_TASK_MODEL nrt;

	nrt_task_default_model(nrt);
	nrt_task_def_system(nrt);

	shutdown_task_PID = task_create("Shutdown Task",shutdown_task_body,&nrt,NULL);
	if (shutdown_task_PID == NIL) {
		sys_shutdown_message("Error: Cannot create shutdown task\n");
		exit(1);
	}

}

int device_drivers_init() {
                                                                                                                             
        LINUXC26_register_module(TRUE);
                                                                                                                             
        PCI26_init();

	return 0;
                                                                                                                             
}

int device_drivers_close() {

	return 0;

}

TASK shutdown_task_body(void *arg) {

	device_drivers_close();

	sys_shutdown_message("-- S.Ha.R.K. Closed --\n");

	return NULL;

}

#define SHUTDOWN_TIMEOUT_SEC 3

void call_shutdown_task(void *arg) {

	struct timespec t;
                                                                                                                             
	sys_gettime(&t);
	t.tv_sec += SHUTDOWN_TIMEOUT_SEC;
                                                                                                                             
	kern_event_post(&t,(void *)((void *)sys_abort_shutdown),(void *)0);

	task_activate(shutdown_task_PID);

}
