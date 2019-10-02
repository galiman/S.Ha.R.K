/*
 *  Input driver event debug module - dumps all events into syslog
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linuxcomp.h>

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/joystick.h>

//#define DEBUG_JOY

extern void shark_joy_exec(void);

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION("Input driver joystick module"); 
MODULE_LICENSE("GPL");

#define JOYDEV_MINOR_BASE	0
#define JOYDEV_MINORS		4	

static char joystick_name[] = "joystick";
static struct input_handler joystick_handler;

struct joydev {
	/*int exist;
	int open;*/
	int minor;
	char name[16];
	struct js_corr corr[ABS_MAX];
	int nabs;
	int nkey;
	__u16 keymap[KEY_MAX - BTN_MISC];
	__u16 keypam[KEY_MAX - BTN_MISC];
	__u8 absmap[ABS_MAX];
	__u8 abspam[ABS_MAX];
	__s16 abs[ABS_MAX];
};

static struct joydev *joydev_table[JOYDEV_MINORS];

/* Buffer Ssize */
#define JOY_BUFFERSIZE 256

/* Buffer Mask ( i=(i+1)&MASK is better than i=(i+1)%SIZE ) */
#define JOY_BUFFERMASK 0xff

/* Circular Buffer */
static struct js_event jbuffer[JOY_BUFFERSIZE];

/*
 * Buffer Pointers
 * data is inserted to jhead
 * data is kept from jtail+1
 * (jhead point to jtail+1 when buffer is empty)
 */
static unsigned jtail, jhead;

//static int tmp_axe[4], axe[4];

/*
 * Get data from the joystick
 */
int joystick_get(int *type, int *number, int *value)
{
	if ( ((jtail+1) & JOY_BUFFERMASK) == ((jhead) & JOY_BUFFERMASK) )
		return -1;
	jtail = (jtail+1) & JOY_BUFFERMASK;

	*type   = jbuffer[jtail].type;
	*number = jbuffer[jtail].number;
	*value  = jbuffer[jtail].value;

#ifdef DEBUG_JOY
	printk(KERN_DEBUG "joystick.c: ( %3d %3d %3d)\n", *type, *number, *value);
#endif
	return 0;
}

static int joystick_correct(int value, struct js_corr *corr)
{
	switch (corr->type) {
		case JS_CORR_NONE:
			break;
		case JS_CORR_BROKEN:
			value = value > corr->coef[0] ? (value < corr->coef[1] ? 0 :
				((corr->coef[3] * (value - corr->coef[1])) >> 14)) :
				((corr->coef[2] * (value - corr->coef[0])) >> 14);
			break;
		default:
			return 0;
	}

	if (value < -32767) return -32767;
	if (value >  32767) return  32767;

	return value;
}

static void joystick_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	struct joydev *joydev = handle->private;

	//printk(KERN_DEBUG "joystick.c: Event. Dev: %s, Type: %d, Code: %d, Value: %d\n", handle->dev->phys, type, code, value);

	switch (type) {

		case EV_KEY:
			if (code < BTN_MISC || value == 2) return;
#ifdef DEBUG_JOY
			printk(KERN_DEBUG "joystick.c: Event KEY. Code: %d, Value: %d\n", code, value);
#endif
			if (jtail != jhead) {
				jbuffer[jhead].type = JS_EVENT_BUTTON;
				jbuffer[jhead].number = joydev->keymap[code - BTN_MISC];
				jbuffer[jhead].value = value;
				jhead = (jhead+1) & JOY_BUFFERMASK;
			} else
				return;

			break;

		case EV_ABS:
#ifdef DEBUG_JOY
			printk(KERN_DEBUG "joystick.c: Event ABS. Code: %d, Value: %d\n", code, value);
#endif
			if (jtail != jhead) {
				jbuffer[jhead].type = JS_EVENT_AXIS;
				jbuffer[jhead].number = joydev->absmap[code];
				jbuffer[jhead].value = joystick_correct(value, joydev->corr + jbuffer[jhead].number);
				if (jbuffer[jhead].value == joydev->abs[jbuffer[jhead].number])
					return;
				joydev->abs[jbuffer[jhead].number] = jbuffer[jhead].value;
				jhead = (jhead+1) & JOY_BUFFERMASK;
			} else
				return;
			
			break;

		case EV_SYN:
#ifdef DEBUG_JOY
			printk(KERN_DEBUG "joystick.c: Event SYN\n");
#endif
			return;

		default:
			return;
	}

	shark_joy_exec();
}

static struct input_handle *joystick_connect(struct input_handler *handler, struct input_dev *dev, struct input_device_id *id)
{
	struct input_handle *handle;
	struct joydev *joydev;
	int i, j, t, minor;

	/* Avoid tablets */
        if (test_bit(EV_KEY, dev->evbit) && test_bit(BTN_TOUCH, dev->keybit))
		return NULL;

	for (minor = 0; minor < JOYDEV_MINORS && joydev_table[minor]; minor++);
	if (minor == JOYDEV_MINORS) {
		printk(KERN_ERR "joydev: no more free joydev devices\n");
		return NULL;
	}
	if (!(joydev = kmalloc(sizeof(struct joydev), GFP_KERNEL)))
		return NULL;
	memset(joydev, 0, sizeof(struct joydev));
	
	if (!(handle = kmalloc(sizeof(struct input_handle), GFP_KERNEL)))
		return NULL;
	memset(handle, 0, sizeof(struct input_handle));

	handle->dev = dev;
	handle->handler = handler;
	handle->name = joystick_name;
	handle->private = joydev;
	
	joydev->minor = minor;
	sprintf26(joydev->name, "js%d", minor);

	for (i = 0; i < ABS_MAX; i++)
		if (test_bit(i, dev->absbit)) {
			joydev->absmap[i] = joydev->nabs;
			joydev->abspam[joydev->nabs] = i;
			joydev->nabs++;
		}

	for (i = BTN_JOYSTICK - BTN_MISC; i < KEY_MAX - BTN_MISC; i++)
		if (test_bit(i + BTN_MISC, dev->keybit)) {
			joydev->keymap[i] = joydev->nkey;
			joydev->keypam[joydev->nkey] = i + BTN_MISC;
			joydev->nkey++;
		}

	for (i = 0; i < BTN_JOYSTICK - BTN_MISC; i++)
		if (test_bit(i + BTN_MISC, dev->keybit)) {
			joydev->keymap[i] = joydev->nkey;
			joydev->keypam[joydev->nkey] = i + BTN_MISC;
			joydev->nkey++;
		}

	for (i = 0; i < joydev->nabs; i++) {
		j = joydev->abspam[i];
		if (dev->absmax[j] == dev->absmin[j]) {
			joydev->corr[i].type = JS_CORR_NONE;
			continue;
		}
		joydev->corr[i].type = JS_CORR_BROKEN;
		joydev->corr[i].prec = dev->absfuzz[j];
		joydev->corr[i].coef[0] = (dev->absmax[j] + dev->absmin[j]) / 2 - dev->absflat[j];
		joydev->corr[i].coef[1] = (dev->absmax[j] + dev->absmin[j]) / 2 + dev->absflat[j];
		if (!(t = ((dev->absmax[j] - dev->absmin[j]) / 2 - 2 * dev->absflat[j])))
			continue;
		joydev->corr[i].coef[2] = (1 << 29) / t;
		joydev->corr[i].coef[3] = (1 << 29) / t;

		joydev->abs[i] = joystick_correct(dev->abs[j], joydev->corr + i);
	}

	joydev_table[minor] = joydev;
	
	input_open_device(handle);

#ifdef DEBUG_JOY
	printk(KERN_DEBUG "joystick.c: Connected device: \"%s\", %s\n", dev->name, dev->phys);
#endif

	return handle;
}

static void joystick_disconnect(struct input_handle *handle)
{
	struct joydev *joydev = handle->private;

	joydev_table[joydev->minor] = NULL;
	kfree(joydev);
	
	input_close_device(handle);

	kfree(handle);

#ifdef DEBUG_JOY
	printk(KERN_DEBUG "joystick.c: Disconnected device: %s\n", handle->dev->phys);
#endif	
}

static struct input_device_id joystick_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT(EV_ABS) },
		.absbit = { BIT(ABS_X) },
	},
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT(EV_ABS) },
		.absbit = { BIT(ABS_WHEEL) },
	},
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT(EV_ABS) },
		.absbit = { BIT(ABS_THROTTLE) },
	},
	{ }, 	/* Terminating entry */
};

MODULE_DEVICE_TABLE(input, joystick_ids);
	
static struct input_handler joystick_handler = {
	.event =	joystick_event,
	.connect =	joystick_connect,
	.disconnect =	joystick_disconnect,
	.name =		"joystick",
	.id_table =	joystick_ids,
};

int __init joystick_init(void)
{
	/* Initialize Buffer Variables */
	jhead=1;
	jtail=0;

	input_register_handler(&joystick_handler);
	return 0;
}

void __exit joystick_exit(void)
{
	input_unregister_handler(&joystick_handler);
}

module_init(joystick_init);
module_exit(joystick_exit);
