/*
 *  Input driver speaker module
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

//#define DEBUG_SPK

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION("Input driver joystick module"); 
MODULE_LICENSE("GPL");

static char speaker_name[] = "speaker";
static struct input_handler speaker_handler;

/*
 * Making beeps and bells. 
 */
void spk_nosound(unsigned long ignored)
{
	struct list_head * node;

	list_for_each(node,&speaker_handler.h_list) {
		struct input_handle *handle = to_handle_h(node);
		if (test_bit(EV_SND, handle->dev->evbit)) {
			if (test_bit(SND_TONE, handle->dev->sndbit))
				input_event(handle->dev, EV_SND, SND_TONE, 0);
			if (test_bit(SND_BELL, handle->dev->sndbit))
				input_event(handle->dev, EV_SND, SND_BELL, 0);
		}
	}
}

static struct timer_list spk_mksound_timer =
		TIMER_INITIALIZER(spk_nosound, 0, 0);

void spk_mksound(unsigned int hz, unsigned int ticks)
{
	struct list_head * node;

	del_timer(&spk_mksound_timer);

	if (hz) {
		list_for_each_prev(node,&speaker_handler.h_list) {
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
			mod_timer(&spk_mksound_timer, jiffies26 + ticks);
	} else
		spk_nosound(0);
}

static void speaker_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
	if (type != EV_SND)
		return;

#ifdef DEBUG_SPK
	printk(KERN_DEBUG "speaker.c: Event. Dev: %s, Type: %d, Code: %d, Value: %d\n", handle->dev->phys, type, code, value);
#endif
}

static struct input_handle *speaker_connect(struct input_handler *handler, struct input_dev *dev, struct input_device_id *id)
{
	struct input_handle *handle;

	if (!test_bit(EV_SND, dev->evbit)) 
		return NULL;
	
	if (!(handle = kmalloc(sizeof(struct input_handle), GFP_KERNEL)))
		return NULL;
	memset(handle, 0, sizeof(struct input_handle));

	handle->dev = dev;
	handle->handler = handler;
	handle->name = speaker_name;

	input_open_device(handle);

#ifdef DEBUG_SPK
	printk(KERN_DEBUG "speaker.c: Connected device: \"%s\", %s\n", dev->name, dev->phys);
#endif

	return handle;
}

static void speaker_disconnect(struct input_handle *handle)
{
#ifdef DEBUG_SPK
	printk(KERN_DEBUG "speaker.c: Disconnected device: %s\n", handle->dev->phys);
#endif	
	input_close_device(handle);

	kfree(handle);
}

static struct input_device_id speaker_ids[] = {
	{ .driver_info = 1 },	// Matches all devices
	{ },			// Terminating zero entry
};

/*static struct input_device_id speaker_ids[] = {
	{
                .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
                .evbit = { BIT(EV_SND) },
        },	

	{ },    // Terminating entry
};*/

MODULE_DEVICE_TABLE(input, speaker_ids);
	
static struct input_handler speaker_handler = {
	.event =	speaker_event,
	.connect =	speaker_connect,
	.disconnect =	speaker_disconnect,
	.name =		"speaker",
	.id_table =	speaker_ids,
};

int __init speaker_init(void)
{
	input_register_handler(&speaker_handler);
	return 0;
}

void __exit speaker_exit(void)
{
	input_unregister_handler(&speaker_handler);
}

module_init(speaker_init);
module_exit(speaker_exit);
