
#include <ll/i386/hw-instr.h>
#include <ll/i386/cons.h>
#include <ll/sys/ll/time.h>

#include <linuxcomp.h>

#include <linux/time.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/completion.h>
#include <linux/tty.h>
#include <asm/setup.h>

#define memory_barrier __asm__("" ::: "memory")

extern unsigned long intr_count;
extern int activeInt;

unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,                        /* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,         /* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,                        /* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,                        /* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,                    /* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,                        /* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,                        /* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,                        /* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,      /* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,                        /* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,                        /* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,                        /* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,      /* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,                        /* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,                        /* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,                        /* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                /* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                /* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};      /* 240-255 */

__kernel_size_t strnlen(const char *s, __kernel_size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
                /* nothing */;
	return sc - s;
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

struct resource ioport_resource = {
	.name   = "PCI IO",
	.start  = 0x0000,
	.end    = IO_SPACE_LIMIT,
	.flags  = IORESOURCE_IO,
};

struct resource iomem_resource = {
	.name   = "PCI mem",
	.start  = 0UL,
	.end    = ~0UL,
	.flags  = IORESOURCE_MEM,
};

/* Return the conflict entry if you can't request it */
static struct resource * __request_resource(struct resource *root, struct resource *new)
{
	unsigned long start = new->start;
	unsigned long end = new->end;
	struct resource *tmp, **p;

	if (end < start)
		return root;
	if (start < root->start)
		return root;
	if (end > root->end)
		return root;
	p = &root->child;
	for (;;) {
		tmp = *p;
		if (!tmp || tmp->start > end) {
			new->sibling = tmp;
			*p = new;
			new->parent = root;
			return NULL;
		}
		p = &tmp->sibling;
		if (tmp->end < start)
			continue;
		return tmp;
	}
}

static int __release_resource(struct resource *old)
{
	struct resource *tmp, **p;

	p = &old->parent->child;
	for (;;) {
		tmp = *p;
		if (!tmp)
			break;
		if (tmp == old) {
			*p = tmp->sibling;
			old->parent = NULL;
			return 0;
		}
		p = &tmp->sibling;
	}
	return -EINVAL;
}

int release_resource(struct resource *old)
{
	int retval;

	retval = __release_resource(old);
	
	return retval;
}

int request_resource(struct resource *root, struct resource *new)
{
	struct resource *conflict;
	
	conflict = __request_resource(root, new);

	return conflict ? -EBUSY : 0;
}



struct resource * __request_region(struct resource *parent, unsigned long start, unsigned long n, const char *name)
{
	return (void *)(0xFFFFFFFF);
}

void __release_region(struct resource *parent, unsigned long start, unsigned long n)
{

}

static int find_resource(struct resource *root, struct resource *new,
			 unsigned long size,
			 unsigned long min, unsigned long max,
			 unsigned long align,
			 void (*alignf)(void *, struct resource *,
					unsigned long, unsigned long),
			 void *alignf_data)
{
	struct resource *this = root->child;

	new->start = root->start;
	/*
	 * Skip past an allocated resource that starts at 0, since the assignment
	 * of this->start - 1 to new->end below would cause an underflow.
	 */
	if (this && this->start == 0) {
		new->start = this->end + 1;
		this = this->sibling;
	}
	for(;;) {
		if (this)
			new->end = this->start - 1;
		else
			new->end = root->end;
		if (new->start < min)
			new->start = min;
		if (new->end > max)
			new->end = max;
		new->start = (new->start + align - 1) & ~(align - 1);
		if (alignf)
			alignf(alignf_data, new, size, align);
		if (new->start < new->end && new->end - new->start + 1 >= size) {
			new->end = new->start + size - 1;
			return 0;
		}
		if (!this)
			break;
		new->start = this->end + 1;
		this = this->sibling;
	}
	return -EBUSY;
}

int allocate_resource(struct resource *root, struct resource *new,
		      unsigned long size,
		      unsigned long min, unsigned long max,
		      unsigned long align,
		      void (*alignf)(void *, struct resource *,
				     unsigned long, unsigned long),
		      void *alignf_data)
{
	int err;

	err = find_resource(root, new, size, min, max, align, alignf, alignf_data);
	if (err >= 0 && __request_resource(root, new))
		err = -EBUSY;
	
	return err;
}

int remap_page_range(struct vm_area_struct *vma, unsigned long from, unsigned long phys_addr, unsigned long size, pgprot_t prot)
{
	return 0;
}

void dump_stack(void)
{

}

void panic(const char * fmt, ...)
{
	cprintf((char *)(fmt));
}

extern void * malloc(size_t size);

void *__kmalloc(size_t size, int flags)
{
	return malloc(size);
}

extern void free(void *);

void kfree(const void *ptr)
{
	free((void *)(ptr));
}

unsigned long pci_mem_start = 0x10000000;

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) signed long schedule_timeout(signed long timeout)
{
	struct timespec t,s,e;

	jiffies_to_timespec(timeout, &t);

	if (!activeInt && !intr_count) {
		nanosleep(&t,NULL);
	} else {
		ll_gettime(TIME_NEW,&s);
		ADDTIMESPEC(&t,&s,&e);

		memory_barrier;

		while(TIMESPEC_A_LT_B(&s,&e)) {
			memory_barrier;
			ll_gettime(TIME_NEW,&s);
		}
	}

	return 0;
}

void __const_udelay(unsigned long usecs)
{
	struct timespec t,s,e;

	if (!activeInt && !intr_count) {
		t.tv_sec = 0;
		t.tv_nsec = usecs * 1000;

		nanosleep(&t,NULL);
	} else {
		ll_gettime(TIME_NEW,&e);
		ADDUSEC2TIMESPEC(usecs,&e);

		memory_barrier;

		ll_gettime(TIME_NEW,&s);
		while(TIMESPEC_A_LT_B(&s,&e)) {
			memory_barrier;
			ll_gettime(TIME_NEW,&s);
		}
	}
}

void * vmalloc_32(size_t size);

void *dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, int gfp)
{
	void *ret;
	/* ignore region specifiers */
	gfp &= ~(__GFP_DMA | __GFP_HIGHMEM);

	if (dev == NULL || (*dev->dma_mask < 0xffffffff))
		gfp |= GFP_DMA;
	ret = (void *)vmalloc_32(size);

	if (ret != NULL) {
		memset(ret, 0, size);
		*dma_handle = (dma_addr_t)ret;
	}
	return ret;
}

void *dma_alloc_coherent_usb(struct device *dev, size_t size, dma_addr_t *dma_handle, int gfp)
{
	void *ret;
	/* ignore region specifiers */
	gfp &= ~(__GFP_DMA | __GFP_HIGHMEM);

	if (dev == NULL || (*dev->dma_mask < 0xffffffff))
		gfp |= GFP_DMA;
	ret = (void *)vmalloc_32_usb(size);

	if (ret != NULL) {
		memset(ret, 0, size);
		*dma_handle = (dma_addr_t)ret;
	}
	return ret;
}

void dma_free_coherent(struct device *dev, size_t size, void *vaddr, dma_addr_t dma_handle)
{
        vfree((void *)dma_handle);
}

extern void __wake_up_common(wait_queue_head_t *q, unsigned int mode, int nr_exclusive, int sync);

void init_completion(struct completion *x) {
	x->done = 0;
	init_waitqueue_head(&x->wait);
}

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) void complete(struct completion *x) 
{
	x->done++;
	__wake_up_common(&x->wait, TASK_UNINTERRUPTIBLE | TASK_INTERRUPTIBLE, 1, 0);
}

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) void wait_for_completion(struct completion *x)
{
	spin_lock_irq(&x->wait.lock);
	if (!x->done) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&x->wait, &wait);
		do {
			__set_current_state(TASK_UNINTERRUPTIBLE);
			spin_unlock_irq(&x->wait.lock);
			schedule();
			spin_lock_irq(&x->wait.lock);
		} while (!x->done);
		__remove_wait_queue(&x->wait, &wait);
	}
	x->done--;
	spin_unlock_irq(&x->wait.lock);
}

struct device legacy_bus = {
        .bus_id         = "legacy",
};

int register_chrdev(unsigned int a, const char *b, struct file_operations *c)
{
	return 0;
}

int unregister_chrdev(unsigned int a, const char *b)
{
	return 0;
}

void * __ioremap(unsigned long offset, unsigned long size, unsigned long flags)
{
	return (void *)offset;
}

void iounmap(void *addr)
{

}

loff_t no_llseek(struct file *file, loff_t offset, int origin)
{
	return 0;
}

void *vmalloc(unsigned long size)
{
	return malloc(size);
}

void *kern_alloc_aligned(size_t size, DWORD flags, int align_bits, DWORD align_ofs);

void * vmalloc_32(size_t size)
{
	void *mem;
	unsigned long diff;

	mem = malloc(size+12);

	diff = (unsigned long)((((unsigned long)mem/4)+1)*4-(unsigned long)mem);

	*(unsigned long *)(mem+diff) = (diff | 0x80000000);

	return (mem+diff+4);
}

void vfree(void *addr)
{
	if (addr == NULL || *(unsigned long *)(addr-4) == 0)
		return;

	if ((*(unsigned long *)(addr-4) & 0x80000000) == 0x80000000) {
		free(addr-(*(unsigned long *)(addr-4) & 0x7FFFFFFF)-4);
		*(unsigned long *)(addr-4) = 0;
		return;
	}

	free(addr);

	return;
}

void * vmalloc_32_usb(size_t size)
{
	void* mem;
	unsigned long diff;

	mem = malloc( size + 2 * 4096 );

	if (! mem)
		return NULL;

	diff = 4096 - (((unsigned long) mem) % 4096);
	*(unsigned long *)(mem+diff) = (diff | 0x80000000);

	return (void*)(mem + diff + 4096);
}

/* TODO */
char * strsep(char **a,const char *b)
{
	return NULL;
}

struct screen_info screen_info;

int linuxcomp_setfd(struct inode *i, int i_rdev)
{
	i->i_rdev = i_rdev;

	return 0;
}

int linuxcomp_init(void)
{
  return 0;
}

struct page *mem_map = 0x0000;

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) int schedule_work(struct work_struct *work)
{
	return 0;
} 

int allow_signal(int sig)
{
	return 0;
}

void flush_scheduled_work(void) { } 
void daemonize(const char *name, ...) { } 
void yield(void) { }

void do_exit(long code) { } 

void complete_and_exit(struct completion *comp, long code)
{
	if (comp)
		complete(comp);
	
	do_exit(code);
}

inline void * ioremap_nocache(unsigned long offset, unsigned long size)
{
         return __ioremap(offset, size, 0);
}

#define NULL_TIMESPEC(t)	((t)->tv_sec = (t)->tv_nsec = 0)

int wait_ms26(unsigned long msec)
{
	struct timespec t1, t2;
	int nsec;

	t1.tv_sec = msec/1000;
	nsec = (msec % 1000) * 1000000;
	t1.tv_nsec = nsec;
	NULL_TIMESPEC(&t2);
	nanosleep(&t1, &t2);

	return 0;
}
