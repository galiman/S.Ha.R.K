#ifndef __LINUX_COMP__
#define __LINUX_COMP__

#ifndef __i386__
#define __i386__
#endif

#define CONFIG_M386
#define CONFIG_X86

#define va_list void*

#define CONFIG_X86_L1_CACHE_SHIFT 6

#define CONFIG_X86_CMPXCHG
#define CONFIG_PCI

#define NR_IRQS 15
#define NR_IRQ_VECTORS 15

#define APIC_DEFINITION
#define __BIT_TYPES_DEFINED__

#include <ll/i386/mem.h>

#define _SIZE_T
#define _SSIZE_T

#define __HAVE_ARCH_MEMSET
#define __HAVE_ARCH_MEMCPY
#define __HAVE_ARCH_MEMMOVE
#define __HAVE_ARCH_MEMSCAN
#define __HAVE_ARCH_MEMCMP
#define __HAVE_ARCH_MEMCHR

void shark_internal_sem_create(void **sem, int init);
void shark_internal_sem_wait(void *sem);
void shark_internal_sem_post(void *sem);


#define CONFIG_GAMEPORT
#define CONFIG_CPU_FREQ_TABLE

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
	long	tv_sec;         /* Seconds */
	long	tv_nsec;        /* Nanoseconds */
};
#endif /* _STRUCT_TIMESPEC */

#define __udelay __const_udelay

extern TIME sys_gettime(struct timespec *t);

#define MAX_TIMER_TABLE 64 /* Shark_glue table for interrupt and timers */

unsigned long long read_jiffies(void);

#define jiffies26 ((unsigned long long)(read_jiffies()))

/* Timer Manager */
int shark_timer_set(const struct timespec *time, void *handler, void *arg);
int shark_timer_delete(int index);

/* Interrupt handler installation and removal */
int shark_handler_set(int no, void *fast);
int shark_handler_remove(int no);

#endif
