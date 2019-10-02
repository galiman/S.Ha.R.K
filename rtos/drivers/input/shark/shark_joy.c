/*
 * Project: S.Ha.R.K.
 *
 * Coordinators:
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     :
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *
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

//#define __JOY_DEBUG__
//#define __JOY_DUMP__

#include <kernel/kern.h>

#include "../include/drivers/shark_input26.h"
#include "../include/drivers/shark_joy26.h"

#include <kernel/func.h>

/* Devices */
extern int ns558_init(void);
extern int ns558_exit(void);

extern int analog_init(void);
extern int analog_exit(void);

extern int joydump_init(void);
extern int joydump_exit(void);

/* Handler */
extern int joystick_init(void);
extern int joystick_exit(void);

/* Functions */
extern int joystick_get(int *type, int *number, int *value);

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

/* joystick driver currently installed */
static int joystick_installed = FALSE;
static int joystick_enabled = FALSE;

static int axis[4], button;

/* Called by handler */
void shark_joy_exec(void) {
	int type, number, value;

	if (joystick_enabled == FALSE)
		return;

	if (joystick_get(&type, &number, &value))
		return;

	switch (type) {					/* TODO */
		case JS_EVENT_BUTTON:
			if (value)
				button |= 1 << number;
			else
				button &= ~(1 << number);
			break;
		case JS_EVENT_AXIS:
			axis[number] = value;
			break;
		default:
			return;
	}
#ifdef __JOY_DEBUG__
	printk(KERN_DEBUG "shark_joy.c: (%4d,%4d) (%4d,%4d) %4x\n", axis[0], axis[1], axis[2], axis[3], button);
#endif
}

/* User Functions */
void joy_getstatus(int *axe0, int *axe1, int *axe2, int *axe3, int *buttons)
{
	if (axe0)
		*axe0 = axis[0];
	if (axe1)
		*axe1 = axis[1];
	if (axe2)
		*axe2 = axis[2];
	if (axe3)
		*axe3 = axis[3];
	if (buttons)
		*buttons = button;
}

void joy_setstatus(int axe0, int axe1, int axe2, int axe3, int buttons)
{
	if ((axe0 > -32767) && (axe0 < 32767))
		axis[0] = axe0;
	if ((axe1 > -32767) && (axe1 < 32767))
		axis[1] = axe1;
	if ((axe2 > -32767) && (axe2 < 32767))
		axis[2] = axe2;
	if ((axe3 > -32767) && (axe3 < 32767))
		axis[3] = axe3;
	button = buttons;
}

void joy_enable(void)
{
	joystick_enabled = TRUE;
#ifdef __JOY_DEBUG__
	printk("shark_joy.c: Joystick Enabled.\n");
#endif
}

void joy_disable(void)
{
	joystick_enabled = FALSE;
#ifdef __JOY_DEBUG__
	printk("shark_joy.c: Joystick Disabled.\n");
#endif
}


/* Init the Linux Joystick Driver */
int JOY26_installed(void) {
	return joystick_installed;
}

int JOY26_init(void) {

	int ret;

	if (INPUT26_installed() == FALSE)
		if (INPUT26_init()) {
			printk(KERN_ERR "Unable to open Input SubSystem.\n");
			return -1;
		}

	ret = ns558_init();
	//if (ret) {
	//	printk(KERN_ERR "Gameport_Init return: %d\n", ret);
	//	return -1;
	//}

#ifdef __JOY_DUMP__
	ret = joydump_init();
#else
	ret = analog_init();
#endif
	if (ret) {
		printk(KERN_ERR "Joystick_Device_Init return: %d\n", ret);
		return -1;
	}

	ret = joystick_init();
	if (ret) {
		printk(KERN_ERR "Joystick_Handler_Init return: %d\n", ret);
		return -1;
	}

	joystick_installed = TRUE;
	joystick_enabled = TRUE;

	return 0;
}

int JOY26_close(void) {
	if (!joystick_installed)
		return -1;

	joystick_enabled = FALSE;
	joystick_exit();
#ifdef __JOY_DUMP__
	joydump_exit();
#else
	analog_exit();
#endif
	ns558_exit();

	joystick_installed = FALSE;
	return 0;
}
