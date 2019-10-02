#ifndef __INTERRUPT__
#define __INTERRUPT__

#include <linux/compatib.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define NR_IRQS 128

int request_irq(unsigned int irq, void (*handler)(int, void *dev_id, struct pt_regs *), unsigned long flags, const char *device, void *dev_id);

#define	disable_irq_nosync(i) irq_mask(i)
#define disable_irq(i)  irq_mask(i)
#define enable_irq(i)   irq_unmask(i)

void free_irq(unsigned int irq, void *d);

__END_DECLS
#endif
