/*
 *  Input driver mouse module
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
 */

#include <linuxcomp.h>

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/device.h>

//#define DEBUG_MOUSE

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION("Input driver mouse module"); 
MODULE_LICENSE("GPL");

extern void shark_mouse_exec(void);

#ifndef CONFIG_INPUT_MOUSEDEV_SCREEN_X
#define CONFIG_INPUT_MOUSEDEV_SCREEN_X	1024
#endif
#ifndef CONFIG_INPUT_MOUSEDEV_SCREEN_Y
#define CONFIG_INPUT_MOUSEDEV_SCREEN_Y	768
#endif

static int xres = CONFIG_INPUT_MOUSEDEV_SCREEN_X;
static int yres = CONFIG_INPUT_MOUSEDEV_SCREEN_Y;

/* Buffer Ssize */
#define MOUSE_BUFFERSIZE 256

/* Buffer Mask ( i=(i+1)&MASK is better than i=(i+1)%SIZE ) */
#define MOUSE_BUFFERMASK 0xff

/* Circular Buffer */
static struct mouse_event {
	unsigned long buttons;
	int dx, dy, dz;
} mbuffer[MOUSE_BUFFERSIZE];

/*
 * Buffer Pointers
 * data is inserted to mhead
 * data is kept from mtail+1
 * (mhead point to mtail+1 when buffer is empty)
 */
static unsigned mtail, mhead;

static char mouse_name[] = "mouse";

struct mouse_list {
	int dx, dy, dz, oldx, oldy;
	signed char ps2[6];
	unsigned long buttons;
	/*unsigned char ready, buffer, bufsiz;
	unsigned char mode, imexseq, impsseq;*/
	int finger;
};

static struct mouse_list m_list;
static struct mouse_list *list = &m_list;

/*
 * Get data from the mouse
 */
int mouse_get(int *dx, int *dy, int *dz, unsigned long *buttons)
{
	if ( ((mtail+1) & MOUSE_BUFFERMASK) == ((mhead) & MOUSE_BUFFERMASK) )
		return -1;
		
	mtail = (mtail+1) & MOUSE_BUFFERMASK;
	
	*dx      = mbuffer[mtail].dx;
	*dy      = mbuffer[mtail].dy;
	*dz      = mbuffer[mtail].dz;
	*buttons = mbuffer[mtail].buttons;
	
#ifdef DEBUG_MOUSE
	printk(KERN_DEBUG "mouse.c: ( %3d %3d %3d - %6x)\n", *dx, *dy, *dz, *(int *)buttons);
#endif
	return 0;
}

static void mouse_abs_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	int size;

	/* Ignore joysticks */
	if (test_bit(BTN_TRIGGER, handle->dev->keybit))
		return;

	/* Handle touchpad data */
	if (test_bit(BTN_TOOL_FINGER, handle->dev->keybit)) {

		if (list->finger && list->finger < 3)
			list->finger++;

		switch (code) {
			case ABS_X:
				if (list->finger == 3)
					list->dx += (value - list->oldx) / 8;
				list->oldx = value;
				return;
			case ABS_Y:
				if (list->finger == 3)
					list->dy -= (value - list->oldy) / 8;
				list->oldy = value;
				return;
		}
		return;
	}

	/* Handle tablet data */
	switch (code) {
		case ABS_X:
			size = handle->dev->absmax[ABS_X] - handle->dev->absmin[ABS_X];
			if (size == 0) size = xres;
			list->dx += (value * xres - list->oldx) / size;
			list->oldx += list->dx * size;
			return;
		case ABS_Y:
			size = handle->dev->absmax[ABS_Y] - handle->dev->absmin[ABS_Y];
			if (size == 0) size = yres;
			list->dy -= (value * yres - list->oldy) / size;
			list->oldy -= list->dy * size;
			return;
	}
}

static void mouse_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	int index;

#ifdef DEBUG_MOUSE
	printk(KERN_DEBUG "mouse.c: Event. Dev: %s, Type: %d, Code: %d, Value: %d\n", handle->dev->phys, type, code, value);
#endif

	switch (type) {
		case EV_ABS:
			mouse_abs_event(handle, type, code, value);
			break;

		case EV_REL:
			switch (code) {
				case REL_X:	list->dx += value; break;
				case REL_Y:	list->dy -= value; break;
				case REL_WHEEL:	list->dz -= value; break;
			}
			break;

		case EV_KEY:
			switch (code) {
				case BTN_TOUCH: /* Handle touchpad data */
					if (test_bit(BTN_TOOL_FINGER, handle->dev->keybit)) {
						list->finger = value;
						return;
					}
				case BTN_0:
				case BTN_FORWARD:
				case BTN_LEFT:   index = 0; break;
				case BTN_4:
				case BTN_EXTRA:  index = 4; break;
				case BTN_STYLUS:
				case BTN_1:
				case BTN_RIGHT:  index = 1; break;
				case BTN_3:
				case BTN_BACK:
				case BTN_SIDE:   index = 3; break;
				case BTN_2:
				case BTN_STYLUS2:
				case BTN_MIDDLE: index = 2; break;	
				default: return;
			}
			switch (value) {
				case 0: clear_bit(index, &list->buttons); break;
				case 1: set_bit(index, &list->buttons); break;
				case 2: return;
			}
			break;

		case EV_SYN:
			switch (code) {
				case SYN_REPORT:
					if (mtail != mhead) {
						mbuffer[mhead].buttons = list->buttons;
						mbuffer[mhead].dx = list->dx;
						mbuffer[mhead].dy = list->dy;
						mbuffer[mhead].dz = list->dz;
						mhead = (mhead+1) & MOUSE_BUFFERMASK;
						list->dx = list->dy = list->dz = 0;
					}
					shark_mouse_exec();
					break;
			}
	}
}

static struct input_handle *mouse_connect(struct input_handler *handler, struct input_dev *dev, struct input_device_id *id)
{
	struct input_handle *handle;

	if (!(handle = kmalloc(sizeof(struct input_handle), GFP_KERNEL)))
		return NULL;
	memset(handle, 0, sizeof(struct input_handle));

	handle->dev = dev;
	handle->handler = handler;
	handle->name = mouse_name;

	input_open_device(handle);

#ifdef DEBUG_MOUSE
	printk(KERN_DEBUG "mouse.c: Connected device: \"%s\", %s\n", dev->name, dev->phys);
#endif

	return handle;
}

static void mouse_disconnect(struct input_handle *handle)
{
#ifdef DEBUG_MOUSE
	printk(KERN_DEBUG "mouse.c: Disconnected device: %s\n", handle->dev->phys);
#endif
	
	input_close_device(handle);

	kfree(handle);
}

static struct input_device_id mouse_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT | INPUT_DEVICE_ID_MATCH_RELBIT,
		.evbit = { BIT(EV_KEY) | BIT(EV_REL) },
		.keybit = { [LONG(BTN_LEFT)] = BIT(BTN_LEFT) },
		.relbit = { BIT(REL_X) | BIT(REL_Y) },
	},	// A mouse like device, at least one button, two relative axes
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_RELBIT,
		.evbit = { BIT(EV_KEY) | BIT(EV_REL) },
		.relbit = { BIT(REL_WHEEL) },
	},	// A separate scrollwheel
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT | INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT(EV_KEY) | BIT(EV_ABS) },
		.keybit = { [LONG(BTN_TOUCH)] = BIT(BTN_TOUCH) },
		.absbit = { BIT(ABS_X) | BIT(ABS_Y) },
	},	// A tablet like device, at least touch detection, two absolute axes
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT | INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT(EV_KEY) | BIT(EV_ABS) },
		.keybit = { [LONG(BTN_TOOL_FINGER)] = BIT(BTN_TOOL_FINGER) },
		.absbit = { BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE) | BIT(ABS_TOOL_WIDTH) },
	},	// A touchpad

	{ }, 	// Terminating entry
};

MODULE_DEVICE_TABLE(input, mouse_ids);
	
static struct input_handler mouse_handler = {
	.event =	mouse_event,
	.connect =	mouse_connect,
	.disconnect =	mouse_disconnect,
	.name =		"mouse",
	.id_table =	mouse_ids,
};

int __init mouse_init(void)
{
	/* Initialize Buffer Variables */
	mhead=1;
	mtail=0;

	input_register_handler(&mouse_handler);
	return 0;
}

void __exit mouse_exit(void)
{
	input_unregister_handler(&mouse_handler);
}

module_init(mouse_init);
module_exit(mouse_exit);
