
/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *
 * Authors     : Mauro Marinoni <mauro.marinoni@unipv.it>
 * (see authors.txt for full list of hartik's authors)
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
 */

#include <kernel/kern.h>
#include <kernel/func.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/shark_joy26.h>

TASK my_getjoy(void *arg) {

	int a0, a1, a2, a3, btn;
	
	while (1) {
		joy_getstatus(&a0, &a1, &a2, &a3, &btn);
		cprintf("(%6d %6d) %2x\n", a0, a1, btn);
		task_endcycle();
		if (btn == 0xF)
			exit(1);
	}
}

int main(int argc, char **argv)
{
	SOFT_TASK_MODEL mp;
	PID pid;

	if (!JOY26_installed()) {
		cprintf("No Joystick found.");
		exit(1);
	}
	soft_task_default_model(mp);
	soft_task_def_level(mp,2);
	soft_task_def_ctrl_jet(mp);
	soft_task_def_met(mp,700);
	soft_task_def_period(mp,10000);
	soft_task_def_usemath(mp);
	pid = task_create("Joy_Print", my_getjoy, &mp, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <Joy_Print>\n");
		exit(1);
	} else
		task_activate(pid);

	return 0;
}
