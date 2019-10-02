#include <kernel/kern.h>
#include <stdlib.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#ifndef __UNIXCOMP__
#define __UNIXCOMP__

#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define LINUX_VERSION_CODE 0x20209

#define SA_NOCLDSTOP	1
#define SA_SHIRQ	0x04000000
#define SA_STACK	0x08000000
#define SA_RESTART	0x10000000
#define SA_INTERRUPT	0x20000000
#define SA_NOMASK	0x40000000
#define SA_ONESHOT	0x80000000

#define PAGE_SIZE 0x400

/* #define USE_SHARED_IRQ */

struct pt_regs {
	};		/* This have to be checked...	*/

#define __u32 unsigned long int
#define u32 unsigned long int
#define s32 signed long int
//#define u_long unsigned long int
#define __u16 unsigned short int
#define u16 unsigned short int
#define s16 signed short int
//#define u_short unsigned short int
#define ushort unsigned short int
//#define u_char unsigned char
#define u8 unsigned char
#define __u8 unsigned char
#define uint8_t unsigned char
#define uint16_t unsigned short int
#define uint32_t unsigned int
#define uint unsigned int
#define ulong unsigned long

#define caddr_t unsigned int

#define atomic_t int

#ifndef NULL
	#define NULL 0
#endif

#define HZ 100			/* Has to be controlled...	*/
#define jiffies (sys_gettime(NULL)*HZ/1000000)      	/* Has to be controlled...	*/
extern long unsigned int loops_per_sec; /* ... */
#define EISA_bus 0		/* We do not support EISA buses...	*/

#define NET_BH 1		/* ???? */

/* Linux Module stub emulation... */
#define MOD_INC_USE_COUNT 	/* Do nothing... */
#define MOD_DEC_USE_COUNT	/* Do nothing... */
#define MOD_IN_USE	0	/* No module => never in use... */
#define MODULE_AUTHOR(a)
#define MODULE_DESCRIPTION(a)
#define MODULE_LICENSE(a)
#define MODULE_PARM(a,b)
#define MODULE_PARM_DESC(a,b)

#define GFP_KERNEL	0x03	/* Don't know what it is... */
#define GFP_ATOMIC	0x01	/* Don't know what it is... */


/*	Linux kernel call emulation	*/
#define kmalloc(a,b) malloc(a)
//#define printk cprintf I would like to use the kernel printk if possible...
#define check_region(a,b) 0
#define request_region(a,b,c)
#define release_region(a,b)

/* Linux funcs emulation... */
#define outb_p(v,p) outp(p,v)
#define outb(v,p) outp(p,v)
#define outw(v,p) outpw(p,v)
#define outl(v,p) outpd(p,v)
#define inb_p(p) inp(p)
#define inb(p) inp(p)
#define inw(p) inpw(p)
#define inl(p) inpd(p)

#define mark_bh(NET_BH) 		/* Don't use soft int emulation... */


#define __cli() kern_cli()
#define __sti() kern_sti()
#define cli() kern_cli()
#define sti() kern_sti()
#define __save_flags(f) f = kern_fsave()
#define __restore_flags(f) kern_frestore(f)
#define save_flags(f) f = kern_fsave()
#define restore_flags(f) kern_frestore(f)


/* URKA Stubs */

extern void panic_stub(void);
/* #define eth_header panic_stub */
#define eth_rebuild_header panic_stub
#define eth_header_cache_bind panic_stub
#define eth_header_cache_update panic_stub


#define atomic_sub(a,b)


#define vremap(a,b) 0


extern __inline__ int suser(void)
{
  return 1;
}


// spinlocks
#define spinlock_t DWORD
#define spin_lock(x)   (void)(x)
#define spin_unlock(x) (void)(x)

#define spin_lock_irqsave(x,y) y = kern_fsave()
#define spin_unlock_irqrestore(x,y) kern_frestore(y)
#define spin_lock_init(x)
#define SPIN_LOCK_UNLOCKED (spinlock_t) 0
#define spin_lock_irq(lock)     kern_cli()
#define spin_unlock_irq(lock)     kern_sti()

extern __inline__ void panic(const char *c)
{
  cputs((char *)c);
  exit(1);
}

/* below tuff added for rtl8139 net driver 
   at some point this stuff should moved in a more conevenient place.
*/

/* *** from linux-2.2.17/include/linux/compatmac.h */  
#define capable(x)                   suser()

#define __devinitdata
#define __devinit

struct pci_device_id {
	unsigned int vendor, device;		/* Vendor and device ID or PCI_ANY_ID */
	unsigned int subvendor, subdevice;	/* Subsystem ID's or PCI_ANY_ID */
	unsigned int class, class_mask;		/* (class,subclass,prog-if) triplet */
	unsigned long driver_data;		/* Data private to the driver */
};
#define PCI_ANY_ID (~0)
#define MODULE_DEVICE_TABLE(a,b)

#ifdef CONFIG_HIGHMEM
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif
/* *** from linux-2.2.17/include/linux/capability.h */
#define CAP_NET_ADMIN        12

/* *** from linux-2.2.17/include/linux/byteorder/little_endian.h */
#define __le16_to_cpu(x) ((__u16)(x))

/* *** from linux-2.2.17/include/linux/byteorder/generic.h */
#define le16_to_cpu __le16_to_cpu

__END_DECLS
#endif
