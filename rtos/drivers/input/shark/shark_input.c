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

#include <kernel/func.h>

/* System */
extern int input_init(void);
extern int input_exit(void);

extern int serio_init(void);
extern int serio_exit(void);

/* Controllers */
extern int i8042_init(void);
extern int i8042_exit(void);

/*extern int serport_init(void);
extern int serport_exit(void);*/

/* Handlers */
extern int evbug_init(void);
extern int evbug_exit(void);

static int	input_installed = FALSE;
static int	evbug_installed = FALSE;

/* Init the Linux Input Layer */
int INPUT26_installed(void)
{
	return input_installed;
}

int INPUT26_init() {

	int ret;
	
	if (input_installed == TRUE) return 0;

	ret = input_init();
	if (ret) {
		printk(KERN_ERR "Input_Init return: %d\n", ret);
		return -1;
	}

	ret = serio_init();
	if (ret) {
		printk(KERN_ERR "Serio_Init return: %d\n", ret);
		return -2;
	}

	ret = i8042_init();
	if (ret) {
		printk(KERN_ERR "i8042_Init return: %d\n", ret);
		return -3;
	}

	/* TODO
	ret = serport_init();
	if (ret) {
		printk(KERN_ERR "SerPort_Init return: %d\n", ret);
		return -1;
	} */

	input_installed = TRUE;

	return ret;
}

int INPUT26_close() {

	if (input_installed == TRUE) {
		i8042_exit();
		serio_exit();
		input_exit();
		
		return 0;
	} else
		return -1;
}

/* Init the Linux Event Debug Driver */
int EVBUG26_installed(void)
{
	return evbug_installed;
}

int EVBUG26_init() {
	evbug_init();

	evbug_installed = TRUE;

	return 0;
}

int EVBUG26_close() {
	evbug_exit();

	evbug_installed = FALSE;

	return 0;
}

