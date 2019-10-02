#ifndef __NOTIFIER__
#define __NOTIFIER__

#include <linux/compatib.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

struct notifier_block
{
	int (*notifier_call)(struct notifier_block *this, unsigned long, void *);
	struct notifier_block *next;
	int priority;
};

__END_DECLS

#endif
