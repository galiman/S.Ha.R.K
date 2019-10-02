
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
#include <drivers/shark_spk26.h>

void my_sysclose(KEY_EVT *e)
{
	speaker_sound(0, 0);
	
	kern_printf("S.Ha.R.K. closed.\n\n");
	exit(0);
}

void no_note(KEY_EVT *e){
	speaker_sound(0, 0);
}

void my_note(KEY_EVT *e){

	switch (e->scan) {
		case KEY_Q:
			speaker_sound(262, 0);
			break;
		case KEY_W:
			speaker_sound(277, 0);
			break;
		case KEY_E:
			speaker_sound(294, 0);
			break;
		case KEY_R:
			speaker_sound(311, 0);
			break;
		case KEY_T:
			speaker_sound(330, 0);
			break;
		case KEY_Y:
			speaker_sound(349, 0);
			break;
		case KEY_U:
			speaker_sound(370, 0);
			break;
		case KEY_I:
			speaker_sound(392, 0);
			break;
		case KEY_O:
			speaker_sound(415, 0);
			break;
		case KEY_P:
			speaker_sound(440, 0);
			break;
		case KEY_BRL:
			speaker_sound(466, 0);
			break;
		case KEY_BRR:
			speaker_sound(494, 0);
			break;
	}
}

int main(int argc, char **argv)
{
	KEY_EVT ev;

	ev.ascii = 'c';
	ev.scan  = KEY_C;
	ev.status = KEY_PRESSED;
	ev.flag = CNTL_BIT;
	keyb_hook(ev, my_sysclose, FALSE);
	ev.flag = CNTR_BIT;
	keyb_hook(ev, my_sysclose, FALSE);

	speaker_sound(440, 400);
	while ( (sys_gettime(NULL)/1000) < 1000);

	ev.ascii = 'q';
	ev.scan  = KEY_Q;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'w';
	ev.scan  = KEY_W;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'e';
	ev.scan  = KEY_E;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'r';
	ev.scan  = KEY_R;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 't';
	ev.scan  = KEY_T;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'y';
	ev.scan  = KEY_Y;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'u';
	ev.scan  = KEY_U;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'i';
	ev.scan  = KEY_I;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'o';
	ev.scan  = KEY_O;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = 'p';
	ev.scan  = KEY_P;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = '[';
	ev.scan  = KEY_BRL;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	ev.ascii = ']';
	ev.scan  = KEY_BRR;
	ev.flag = 0;
	ev.status = KEY_PRESSED;
	keyb_hook(ev, my_note, FALSE);
	ev.status = KEY_RELEASED;
	keyb_hook(ev, no_note, FALSE);

	while(1);
	return 0;
}
