#include <linuxcomp.h>

#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/module.h>

/*
 * Pool allocator ... wraps the pci_alloc_consistent page allocator, so
 * small blocks are easily used by drivers for bus mastering controllers.
 * This should probably be sharing the guts of the slab allocator.
 */

struct pci_pool {	/* the pool */
	struct list_head	page_list;
	spinlock_t		lock;
	size_t			blocks_per_page;
	size_t			size;
	struct pci_dev		*dev;
	size_t			allocation;
	char			name [32];
	wait_queue_head_t	waitq;
	struct list_head	pools;
};

struct pci_page {	/* cacheable header for 'allocation' bytes */
	struct list_head	page_list;
	void			*vaddr;
	dma_addr_t		dma;
	unsigned		in_use;
	unsigned long		bitmap [0];
};

#define	POOL_TIMEOUT_JIFFIES	((100 /* msec */ * HZ) / 1000)
#define	POOL_POISON_FREED	0xa7	/* !inuse */
#define	POOL_POISON_ALLOCATED	0xa9	/* !initted */

static DECLARE_MUTEX (pools_lock);

static ssize_t
show_pools (struct device *dev, char *buf)
{
	struct pci_dev		*pdev;
	unsigned		temp, size;
	char			*next;
	struct list_head	*i, *j;

	pdev = container_of (dev, struct pci_dev, dev);
	next = buf;
	size = PAGE_SIZE;

	temp = snprintf26(next, size, "poolinfo - 0.1\n");
	size -= temp;
	next += temp;

	down (&pools_lock);
	list_for_each (i, &pdev->pools) {
		struct pci_pool	*pool;
		unsigned	pages = 0, blocks = 0;

		pool = list_entry (i, struct pci_pool, pools);

		list_for_each (j, &pool->page_list) {
			struct pci_page	*page;

			page = list_entry (j, struct pci_page, page_list);
			pages++;
			blocks += page->in_use;
		}

		/* per-pool info, no real statistics yet */
		temp = snprintf26(next, size, "%-16s %4u %4Zu %4Zu %2u\n",
				pool->name,
				blocks, (unsigned int)(pages * pool->blocks_per_page),
				(unsigned int)(pool->size), pages);
		size -= temp;
		next += temp;
	}
	up (&pools_lock);

	return PAGE_SIZE - size;
}
static DEVICE_ATTR (pools, S_IRUGO, show_pools, NULL);

/**
 * pci_pool_create - Creates a pool of pci consistent memory blocks, for dma.
 * @name: name of pool, for diagnostics
 * @pdev: pci device that will be doing the DMA
 * @size: size of the blocks in this pool.
 * @align: alignment requirement for blocks; must be a power of two
 * @allocation: returned blocks won't cross this boundary (or zero)
 * Context: !in_interrupt()
 *
 * Returns a pci allocation pool with the requested characteristics, or
 * null if one can't be created.  Given one of these pools, pci_pool_alloc()
 * may be used to allocate memory.  Such memory will all have "consistent"
 * DMA mappings, accessible by the device and its driver without using
 * cache flushing primitives.  The actual size of blocks allocated may be
 * larger than requested because of alignment.
 *
 * If allocation is nonzero, objects returned from pci_pool_alloc() won't
 * cross that size boundary.  This is useful for devices which have
 * addressing restrictions on individual DMA transfers, such as not crossing
 * boundaries of 4KBytes.
 */
struct pci_pool *
pci_pool_create (const char *name, struct pci_dev *pdev,
	size_t size, size_t align, size_t allocation)
{
	struct pci_pool		*retval;

	if (align == 0)
		align = 1;
	if (size == 0)
		return 0;
	else if (size < align)
		size = align;
	else if ((size % align) != 0) {
		size += align + 1;
		size &= ~(align - 1);
	}

	if (allocation == 0) {
		if (PAGE_SIZE < size)
			allocation = size;
		else
			allocation = PAGE_SIZE;
		// FIXME: round up for less fragmentation
	} else if (allocation < size)
		return 0;

	if (!(retval = kmalloc (sizeof *retval, SLAB_KERNEL)))
		return retval;

	strlcpy (retval->name, name, sizeof retval->name);

	retval->dev = pdev;

	INIT_LIST_HEAD (&retval->page_list);
	spin_lock_init (&retval->lock);
	retval->size = size;
	retval->allocation = allocation;
	retval->blocks_per_page = allocation / size;
	init_waitqueue_head (&retval->waitq);

	if (pdev) {
		down (&pools_lock);
		if (list_empty (&pdev->pools))
			device_create_file (&pdev->dev, &dev_attr_pools);
		/* note:  not currently insisting "name" be unique */
		list_add (&retval->pools, &pdev->pools);
		up (&pools_lock);
	} else
		INIT_LIST_HEAD (&retval->pools);

	return retval;
}


static struct pci_page *
pool_alloc_page (struct pci_pool *pool, int mem_flags)
{
	struct pci_page	*page;
	int		mapsize;

	mapsize = pool->blocks_per_page;
	mapsize = (mapsize + BITS_PER_LONG - 1) / BITS_PER_LONG;
	mapsize *= sizeof (long);

	page = (struct pci_page *) kmalloc (mapsize + sizeof *page, mem_flags);
	if (!page)
		return 0;
	page->vaddr = pci_alloc_consistent (pool->dev,
					    pool->allocation,
					    &page->dma);
	if (page->vaddr) {
		memset (page->bitmap, 0xff, mapsize);	// bit set == free
#ifdef	CONFIG_DEBUG_SLAB
		memset (page->vaddr, POOL_POISON_FREED, pool->allocation);
#endif
		list_add (&page->page_list, &pool->page_list);
		page->in_use = 0;
	} else {
		kfree (page);
		page = 0;
	}
	return page;
}

static struct pci_page *
pool_alloc_page_usb (struct pci_pool *pool, int mem_flags)
{
	struct pci_page *page;
	int             mapsize;

	mapsize = pool->blocks_per_page;
	mapsize = (mapsize + BITS_PER_LONG - 1) / BITS_PER_LONG;
	mapsize *= sizeof (long);

	page = (struct pci_page *) kmalloc (mapsize + sizeof *page, mem_flags);
	if (!page)
		return 0;
	page->vaddr = pci_alloc_consistent_usb (pool->dev, pool->allocation, &page->dma);
	if (page->vaddr) {
		memset (page->bitmap, 0xff, mapsize);   // bit set == free
#ifdef  CONFIG_DEBUG_SLAB
		memset (page->vaddr, POOL_POISON_FREED, pool->allocation);
#endif
		list_add (&page->page_list, &pool->page_list);
		page->in_use = 0;
	} else {
		kfree (page);
		page = 0;
	}
	return page;
}

static inline int
is_page_busy (int blocks, unsigned long *bitmap)
{
	while (blocks > 0) {
		if (*bitmap++ != ~0UL)
			return 1;
		blocks -= BITS_PER_LONG;
	}
	return 0;
}

static void
pool_free_page (struct pci_pool *pool, struct pci_page *page)
{
	dma_addr_t	dma = page->dma;

#ifdef	CONFIG_DEBUG_SLAB
	memset (page->vaddr, POOL_POISON_FREED, pool->allocation);
#endif
	pci_free_consistent (pool->dev, pool->allocation, page->vaddr, dma);
	list_del (&page->page_list);
	kfree (page);
}

/**
 * pci_pool_destroy - destroys a pool of pci memory blocks.
 * @pool: pci pool that will be destroyed
 * Context: !in_interrupt()
 *
 * Caller guarantees that no more memory from the pool is in use,
 * and that nothing will try to use the pool after this call.
 */
void
pci_pool_destroy (struct pci_pool *pool)
{
	down (&pools_lock);
	list_del (&pool->pools);
	if (pool->dev && list_empty (&pool->dev->pools))
		device_remove_file (&pool->dev->dev, &dev_attr_pools);
	up (&pools_lock);

	while (!list_empty (&pool->page_list)) {
		struct pci_page		*page;
		page = list_entry (pool->page_list.next,
				struct pci_page, page_list);
		if (is_page_busy (pool->blocks_per_page, page->bitmap)) {
			printk (KERN_ERR "pci_pool_destroy %s/%s, %p busy\n",
				pool->dev ? pci_name(pool->dev) : NULL,
				pool->name, page->vaddr);
			/* leak the still-in-use consistent memory */
			list_del (&page->page_list);
			kfree (page);
		} else
			pool_free_page (pool, page);
	}

	kfree (pool);
}

/**
 * pci_pool_alloc - get a block of consistent memory
 * @pool: pci pool that will produce the block
 * @mem_flags: SLAB_KERNEL or SLAB_ATOMIC
 * @handle: pointer to dma address of block
 *
 * This returns the kernel virtual address of a currently unused block,
 * and reports its dma address through the handle.
 * If such a memory block can't be allocated, null is returned.
 */
void *
pci_pool_alloc (struct pci_pool *pool, int mem_flags, dma_addr_t *handle)
{
	unsigned long		flags;
	struct list_head	*entry;
	struct pci_page		*page;
	int			map, block;
	size_t			offset;
	void			*retval;

restart:
	spin_lock_irqsave (&pool->lock, flags);
	list_for_each (entry, &pool->page_list) {
		int		i;
		page = list_entry (entry, struct pci_page, page_list);
		/* only cachable accesses here ... */
		for (map = 0, i = 0;
				i < pool->blocks_per_page;
				i += BITS_PER_LONG, map++) {
			if (page->bitmap [map] == 0)
				continue;
			block = ffz (~ page->bitmap [map]);
			if ((i + block) < pool->blocks_per_page) {
				clear_bit (block, &page->bitmap [map]);
				offset = (BITS_PER_LONG * map) + block;
				offset *= pool->size;
				goto ready;
			}
		}
	}
	if (!(page = pool_alloc_page (pool, SLAB_ATOMIC))) {
		if (mem_flags == SLAB_KERNEL) {
			DECLARE_WAITQUEUE (wait, current);

			//current->state = TASK_INTERRUPTIBLE;
			add_wait_queue (&pool->waitq, &wait);
			spin_unlock_irqrestore (&pool->lock, flags);

			schedule_timeout (POOL_TIMEOUT_JIFFIES);

			remove_wait_queue (&pool->waitq, &wait);
			goto restart;
		}
		retval = 0;
		goto done;
	}

	clear_bit (0, &page->bitmap [0]);
	offset = 0;
ready:
	page->in_use++;
	retval = offset + page->vaddr;
	*handle = offset + page->dma;
#ifdef	CONFIG_DEBUG_SLAB
	memset (retval, POOL_POISON_ALLOCATED, pool->size);
#endif
done:
	spin_unlock_irqrestore (&pool->lock, flags);
	return retval;
}

/**
 * pci_pool_alloc_usb - get a block of consistent memory
 * @pool: pci pool that will produce the block
 * @mem_flags: SLAB_KERNEL or SLAB_ATOMIC
 * @handle: pointer to dma address of block
 *
 * This returns the kernel virtual address of a currently unused block,
 * and reports its dma address through the handle.
 * If such a memory block can't be allocated, null is returned.
 */
void *
pci_pool_alloc_usb (struct pci_pool *pool, int mem_flags, dma_addr_t *handle)
{
        unsigned long           flags;
        struct list_head        *entry;
        struct pci_page         *page;
        int                     map, block;
        size_t                  offset;
        void                    *retval;

restart:
        spin_lock_irqsave (&pool->lock, flags);
        list_for_each (entry, &pool->page_list) {
                int             i;
                page = list_entry (entry, struct pci_page, page_list);
                /* only cachable accesses here ... */
                for (map = 0, i = 0;
                                i < pool->blocks_per_page;
                                i += BITS_PER_LONG, map++) {
                        if (page->bitmap [map] == 0)
                                continue;
                        block = ffz (~ page->bitmap [map]);
                        if ((i + block) < pool->blocks_per_page) {
                                clear_bit (block, &page->bitmap [map]);
                                offset = (BITS_PER_LONG * map) + block;
                                offset *= pool->size;
                                goto ready;
                        }
                }
        }
        if (!(page = pool_alloc_page_usb (pool, SLAB_ATOMIC))) {
                if (mem_flags == SLAB_KERNEL) {
                        DECLARE_WAITQUEUE (wait, current);

                        //current->state = TASK_INTERRUPTIBLE;
                        add_wait_queue (&pool->waitq, &wait);
                        spin_unlock_irqrestore (&pool->lock, flags);

                        schedule_timeout (POOL_TIMEOUT_JIFFIES);

                        remove_wait_queue (&pool->waitq, &wait);
                        goto restart;
                }
                retval = 0;
                goto done;
        }

        clear_bit (0, &page->bitmap [0]);
        offset = 0;
ready:
        page->in_use++;
        retval = offset + page->vaddr;
        *handle = offset + page->dma;
#ifdef  CONFIG_DEBUG_SLAB
        memset (retval, POOL_POISON_ALLOCATED, pool->size);
#endif
done:
        spin_unlock_irqrestore (&pool->lock, flags);
        return retval;
}

static struct pci_page *
pool_find_page (struct pci_pool *pool, dma_addr_t dma)
{
	unsigned long		flags;
	struct list_head	*entry;
	struct pci_page		*page;

	spin_lock_irqsave (&pool->lock, flags);
	list_for_each (entry, &pool->page_list) {
		page = list_entry (entry, struct pci_page, page_list);
		if (dma < page->dma)
			continue;
		if (dma < (page->dma + pool->allocation))
			goto done;
	}
	page = 0;
done:
	spin_unlock_irqrestore (&pool->lock, flags);
	return page;
}

/**
 * pci_pool_free - put block back into pci pool
 * @pool: the pci pool holding the block
 * @vaddr: virtual address of block
 * @dma: dma address of block
 *
 * Caller promises neither device nor driver will again touch this block
 * unless it is first re-allocated.
 */
void
pci_pool_free (struct pci_pool *pool, void *vaddr, dma_addr_t dma)
{
	struct pci_page		*page;
	unsigned long		flags;
	int			map, block;

	if ((page = pool_find_page (pool, dma)) == 0) {
		printk (KERN_ERR "pci_pool_free %s/%s, %p/%lx (bad dma)\n",
			pool->dev ? pci_name(pool->dev) : NULL,
			pool->name, vaddr, (unsigned long) dma);
		return;
	}

	block = dma - page->dma;
	block /= pool->size;
	map = block / BITS_PER_LONG;
	block %= BITS_PER_LONG;

#ifdef	CONFIG_DEBUG_SLAB
	if (((dma - page->dma) + (void *)page->vaddr) != vaddr) {
		printk (KERN_ERR "pci_pool_free %s/%s, %p (bad vaddr)/%Lx\n",
			pool->dev ? pci_name(pool->dev) : NULL,
			pool->name, vaddr, (unsigned long long) dma);
		return;
	}
	if (page->bitmap [map] & (1UL << block)) {
		printk (KERN_ERR "pci_pool_free %s/%s, dma %Lx already free\n",
			pool->dev ? pci_name(pool->dev) : NULL,
			pool->name, (unsigned long long)dma);
		return;
	}
	memset (vaddr, POOL_POISON_FREED, pool->size);
#endif

	spin_lock_irqsave (&pool->lock, flags);
	page->in_use--;
	set_bit (block, &page->bitmap [map]);
	if (waitqueue_active (&pool->waitq))
		wake_up (&pool->waitq);
	/*
	 * Resist a temptation to do
	 *    if (!is_page_busy(bpp, page->bitmap)) pool_free_page(pool, page);
	 * it is not interrupt safe. Better have empty pages hang around.
	 */
	spin_unlock_irqrestore (&pool->lock, flags);
}

EXPORT_SYMBOL (pci_pool_create);
EXPORT_SYMBOL (pci_pool_destroy);
EXPORT_SYMBOL (pci_pool_alloc);
EXPORT_SYMBOL (pci_pool_free);
