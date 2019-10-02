/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
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

/* Glue Layer for Linux PCI 2.6 Driver */
    
#include <kernel/kern.h>

extern int bttv_init_module(void);
extern void bttv_cleanup_module(void);

static int bttv_installed = FALSE;

/* Init the Linux BTTV 2.6 Driver */
int BTTV26_installed(void)
{
	return bttv_installed;
}

extern int videodev_init(void);

int BTTV26_init() {

	int res;

	if (bttv_installed == TRUE)
		return 0;

	videodev_init();

	res = bttv_init_module();

	if (res != 0) {
		sys_shutdown_message("ERROR: Cannot initialize BTTV\n");
		exit(1);
	}

	bttv_installed = TRUE;

	return 0;

}

int BTTV26_close() {

	if (bttv_installed == FALSE)
		return 0;

	bttv_cleanup_module();

	return 0;

}
