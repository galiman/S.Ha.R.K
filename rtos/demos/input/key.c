
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

#include <drivers/shark_keyb26.h>

//#define SHOW_EVT

void my_sysclose(KEY_EVT *e)
{
	kern_printf("S.Ha.R.K. closed.\n\n");
	exit(0);
}

TASK my_get(void *arg) {
#ifdef SHOW_EVT
	int status;
	KEY_EVT ev;
#else
	BYTE ch;
#endif

	while (1) {
#ifdef SHOW_EVT
		status = keyb_getcode(&ev, NON_BLOCK);
		if (status == TRUE)
			cprintf("[%c %d %d %d]\n", ev.ascii, ev.scan, ev.status, ev.flag);
#else
		ch = keyb_getch(NON_BLOCK);
		if (ch)
			cprintf("%c", ch);
#endif
		task_endcycle();
	}
}

void my_pause(KEY_EVT *e){

	TIME t;
	
	cprintf("Start Pause.\n");
	keyb_disable();
	cprintf("Keyboard Disabled.\n");
	t = sys_gettime(NULL);
	while ( ( (sys_gettime(NULL) -t) / 1000) < 5000);
	keyb_enable();
	cprintf("Keyboard Enabled.\n");
}

void my_test(KEY_EVT *e){
	cprintf("<test>\n");
}

void my_arrow(KEY_EVT *e){
	cprintf("<arrow>\n");
}

int main(int argc, char **argv)
{
	SOFT_TASK_MODEL mp;
	PID pid;

	KEY_EVT ev;
	
	ev.ascii = 'p';
	ev.scan  = KEY_P;
	ev.status = KEY_PRESSED;
	ev.flag = CNTR_BIT;
	keyb_hook(ev, my_pause, FALSE);

	set_keyevt(&ev, 'a', KEY_A, 0, KEY_PRESSED|KEY_REPEATED);
	keyb_hook(ev, my_test, FALSE);

	set_keyevt(&ev, 0, EXT_UP, 0, KEY_PRESSED);
	keyb_hook(ev, my_arrow, FALSE);

	set_keyevt(&ev, 'b', KEY_B, CNTL_BIT, KEY_PRESSED);
	keyb_hook(ev, my_test, TRUE);

	ev.ascii = 'c';
	ev.scan  = KEY_C;
	ev.status = KEY_PRESSED;
	ev.flag = CNTR_BIT;
	keyb_hook(ev, my_sysclose, FALSE);
	ev.flag = CNTL_BIT;
	keyb_hook(ev, my_sysclose, FALSE);
	
	soft_task_default_model(mp);
	soft_task_def_level(mp,2);
	soft_task_def_ctrl_jet(mp);
	soft_task_def_met(mp,700);
	soft_task_def_period(mp,1000);
	soft_task_def_usemath(mp);
	pid = task_create("Keyb_Print", my_get, &mp, NULL);
	if (pid == NIL) {
		perror("Could not create task <Keyb_Print>");
		exit(1);
	} else
		task_activate(pid);

	return 0;
}
