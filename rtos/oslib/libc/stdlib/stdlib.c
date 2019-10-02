/* Project:     OSLib
 * Description: The OS Construction Kit
 * Date:                1.6.2000
 * Idea by:             Luca Abeni & Gerardo Lamastra
 *
 * OSLib is an SO project aimed at developing a common, easy-to-use
 * low-level infrastructure for developing OS kernels and Embedded
 * Applications; it partially derives from the HARTIK project but it
 * currently is independently developed.
 *
 * OSLib is distributed under GPL License, and some of its code has
 * been derived from the Linux kernel source; also some important
 * ideas come from studying the DJGPP go32 extender.
 *
 * We acknowledge the Linux Community, Free Software Foundation,
 * D.J. Delorie and all the other developers who believe in the
 * freedom of software and ideas.
 *
 * For legalese, check out the included GPL license.
 */

#include <ll/i386/hw-func.h>

#include <ll/i386/stdlib.h>
#include <ll/unistd.h>

FILE(stdlib);

int ll_exit_mode = EXIT_MODE_REAL;

unsigned abs(int x)
{
	if (x < 0) return(-x);
	else return(x);
}

int ll_set_reboot(int mode)
{
	switch(mode) {
		case EXIT_MODE_HALT:
			ll_exit_mode = EXIT_MODE_HALT;
			break;
		case EXIT_MODE_COLD:
			ll_exit_mode = EXIT_MODE_COLD;
			break;
		case EXIT_MODE_WARM:
			ll_exit_mode = EXIT_MODE_WARM;
			break;
		default:
			ll_exit_mode = EXIT_MODE_REAL;
			break;
	}
	return ll_exit_mode;
}

void l1_exit(int code)
{
	extern void bios_restore(void);

	bios_restore();
	//printk("Exiting with mode: %d\n", ll_exit_mode);
	switch (ll_exit_mode) {
		case EXIT_MODE_HALT:
			halt();
			break;
		case EXIT_MODE_COLD:
			cold_reboot();
			break;
		case EXIT_MODE_WARM:
			warm_reboot();
			break;
		default:
			__exit(code);
			break;
	}
}
