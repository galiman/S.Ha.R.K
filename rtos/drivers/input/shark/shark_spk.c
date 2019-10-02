/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *   (see the web pages for full authors list)
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

//#define __MOUSE_DEBUG__ 1

#include <kernel/kern.h>

#include "../include/drivers/shark_input26.h"
#include "../include/drivers/shark_spk26.h"

/* Devices */
extern int pcspkr_init(void);
extern int pcspkr_exit(void);

/* Handlers */
extern int  speaker_init(void);
extern void speaker_exit(void);

/* Functions */
extern void spk_mksound(unsigned int hz, unsigned int ticks);
extern void spk_nosound(unsigned long ignored);

static int speaker_installed;

/* User Functions */
void speaker_sound(unsigned int hz, unsigned int ticks)
{
	spk_mksound(hz, ticks);
}

void speaker_mute(void)
{
	spk_nosound(0);
}

/* Init the Linux Speaker Driver */
int SPEAK26_installed()
{
	return speaker_installed;
}

int SPEAK26_init()
{
	int ret;

	if (speaker_installed == TRUE) return 0;

	if (INPUT26_installed() == FALSE)
		if (INPUT26_init()) {
			printk(KERN_ERR "Unable to open Input SubSystem.\n");
			return -1;
		}

	ret = pcspkr_init();
	if (ret) {
		printk(KERN_ERR "PcSpkr_Init return: %d\n", ret);
		return -1;
	}

	ret = speaker_init();
	if (ret) {
		printk(KERN_ERR "Speaker_Init return: %d\n", ret);
		return -1;
	}

	speaker_installed = TRUE;

	return 0;
}

int SPEAK26_close()
{
	if (!speaker_installed)
		return -1;

	speaker_exit();
	pcspkr_exit();

	speaker_installed = TRUE;

	return 0;
}

