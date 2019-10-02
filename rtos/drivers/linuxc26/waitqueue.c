#include <linuxcomp.h>
#include <linux/wait.h>
#include <linux/config.h>
#include <linux/list.h>

#include <kernel/types.h>

extern int task_activate(PID pid);
extern int exec_shadow;

struct task_struct {
	PID pid;
	int state;
	char comm[1024];
};

typedef struct task_struct task_t;

inline void init_waitqueue_head(wait_queue_head_t *q)
{
	q->lock = SPIN_LOCK_UNLOCKED;
	INIT_LIST_HEAD(&q->task_list);
}

static int try_to_wake_up(task_t * p, unsigned int state, int sync)
{
	task_activate(p->pid);
	return 0;
}

int default_wake_function(wait_queue_t *curr, unsigned mode, int sync)
{
	task_t *p = curr->task;
	return try_to_wake_up(p, mode, sync);
}

void __wake_up_common(wait_queue_head_t *q, unsigned int mode, int nr_exclusive, int sync)
{
	struct list_head *tmp, *next;

	list_for_each_safe(tmp, next, &q->task_list) {
		wait_queue_t *curr;
		unsigned flags;
		
		curr = list_entry(tmp, wait_queue_t, task_list);
		flags = curr->flags;
		curr->func(curr, mode, sync);
	}

}

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr_exclusive)
{
	unsigned long flags;

	spin_lock_irqsave(&q->lock, flags);
	__wake_up_common(q, mode, nr_exclusive, 0);
	spin_unlock_irqrestore(&q->lock, flags);
}

inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
	list_add(&new->task_list, &head->task_list);
}

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) void add_wait_queue(wait_queue_head_t *q, wait_queue_t * wait)
{
	unsigned long flags;

	wait->flags &= ~WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave(&q->lock, flags);
	__add_wait_queue(q, wait);
	spin_unlock_irqrestore(&q->lock, flags);
}

// added __attribute__ for gcc4 compatibility
__attribute__((regparm(3))) void remove_wait_queue(wait_queue_head_t *q, wait_queue_t * wait)
{
	unsigned long flags;

	spin_lock_irqsave(&q->lock, flags);
	__remove_wait_queue(q, wait);
	spin_unlock_irqrestore(&q->lock, flags);
}

inline int waitqueue_active(wait_queue_head_t *q)
{
	return !list_empty(&q->task_list);
}

inline void init_waitqueue_entry(wait_queue_t *q, struct task_struct *p)
{
	q->flags = 0;
	q->task = p;
	q->func = default_wake_function;
}

extern void * malloc(size_t size);

struct task_struct * get_current26(void)
{
	struct task_struct *tsp;
	
	tsp = malloc(sizeof(struct task_struct));
	if (!tsp)
	{
		printk(KERN_DEBUG "@get_current26 out of memory!!!\n");
		//
		// exit?
		//
	}
	tsp->pid = exec_shadow;

	//        printk(KERN_DEBUG "File: %s Pid=%d\n", __FILE__, tsp->pid);
	return tsp;
}
