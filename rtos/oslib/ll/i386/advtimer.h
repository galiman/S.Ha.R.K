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

/*	Advanced Timer
 *	Date: 8.4.2003
 * 	Author: Giacomo Guidi <giacomo@gandalf.sssup.it>
 *
 */

#ifndef __ADVTIMER_H__
#define __ADVTIMER_H__

#include <ll/i386/defs.h>
BEGIN_DEF

#include <ll/sys/ll/time.h>

/* TSC */

#define rdtsc(low,high) \
	__asm__ __volatile__("xorl %%eax,%%eax\n\t" \
			     "cpuid\n\t"            \
			     "rdtsc\n\t"            \
			     : "=a" (low), "=d" (high) \
			     :: "ebx", "ecx")

#define rdtscll(val) \
	__asm__ __volatile__("xorl %%eax,%%eax\n\t" \
			     "cpuid\n\t"            \
			     "rdtsc\n\t"            \
			     : "=A" (val)           \
			     :: "ebx","ecx")

#ifdef __O1000__
	#define ll_read_timespec ll_read_timespec_1000
#else 
	#ifdef __02000__
		#define ll_read_timespec ll_read_timespec_2000
	#else
		#ifdef __O4000__
			#define ll_read_timespec ll_read_timespec_4000
		#else
			#define ll_read_timespec ll_read_timespec_8000
		#endif
	#endif
#endif

//Low level time read function: Optimized for CPU < 1 GHz
extern __inline__ void ll_read_timespec_1000(struct timespec *tspec)
{
	extern unsigned int clk_opt_1,clk_opt_2;
	extern unsigned long long *ptr_init_tsc;
	extern struct timespec init_time;
                 
	if (clk_opt_1 == 0) {
		NULL_TIMESPEC(tspec);
		return;
	}
                                                                                                            
	__asm__("rdtsc\n\t"
		"subl (%%edi),%%eax\n\t"
		"sbbl 4(%%edi),%%edx\n\t"
		"divl %%ebx\n\t"
		"movl %%eax,%%ebx\n\t"
		"xorl %%eax,%%eax\n\t"
		"divl %%ecx\n\t"
		: "=a" (tspec->tv_nsec), "=b" (tspec->tv_sec)
		: "D" (ptr_init_tsc) , "b" (clk_opt_1), "c" (clk_opt_2)
		: "edx" );
	
	if (init_time.tv_sec != 0 || init_time.tv_nsec != 0) {
		__asm__("divl %%ecx\n\t"
			"addl %%ebx,%%eax\n\t"
			:"=a" (tspec->tv_sec), "=d" (tspec->tv_nsec)
			:"a" (init_time.tv_nsec+tspec->tv_nsec), "b" (tspec->tv_sec+init_time.tv_sec), "c" (1000000000), "d" (0));
	};
	            
}

//Low level time read function: Optimized for CPU < 2 GHz
extern __inline__ void ll_read_timespec_2000(struct timespec *tspec)
{
	extern unsigned int clk_opt_1,clk_opt_3;
	extern unsigned long long *ptr_init_tsc;
	extern struct timespec init_time;
                
	if (clk_opt_1 == 0) {
		NULL_TIMESPEC(tspec);
		return;
	}
                                                                                                             
	__asm__("rdtsc\n\t"
		"subl (%%edi),%%eax\n\t"
		"sbbl 4(%%edi),%%edx\n\t"
		"divl %%ebx\n\t"
		"movl %%eax,%%ebx\n\t"
		"xorl %%eax,%%eax\n\t"
		"shrdl $1,%%edx,%%eax\n\t"
		"shrl %%edx\n\t"
		"divl %%ecx\n\t"
		: "=a" (tspec->tv_nsec), "=b" (tspec->tv_sec)
		: "D" (ptr_init_tsc) , "b" (clk_opt_1), "c" (clk_opt_3)
		: "edx" );
		
	if (init_time.tv_sec != 0 || init_time.tv_nsec != 0) {
		__asm__("divl %%ecx\n\t"
			"addl %%ebx,%%eax\n\t"
			:"=a" (tspec->tv_sec), "=d" (tspec->tv_nsec)
			:"a" (init_time.tv_nsec+tspec->tv_nsec), "b" (tspec->tv_sec+init_time.tv_sec), "c" (1000000000), "d" (0));
	};
	
}

//Low level time read function: Optimized for CPU < 4 GHz
extern __inline__ void ll_read_timespec_4000(struct timespec *tspec)
{
	extern unsigned int clk_opt_1,clk_opt_4;
	extern unsigned long long *ptr_init_tsc;
	extern struct timespec init_time;
            
	if (clk_opt_1 == 0) {
		NULL_TIMESPEC(tspec);
		return;
	}
                                                                                                                 
	__asm__("rdtsc\n\t"
		"subl (%%edi),%%eax\n\t"
		"sbbl 4(%%edi),%%edx\n\t"
		"divl %%ebx\n\t"
		"movl %%eax,%%ebx\n\t"
		"xorl %%eax,%%eax\n\t"
		"shrdl $2,%%edx,%%eax\n\t"
		"shrl $2,%%edx\n\t"
		"divl %%ecx\n\t"
		: "=a" (tspec->tv_nsec), "=b" (tspec->tv_sec)
		: "D" (ptr_init_tsc) , "b" (clk_opt_1), "c" (clk_opt_4)
		: "edx" );
		
	if (init_time.tv_sec != 0 || init_time.tv_nsec != 0) {
		__asm__("divl %%ecx\n\t"
			"addl %%ebx,%%eax\n\t"
			:"=a" (tspec->tv_sec), "=d" (tspec->tv_nsec)
			:"a" (init_time.tv_nsec+tspec->tv_nsec), "b" (tspec->tv_sec+init_time.tv_sec), "c" (1000000000), "d" (0));
	};
	
}

//Low level time read function
extern __inline__ void ll_read_timespec_8000(struct timespec *tspec)
{
	extern unsigned int clk_opt_0,clk_opt_5;
	extern unsigned long long *ptr_init_tsc;
	extern struct timespec init_time;
	
	if (clk_opt_0 == 0) {
		NULL_TIMESPEC(tspec);
		return;
	}

	__asm__("rdtsc\n\t"
		"subl (%%edi),%%eax\n\t"
		"sbbl 4(%%edi),%%edx\n\t"
		"shrdl $1,%%edx,%%eax\n\t"
		"shrl %%edx\n\t"
		"divl %%ebx\n\t"
		"movl %%eax,%%ebx\n\t"
		"xorl %%eax,%%eax\n\t"
		"shrdl $2,%%edx,%%eax\n\t"
		"shrl $2,%%edx\n\t"
		"divl %%ecx\n\t"
		: "=b" (tspec->tv_sec), "=a" (tspec->tv_nsec)
		: "D" (ptr_init_tsc), "b" (clk_opt_0), "c" (clk_opt_5)
		: "edx");
    
	if (init_time.tv_sec != 0 || init_time.tv_nsec != 0) {
		__asm__("divl %%ecx\n\t"
			"addl %%ebx,%%eax\n\t"
			:"=a" (tspec->tv_sec), "=d" (tspec->tv_nsec)
			:"a" (init_time.tv_nsec+tspec->tv_nsec), "b" (tspec->tv_sec+init_time.tv_sec), "c" (1000000000), "d" (0));
	};
	
}

#define rdmsr(msr,val1,val2) \
	__asm__ __volatile__("rdmsr" \
			     : "=a" (val1), "=d" (val2) \
			     : "c" (msr))
                                                                                                                             
#define wrmsr(msr,val1,val2) \
	__asm__ __volatile__("wrmsr" \
			     : /* no outputs */ \
			     : "c" (msr), "a" (val1), "d" (val2))

/* RTC */

#define RTC_PORT(x)	(0x70 + (x))

#define CMOS_READ(addr,val)	\
{				\
outp(RTC_PORT(0),(addr)); 	\
val = inp(RTC_PORT(1));	 	\
}

#define CMOS_WRITE(addr,val) 	\
{ 				\
outp(RTC_PORT(0),(addr)); 	\
outp(RTC_PORT(1),(val)); 	\
}

#define RTC_IRQ 8

void ll_init_advtimer(void);
void ll_restore_adv(void);
void ll_scale_advtimer(unsigned int old_f, unsigned int new_f);

END_DEF
#endif
