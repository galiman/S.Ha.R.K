/*
 * Linux timer support.
 */

#include <linuxcomp.h>

#include <linux/kernel.h>
#include <linux/timer.h>
#include <asm/param.h>

//#define TIMER_DEBUG

unsigned long long read_jiffies()
{
	struct timespec acttime;
	unsigned long long temp;	
	
	sys_gettime(&acttime);
	temp = (unsigned long long)(acttime.tv_sec) * (unsigned long long)(HZ);
	temp += (unsigned long long)(acttime.tv_nsec) * (unsigned long long)(HZ) / 1000000000L;

	return temp;

}

static inline void jiffies_to_timespec(unsigned long j, struct timespec *value)
{
	value->tv_nsec = (j % HZ) * (1000000000L / HZ);
	value->tv_sec = j / HZ;
}

/*extern void *timer_func_table[MAX_TIMER_TABLE];
extern void *timer_arg_table[MAX_TIMER_TABLE];
extern int intr_count;
void linux_timer(int no)
{
        intr_count++;
                                                                                                                             
        if (timer_func_table[no] != NULL)
                (*(void (*)(void *arg))timer_func_table[no])(timer_arg_table[no]);
                                                                                                                             
        intr_count--;
}*/

/*
 * Generic Linux time handler.
 */
void add_timer(struct timer_list *timer)
{
	struct timespec timeout;

	jiffies_to_timespec(timer->expires, &timeout);
	if (timer->function) {
		timer->event_timer = shark_timer_set(&timeout, (void *)timer->function, (void *)timer->data);
	}

	#ifdef TIMER_DEBUG
	  printk("Set to %ld:%ld\n",timeout.tv_sec,timeout.tv_nsec);
	#endif
}


int del_timer(struct timer_list *timer)
{

	shark_timer_delete(timer->event_timer);
	return 0;

}

int mod_timer(struct timer_list *timer, unsigned long expires)
{

	if (timer->event_timer != -1) shark_timer_delete(timer->event_timer);
	timer->expires = expires;
	add_timer(timer);
	return 0;

}

void init_timer(struct timer_list * timer)
{
	timer->event_timer = -1;
	timer->function = NULL;
}

int timer_pending(struct timer_list * timer){
	return timer->event_timer != -1;
}

