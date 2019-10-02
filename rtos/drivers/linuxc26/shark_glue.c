#include <kernel/int_sem.h>
#include <stdlib.h>
#include <kernel/func.h>
#include <ll/sys/ll/event.h>
#include <ll/i386/pic.h>
#include <tracer.h>

#include <linuxcomp.h>

extern int add_interrupt_job(int no);
extern void set_noint_handler(void * new_handler);

extern int intr_count;

//#define DEBUG_SHARK_GLUE

/* 1-15 for IRQ and 16-63 for timers */
void *timer_arg_table[MAX_TIMER_TABLE];
void *timer_func_table[MAX_TIMER_TABLE];
int  timer_table[MAX_TIMER_TABLE];
int  intr_table[NR_IRQS];

int use_intdrive = 0;
static int current_timer = 0;

void shark_internal_sem_create(void **sem, int init) {
	*sem = (void *)malloc(sizeof(internal_sem_t));
	internal_sem_init((internal_sem_t *)(*sem),init);
}

void shark_internal_sem_wait(void *sem) {
	internal_sem_wait((internal_sem_t *)(sem));
}

void shark_internal_sem_post(void *sem) {
	internal_sem_post((internal_sem_t *)(sem));
}

/* Timers */

int get_free_timer_slot()
{
	int i = current_timer;

	while(timer_table[i] != -1) {
		if (i < MAX_TIMER_TABLE) {
			i++;
		} else {
			i = 0;
		}
		if (i == current_timer) return -1;
	}

	current_timer = i+1;
	if (current_timer >= MAX_TIMER_TABLE) current_timer = 0;

	return i;
}

void shark_timer_exec(int n)
{
	int no = n - 16;
	void(*tmp_func)(void*) = timer_func_table[no];

#ifdef DEBUG_SHARK_GLUE
	cprintf("(Timer EXEC %d)", no);
#endif

	if (tmp_func != NULL){
		intr_count++;
		(tmp_func)(timer_arg_table[no]);
		intr_count--;
	}

	timer_table[no] = -1;
	timer_func_table[no] = NULL;
	timer_arg_table[no] = NULL;
}

void fast_call_timer(void *arg)
{
	int no = (int)arg;
	int res;

#ifdef DEBUG_SHARK_GLUE
	cprintf("(Timer fast %d)", no);
#endif

	timer_table[no] = -2;

	if (use_intdrive == TRUE) {
		res = add_interrupt_job(no+16);
	} else {
		shark_timer_exec(no+16);
	}
}

int shark_timer_set(const struct timespec *time, void *handler, void *arg)
{
	SYS_FLAGS f;
	int i;

	f = kern_fsave();

	i = get_free_timer_slot();

	if (i == -1) {
		kern_frestore(f);
		return -1;
	}

	timer_func_table[i] = handler;
	timer_arg_table[i] = arg;

	timer_table[i] = kern_event_post(time, fast_call_timer, (void *)(i));
	
	if (timer_table[i] == -1) {
		kern_frestore(f);
		return -1;
	}

	kern_frestore(f);

	return i;
}

int shark_timer_delete(int index)
{
	SYS_FLAGS f;

	f = kern_fsave();

	if (index < 0 || index >= MAX_TIMER_TABLE) {
		kern_frestore(f);
		return -1;
	}

#ifdef DEBUG_SHARK_GLUE
	cprintf("(Timer Del %d)", index);
#endif

	if (timer_table[index] != -1 && timer_table[index] != -2) {
		timer_func_table[index] = NULL;
		timer_arg_table[index] = NULL;

		kern_event_delete(timer_table[index]);
		timer_table[index] = -1;
	}
	kern_frestore(f);

	return 0;
}

/* Interrupt */
int shark_handler_set(int no, void *fast){

	if ((no >= 1) && (no < 16) && (intr_table[no] <= 0)) {
		if (use_intdrive == TRUE) {
			intr_table[no] = handler_set(no, NULL, TRUE, NIL, fast);
#ifdef DEBUG_SHARK_GLUE
			cprintf("(shark_handler_set - INT: %d - %d)", no, intr_table[no]);
#endif
		} else {
			intr_table[no] = handler_set(no, fast, TRUE, NIL, NULL);
#ifdef DEBUG_SHARK_GLUE
			cprintf("(Shark_Handler_Set - NOINT: %d - %d)", no, intr_table[no]);
#endif
		}
		return intr_table[no];
	} else {
#ifdef DEBUG_SHARK_GLUE
		cprintf("(shark_handler_set - ERR: %d - %d)", no, intr_table[no]);
#endif
		return -1;
	}
}

int shark_handler_remove(int no){

	if (intr_table[no] == 1) {
		handler_remove(no);
#ifdef DEBUG_SHARK_GLUE
		cprintf("(shark_handler_remove: %d)", no);
#endif
		intr_table[no] = 0;
	}

	return 0;
}
 
/* Boot function */
int shark_interrupt_server(int use_intdrv) {
	int i;

	for(i=0; i<MAX_TIMER_TABLE; i++) { 
		timer_arg_table[i] = NULL;
		timer_func_table[i] = NULL;
		timer_table[i] = -1;
	}
	for(i=0; i<NR_IRQS; i++) { 
		intr_table[i] = 0;
	}
	
	use_intdrive = use_intdrv;
#ifdef DEBUG_SHARK_GLUE
	cprintf("(Use_intdrive: %d)", use_intdrive);
#endif
	if (use_intdrive == TRUE)
		set_noint_handler(shark_timer_exec);

	return 0;
}

