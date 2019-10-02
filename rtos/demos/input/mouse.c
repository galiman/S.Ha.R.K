
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

#include <drivers/shark_mouse26.h>
#include <drivers/shark_keyb26.h>
#include <drivers/shark_spk26.h>

void my_sysclose(KEY_EVT *e)
{
	mouse_txtcursor(DISABLE);

	exit(0);
}

TASK my_putxy(void *arg) {

	int x, y, z;
	unsigned long btn;

	clear();

	while (1) {
		mouse_getposition(&x, &y, &z, &btn);
		place(10, 10);
		cprintf("X: %2d - Y: %2d - Z: %3d - Btn: %4d\n", x, y, z, (int)btn);

		task_endcycle();
	}
}

int main(int argc, char **argv)
{
	SOFT_TASK_MODEL mp;
	PID pid;
	KEY_EVT ev;

	ev.ascii = 'c';
	ev.scan  = KEY_C;
	ev.status = KEY_PRESSED;
	ev.flag = CNTL_BIT;
	keyb_hook(ev, my_sysclose, FALSE);
	ev.flag = CNTR_BIT;
	keyb_hook(ev, my_sysclose, FALSE);
	
	mouse_txtcursor(ENABLE);

	soft_task_default_model(mp);
	soft_task_def_level(mp,2);
	soft_task_def_ctrl_jet(mp);
	soft_task_def_met(mp,700);
	soft_task_def_period(mp,1000);
	soft_task_def_usemath(mp);
	pid = task_create("Mouse_Print", my_putxy, &mp, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <Mouse_Print>\n");
		my_sysclose(NULL);
	} else
		task_activate(pid);

	return 0;
}
