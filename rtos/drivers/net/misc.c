#include <string.h>
#include <kernel/kern.h>

void panic_stub(void)
{
	cprintf("Panic: stub called!!!\n");

	exit(200);			/* Just a number... */
}

