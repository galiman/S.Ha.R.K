#ifndef _I386_DELAY_H
#define _I386_DELAY_H

/*
 * Copyright (C) 1993 Linus Torvalds
 *
 * Delay routines calling functions in arch/i386/lib/delay.c
 */
 
extern void __bad_udelay(void);
extern void __bad_ndelay(void);

extern void __udelay(unsigned long usecs);
extern void __ndelay(unsigned long nsecs);
extern void __const_udelay(unsigned long usecs);
extern void __delay(unsigned long loops);

/**
 * Remap on linking traps and longer delays removed.
 * Should not be needed in Shark.
 */

/*
#define udelay(n) (__builtin_constant_p(n) ? \
	((n) > 20000 ? __bad_udelay() : __const_udelay((n) * 0x10c6ul)) : \
	__udelay(n))
*/
#define udelay(n) __udelay(n)

/*
#define ndelay(n) (__builtin_constant_p(n) ? \
	((n) > 20000 ? __bad_ndelay() : __const_udelay((n) * 5ul)) : \
	__ndelay(n))
*/
#define ndelay(n) __ndelay(n)

#endif /* defined(_I386_DELAY_H) */
