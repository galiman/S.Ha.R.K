
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

#include <drivers/shark_fb26.h>

#include <drivers/shark_mouse26.h>
#include <drivers/shark_keyb26.h>
#include <drivers/shark_spk26.h>
#include <drivers/shark_joy26.h>

#define RGB_BLACK     rgb16(  0,  0,  0)
#define RGB_GRAY      rgb16(127,127,127)
#define RGB_WHITE     rgb16(255,255,255)
#define RGB_RED       rgb16(255,  0,  0)
#define RGB_GREEN     rgb16(  0,255,  0)
#define RGB_BLUE      rgb16(  0,  0,255)
#define RGB_YELLOW    rgb16(255,255,  0)
#define RGB_MAGENTA   rgb16(255,  0,255)
#define RGB_CYAN      rgb16(  0,255,255)
#define RGB_D_RED     rgb16(127,  0,  0)
#define RGB_D_GREEN   rgb16(  0,127,  0)
#define RGB_D_BLUE    rgb16(  0,  0,127)
#define RGB_D_YELLOW  rgb16(127,127,  0)
#define RGB_D_MAGENTA rgb16(127,  0,127)
#define RGB_D_CYAN    rgb16(  0,127,127)

void my_sysend(KEY_EVT *e)
{
  exit(0);
}

void no_note(KEY_EVT *e)
{
	speaker_sound(0, 0);
}

void my_note(KEY_EVT *e)
{
	switch (e->scan) {
		case KEY_Q:
			speaker_sound(262, 0); /* DO */
			break;
		case KEY_W:
			speaker_sound(277, 0); /* DO# */
			break;
		case KEY_E:
			speaker_sound(294, 0); /* RE */
			break;
		case KEY_R:
			speaker_sound(311, 0); /* RE# */
			break;
		case KEY_T:
			speaker_sound(330, 0); /* MI */
			break;
		case KEY_Y:
			speaker_sound(349, 0); /* FA */
			break;
		case KEY_U:
			speaker_sound(370, 0); /* FA# */
			break;
		case KEY_I:
			speaker_sound(392, 0); /* SOL */
			break;
		case KEY_O:
			speaker_sound(415, 0); /* SOL# */
			break;
		case KEY_P:
			speaker_sound(440, 0); /* LA */
			break;
		case KEY_BRL:
			speaker_sound(466, 0); /* LA# */
			break;
		case KEY_BRR:
			speaker_sound(494, 0); /* SI */
			break;
	}
}

TASK my_getjoy(void *arg)
{
	int a0, a1, a2, a3, btn;
	char st[20];

	while (1) {
		joy_getstatus(&a0, &a1, &a2, &a3, &btn);

		sprintf(st, "X Axis : %6d ", a0);
		grx_text(st, 100,  64, RGB_CYAN, RGB_BLACK);
		sprintf(st, "Y Axis : %6d ", a1);
		grx_text(st, 100, 114, RGB_CYAN, RGB_BLACK);
		sprintf(st, "Buttons: %2x ", btn);
		grx_text(st, 100, 164, RGB_CYAN, RGB_BLACK);

		task_endcycle();
		if (btn == 0xF)
			my_sysend(NULL);
	}
}

TASK my_getch(void *arg)
{
#define  MYNCHAR 25

	BYTE ch;
	int i = 0;
	char st[20];

	while (1) {
		ch = keyb_getch(NON_BLOCK);
		if (ch) {
			if (ch == BACKSPACE) { //backspace
				i--;
				//ch = 0x20;
			}
			if (ch == ENTER) { //enter
				i = ((i / MYNCHAR) + 1) * MYNCHAR - 1;
				ch = 0x20;
			}
			sprintf(st, "%c", ch);
			grx_text(st, 340 + 10 * (i%MYNCHAR), 25 + 20 * (i/MYNCHAR), RGB_BLUE, RGB_BLACK);

			if (ch == BACKSPACE) //backspace
				i--;
			if (++i >= MYNCHAR * 9) {
				i = 0;
				grx_box(315,  15, 623, 223, RGB_BLACK);
			}
		}

		task_endcycle();
	}
}

void my_mouse(MOUSE_EVT *e)
{
	char st[20];
	char pressed = 0;

	sprintf(st, "X Axis : %3d (%4d)", e->x, e->dx);
	grx_text(st, 100, 280, RGB_YELLOW, RGB_BLACK);
	sprintf(st, "Y Axis : %3d (%4d)", e->y, e->dy);
	grx_text(st, 100, 320, RGB_YELLOW, RGB_BLACK);
	sprintf(st, "Z Axis : %3d (%4d)", e->z, e->dz);
	grx_text(st, 100, 360, RGB_YELLOW, RGB_BLACK);
	sprintf(st, "Buttons: %6x ", (int)e->buttons);
	grx_text(st, 100, 400, RGB_YELLOW, RGB_BLACK);

	if ((e->x > 377) && (e->x < 401) && (e->y > 300) && (e->y < 360) && (e->buttons == MOUSE_LBUTTON)){
		if (!pressed) {
			speaker_sound(277, 0); /* DO# */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 407) && (e->x < 431) && (e->y > 300) && (e->y < 360) && (e->buttons == MOUSE_LBUTTON)){
		if (!pressed) {
			speaker_sound(311, 0); /* RE# */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 467) && (e->x < 491) && (e->y > 300) && (e->y < 360) && (e->buttons == MOUSE_LBUTTON)){
		if (!pressed) {
			speaker_sound(370, 0); /* FA# */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 497) && (e->x < 521) && (e->y > 300) && (e->y < 360) && (e->buttons == MOUSE_LBUTTON)){
		if (!pressed) {
			speaker_sound(415, 0); /* SOL# */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 527) && (e->x < 551) && (e->y > 300) && (e->y < 360) && (e->buttons == MOUSE_LBUTTON)){
		if (!pressed) {
			speaker_sound(466, 0); /* LA# */
			pressed = 1;
		}
		return;
	}

	if ((e->x > 360) && (e->x < 388) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)){
		if (!pressed) {
			speaker_sound(262, 0); /* DO */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 390) && (e->x < 418) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)) {
		if (!pressed) {
			speaker_sound(294, 0); /* RE */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 420) && (e->x < 448) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)) {
		if (!pressed) {
			speaker_sound(330, 0); /* MI */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 450) && (e->x < 478) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)) {
		if (!pressed) {
			speaker_sound(349, 0); /* FA */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 480) && (e->x < 508) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)) {
		if (!pressed) {
			speaker_sound(392, 0); /* SOL */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 510) && (e->x < 538) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)) {
		if (!pressed) {
			speaker_sound(440, 0); /* LA */
			pressed = 1;
		}
		return;
	}
	if ((e->x > 540) && (e->x < 568) && (e->y > 300) && (e->y < 400) && (e->buttons == MOUSE_LBUTTON)) {
		if (!pressed) {
			speaker_sound(494, 0); /* SI */
			pressed = 1;
		}
		return;
	}
	
	speaker_sound(0, 0);
	pressed = 0;
}

void graph_init(void)
{
	grx_rect(  4,   4, 634, 474, RGB_WHITE);
	grx_rect( 14,  14, 304, 224, RGB_YELLOW);
	grx_rect(314,  14, 624, 224, RGB_RED);
	grx_rect( 14, 234, 304, 464, RGB_GREEN);
	grx_rect(314, 234, 624, 464, RGB_BLUE);

	/* Draw Teyboard */
	grx_box(360, 300, 388, 400, RGB_WHITE); /* DO  */
	grx_box(390, 300, 418, 400, RGB_WHITE); /* RE  */
	grx_box(420, 300, 448, 400, RGB_WHITE); /* MI  */
	grx_box(450, 300, 478, 400, RGB_WHITE); /* FA  */
	grx_box(480, 300, 508, 400, RGB_WHITE); /* SOL */
	grx_box(510, 300, 538, 400, RGB_WHITE); /* LA  */
	grx_box(540, 300, 568, 400, RGB_WHITE); /* SI  */

	grx_box(377, 301, 401, 360, RGB_BLACK); /* DO#  */
	grx_box(407, 301, 431, 360, RGB_BLACK); /* RE#  */
	grx_box(467, 301, 491, 360, RGB_BLACK); /* FA#  */
	grx_box(497, 301, 521, 360, RGB_BLACK); /* SOL# */
	grx_box(527, 301, 551, 360, RGB_BLACK); /* LA#  */
}

void start_sound(void)
{
	KEY_EVT ev;

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
	keyb_hook(ev, my_sysend, FALSE);
	ev.flag = CNTR_BIT;
	keyb_hook(ev, my_sysend, FALSE);

	graph_init();

	mouse_grxlimits(639, 479);
	mouse_setposition(319, 239, 0);
	mouse_hook(my_mouse);
	mouse_grxcursor(ENABLE, 2);

	soft_task_default_model(mp);
	soft_task_def_level(mp,2);
	soft_task_def_ctrl_jet(mp);
	soft_task_def_met(mp,700);
	soft_task_def_period(mp,10000);
	soft_task_def_usemath(mp);
	pid = task_create("Key_Print", my_getch, &mp, NULL);
	if (pid == NIL) {
		sys_shutdown_message("Could not create task <Key_Print>\n");
		exit(1);
	} else
		task_activate(pid);

	if (JOY26_installed()) {
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
	}

	start_sound();
	return 0;
}
