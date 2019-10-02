#ifndef _I386_CURRENT_H
#define _I386_CURRENT_H

#include <linux/thread_info.h>

struct task_struct;
extern struct task_struct * get_current26(void);

static inline struct task_struct * get_current(void)
{
//        printk(KERN_DEBUG "File: %s @get_current\n", __FILE__);
        return get_current26();
}
 
#define current get_current()

#endif /* !(_I386_CURRENT_H) */
