#include <linuxcomp.h>

#include <asm/ptrace.h>
#include <asm-generic/errno-base.h>
#include <linux/kernel.h>

#define MAX_IRQS (NR_IRQS +1)

#ifndef NIL
#define NIL     -1                      /*+ integer unvalid value   +*/
#endif

//#define DEBUG_LINUX_INT

struct int_handler {
	void (*func)(int, void *dev_id, struct pt_regs *);
	void *data;
	int flags;
	struct int_handler *next;
};

static struct irq_handler_list {
	struct int_handler *handlers;
} irq_list[MAX_IRQS];

void init_linux_irq();
extern void fast_call_intr(int no);
extern void* malloc(int size);
extern void free(void *ptr);
extern int handler_set(int no, void (*fast)(int), int pi, BYTE lock);
extern int handler_remove(int no);

unsigned long intr_count = 0;
static int init = 0;

/*
* Generic Linux interrupt handler.
*/
void linux_intr(int irq)
{
	struct pt_regs regs;
	struct int_handler *ihp;

	intr_count++;

#ifdef DEBUG_LINUX_INT
	printk("(linux_intr %d)", irq);
#endif

	ihp=irq_list[irq].handlers;
	while (ihp) {
		(*ihp->func)(irq, ihp->data, &regs);
		ihp=ihp->next;
	}

	intr_count--;

}

void add_list(struct int_handler** headp, struct int_handler *ihp)
{
	if (*headp == NULL) {
		*headp=ihp;
		return;
	}
	ihp->next=*headp;
	*headp=ihp;
}

/*
* Attach a handler to an IRQ.
*/
int request_irq(unsigned int irq, void (*handler)(int, void *dev_id, struct pt_regs *), unsigned long flags, const char *device, void *dev_id)
{
	struct int_handler *ihp;

	if (init == 0)
		init_linux_irq();

	ihp=malloc(sizeof(struct int_handler));
	if (ihp == NULL)
		return -ENOMEM;

	if (irq_list[irq].handlers == NULL)
	{
		//* Warning: check if irq is used from somebody that doesn't share! (Claudio ?!?)
		shark_handler_set(irq, linux_intr);
		//shark_handler_set(irq, NULL, NULL);

#ifdef DEBUG_LINUX_INT
		printk("(request_irq %d)", irq);
#endif
	}
	ihp->func = handler;
	ihp->flags = flags;
	ihp->data = dev_id;
	ihp->next = NULL;
	add_list(&irq_list[irq].handlers, ihp);

	return 0;
}

/*
* Deallocate an irq
*/
void free_irq(unsigned int irq, void *dev_id)
{
	struct int_handler **headp, *ihp;

	headp=&irq_list[irq].handlers;
	while (*headp)
	{
		ihp=*headp;
		if (ihp->data == dev_id)
		{
			*headp=ihp->next;
			free(ihp);
			break;
		}
		headp=&ihp->next;
	}

	if (irq_list[irq].handlers == NULL)
	{
		shark_handler_remove(irq);

#ifdef DEBUG_LINUX_INT
		printk("(free_irq %d)", irq);
#endif
	}
}

void init_linux_irq()
{
	int i;

	for (i=0; i<MAX_IRQS; i++)
	{
		irq_list[i].handlers = NULL;
	}
	init=1;
}
