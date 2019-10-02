/*
 *  Input driver keyboard module
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

#include <linux/kbd_kern.h>
#include <linux/kbd_diacr.h>

//#define DEBUG_KBD

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION("Input driver keyboard module"); 
MODULE_LICENSE("GPL");

extern void shark_kbd_exec(void);

/* Buffer Ssize */
#define KEYB_BUFFERSIZE 256

/* Buffer Mask ( i=(i+1)&MASK is better than i=(i+1)%SIZE ) */
#define KEYB_BUFFERMASK 0xff

/* Circular Buffer */
static struct keyb_event {
	unsigned int keycode;
	int down;
} kbuffer[KEYB_BUFFERSIZE];

/*
 * Buffer Pointers
 * data is inserted to khead
 * data is kept from ktail+1
 * (khead point to ktail+1 when buffer is empty)
 */
static unsigned ktail, khead;

static char kbd_name[] = "kbd";
static struct input_handler kbd_handler;

static unsigned char ledstate = 0xff;			/* undefined */

/*
 * Start / Stop Keyboard
 */
int kbd_enable(void)
{
	struct list_head *node;

	list_for_each(node,&kbd_handler.h_list) {
		struct input_handle *handle = to_handle_h(node);
		struct input_dev *dev = handle->dev;

		if (test_bit(EV_REP, dev->evbit)) {
			input_event(dev, EV_PWR, 1, 0);
		}
	}

	return 0;
}

int kbd_disable(void)
{
	struct list_head *node;

	list_for_each(node,&kbd_handler.h_list) {
		struct input_handle *handle = to_handle_h(node);
		struct input_dev *dev = handle->dev;

		if (test_bit(EV_REP, dev->evbit)) {
			input_event(dev, EV_PWR, 0, 0);
		}
	}

	return 0;
}

/*
 * Get data from the keyboard (primary port)
 *
 * it's follow the port_receive() semantic
 */
int kbd_get(unsigned int *data, BYTE access)
{
	if ( ((ktail+1) & KEYB_BUFFERMASK) == ((khead) & KEYB_BUFFERMASK) )
		return -1;
		
	ktail = (ktail+1) & KEYB_BUFFERMASK;
	*data = kbuffer[ktail].keycode;
	
	return kbuffer[ktail].down;
}

/*
 * Making beeps and bells. 
 */
static void kbd_nosound(unsigned long ignored)
{
	struct list_head * node;

	list_for_each(node,&kbd_handler.h_list) {
		struct input_handle *handle = to_handle_h(node);
		if (test_bit(EV_SND, handle->dev->evbit)) {
			if (test_bit(SND_TONE, handle->dev->sndbit))
				input_event(handle->dev, EV_SND, SND_TONE, 0);
			if (test_bit(SND_BELL, handle->dev->sndbit))
				input_event(handle->dev, EV_SND, SND_BELL, 0);
		}
	}
}

static struct timer_list kd_mksound_timer =
		TIMER_INITIALIZER(kbd_nosound, 0, 0);

void kbd_mksound(unsigned int hz, unsigned int ticks)
{
	struct list_head * node;

	del_timer(&kd_mksound_timer);

	if (hz) {
		list_for_each_prev(node,&kbd_handler.h_list) {
			struct input_handle *handle = to_handle_h(node);
			if (test_bit(EV_SND, handle->dev->evbit)) {
				if (test_bit(SND_TONE, handle->dev->sndbit)) {
					input_event(handle->dev, EV_SND, SND_TONE, hz);
					break;
				}
				if (test_bit(SND_BELL, handle->dev->sndbit)) {
					input_event(handle->dev, EV_SND, SND_BELL, 1);
					break;
				}
			}
		}
		if (ticks)
			mod_timer(&kd_mksound_timer, jiffies26 + ticks);
	} else
		kbd_nosound(0);
}

/*
 * Setting the keyboard rate.
 */

//int kbd_rate(struct kbd_repeat *rep)
int kbd_rate(unsigned int *delay, unsigned int *period)
{
	struct list_head *node;
	unsigned int d = 0;
	unsigned int p = 0;

	list_for_each(node,&kbd_handler.h_list) {
		struct input_handle *handle = to_handle_h(node);
		struct input_dev *dev = handle->dev;

		if (test_bit(EV_REP, dev->evbit)) {
			if (*delay > 0)
				input_event(dev, EV_REP, REP_DELAY, *delay);
			if (*period > 0)
				input_event(dev, EV_REP, REP_PERIOD, *period);
			d = dev->rep[REP_DELAY];
			p = dev->rep[REP_PERIOD];
		}
	}
	*delay  = d;
	*period = p;
	return 0;
}

/*
 * This allows a keyboard to pick the LED state.
 */
void kbd_refresh_leds(void)
{
	struct list_head * node;
	unsigned char leds = ledstate;
		
	list_for_each(node,&kbd_handler.h_list) {
		struct input_handle * handle = to_handle_h(node);
		input_event(handle->dev, EV_LED, LED_SCROLLL, !!(leds & 0x01));
		input_event(handle->dev, EV_LED, LED_NUML,    !!(leds & 0x02));
		input_event(handle->dev, EV_LED, LED_CAPSL,   !!(leds & 0x04));
		input_sync(handle->dev);
	}
}

void kbd_setleds(unsigned int led)
{
	ledstate = led;
	kbd_refresh_leds();
}

static void kbd_event(struct input_handle *handle, unsigned int type, unsigned int keycode, int down)
{
	if (type != EV_KEY)
		return;

#ifdef DEBUG_KBD
	printk(KERN_DEBUG "keyboard.c: Event. Dev: %s, Type: %d, Code: %d, Value: %d\n", handle->dev->phys, type, keycode, down);
#endif

	if (ktail != khead) {
		kbuffer[khead].keycode = keycode;
		kbuffer[khead].down = down;
		khead = (khead+1) & KEYB_BUFFERMASK;
	}

	shark_kbd_exec();
}

static struct input_handle *kbd_connect(struct input_handler *handler, struct input_dev *dev, struct input_device_id *id)
{
	struct input_handle *handle;
	int i;

	for (i = KEY_RESERVED; i < BTN_MISC; i++)
		if (test_bit(i, dev->keybit)) break;

	if ((i == BTN_MISC) && !test_bit(EV_SND, dev->evbit)) 
		return NULL;

	if (!(handle = kmalloc(sizeof(struct input_handle), GFP_KERNEL)))
		return NULL;
	memset(handle, 0, sizeof(struct input_handle));

	handle->dev = dev;
	handle->handler = handler;
	handle->name = kbd_name;

	input_open_device(handle);
#ifdef DEBUG_KBD
	printk(KERN_DEBUG "keyboard.c: Connected device: \"%s\", %s\n", dev->name, dev->phys);
#endif
	return handle;
}

static void kbd_disconnect(struct input_handle *handle)
{
#ifdef DEBUG_KBD
	printk(KERN_DEBUG "keyboard.c: Disconnected device: %s\n", handle->dev->phys);
#endif
	
	input_close_device(handle);

	kfree(handle);
}

static struct input_device_id kbd_ids[] = {
	{
                .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
                .evbit = { BIT(EV_KEY) },
        },
	
	{
                .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
                .evbit = { BIT(EV_SND) },
        },	

	{ },    /* Terminating entry */
};

MODULE_DEVICE_TABLE(input, kbd_ids);
	
static struct input_handler kbd_handler = {
	.event =	kbd_event,
	.connect =	kbd_connect,
	.disconnect =	kbd_disconnect,
	.name =		"kbd",
	.id_table =	kbd_ids,
};

int __init kbd_init(void)
{
	/* Initialize Buffer Variables */
	khead=1;
	ktail=0;

	input_register_handler(&kbd_handler);

	return 0;
}

void __exit kbd_exit(void)
{
	input_unregister_handler(&kbd_handler);
}

module_init(kbd_init);
module_exit(kbd_exit);
