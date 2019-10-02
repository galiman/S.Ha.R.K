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

/*	64 bit arithmetic	*/

#ifndef __LL_I386_64BIT__
#define __LL_I386_64BIT__

#define mul32div32to32(a,b,c,res)                                   \
     __asm__ __volatile__("mull %%ebx\n\t"                          \
			  "divl %%ecx\n\t"                          \
			  : "=a" ((res))                            \
			  : "a" ((a)), "b" ((b)), "c" ((c)), "d" (0))

#define smul32div32to32(a,b,c,res)                                   \
     __asm__ __volatile__("imull %%ebx\n\t"                          \
                          "idivl %%ecx\n\t"                          \
                          : "=a" ((res))                            \
                          : "a" ((a)), "b" ((b)), "c" ((c)), "d" (0))

#endif
