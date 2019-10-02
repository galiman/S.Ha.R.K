/*
 * Some of this code has been taken from bitops.h
 * Copyright 1992, Linus Torvalds.
 *
 * Others functions has been added by
 * Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) Feb, 2003 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 */

#define ADDR (*(volatile long *) addr)

/**
 * ffs - find first bit set
 * @x: the word to search
 *
 * This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */


static __inline__ int DIDMA_ffs(int x)
{
	int r;

	__asm__("bsfl %1,%0\n\t"
		"jnz 1f\n\t"
		"movl $-1,%0\n"
		"1:" : "=r" (r) : "g" (x));
	return r;
}


/**
 * fls - find last bit set
 * @x: the word to search
 *
 * This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static __inline__ int DIDMA_fls(int x)
{
	int r;

	__asm__("bsrl %1,%0\n\t"
		"jnz 1f\n\t"
		"movl $-1,%0\n"
		"1:" : "=r" (r) : "g" (x));
	return r;
}

/**
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
/*
static __inline__ void __set_bit(int nr, volatile void * addr)
{
	__asm__(
		"btsl %1,%0"
		:"=m" (ADDR)
		:"Ir" (nr));
}
*/
/**
 * __clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is non-atomic and may be reordered.  However, it does
 * not contain a memory barrier, so if it is used for locking purposes,
 * you should call smp_mb__before_clear_bit() and/or smp_mb__after_clear_bit()
 * in order to ensure changes are visible on other processors.
 */
static __inline__ void __clear_bit(int nr, volatile void * addr)
{
	__asm__ __volatile__( 
		"btrl %1,%0"
		:"=m" (ADDR)
		:"Ir" (nr));
}




