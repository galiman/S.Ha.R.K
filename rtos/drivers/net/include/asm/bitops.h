#ifndef __BITOPS__
#define __BITOPS__

#include <linux/compatib.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define LOCK_PREFIX ""
#define SMPVOL

/*
 * Some hacks to defeat gcc over-optimizations..
 */
struct __dummy { unsigned long a[100]; };
#define ADDR (*(struct __dummy *) addr)
#define CONST_ADDR (*(const struct __dummy *) addr)

extern __inline__ int set_bit(int nr, SMPVOL void * addr)
{
	int oldbit;

	__asm__ __volatile__(LOCK_PREFIX
		"btsl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"=m" (ADDR)
		:"ir" (nr));
	return oldbit;
}

extern __inline__ int clear_bit(int nr, SMPVOL void * addr)
{
	int oldbit;

	__asm__ __volatile__(LOCK_PREFIX
		"btrl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"=m" (ADDR)
		:"ir" (nr));
	return oldbit;
}

extern __inline__ int test_bit(int nr, const SMPVOL void * addr)
{
	return ((1UL << (nr & 31)) & (((const unsigned int *) addr)[nr >> 5])) != 0;
}

__END_DECLS
#endif
