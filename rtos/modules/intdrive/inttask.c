/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     :
 *   ...	 <......>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * Copyright (C) 2000,2002 Paolo Gai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* Interrupt Driver Module */

#include <kernel/int_sem.h>
#include <stdlib.h>
#include <kernel/func.h>
#include <ll/sys/ll/event.h>
#include <ll/i386/pic.h>
#include <tracer.h>

#include <intdrive/intdrive/inttask.h>

//#define DEBUG_SHARK_GLUE

/*
 * Moved into intdrive.c
 */
extern PID intr_server;

void (*noint_handler)(int n);

#define MAX_INT_LIST 50

int int_list[MAX_INT_LIST];

int next_free_int = 0;
int next_execute_int = 0;
int n_intact = 0, n_lost = 0;

/* FIFO add job */
int add_interrupt_job(int no)
{
	int old_free_int = next_free_int;

#ifdef DEBUG_SHARK_GLUE
	kern_printf("(add_interrupt_job: %d)", no);
#endif

	if (no<16)
		irq_mask(no);

#ifdef DEBUG_SHARK_GLUE
	TRACER_LOGEVENT(FTrace_EVT_user_event_1, no, 0);
#endif

	int_list[next_free_int] = no;
	next_free_int++;

	if (next_free_int == MAX_INT_LIST) next_free_int = 0;
	if (next_free_int == next_execute_int) {
		next_free_int = old_free_int;
		n_lost++;
		//Raise an exception?!?
		return -1;
	}

	if (intr_server!=NIL)
		task_activate(intr_server);

	return 0;
}

/* FIFO get job */
int get_interrupt_job()
{
	int res = -1;

	if (next_free_int != next_execute_int) {
		res = int_list[next_execute_int];
		next_execute_int++;
		if (next_execute_int == MAX_INT_LIST) next_execute_int = 0;
	}

#ifdef DEBUG_SHARK_GLUE
	TRACER_LOGEVENT(FTrace_EVT_user_event_2, res, 0);
#endif

#ifdef DEBUG_SHARK_GLUE
	kern_printf("(get_interrupt_job: %d)", res);
#endif

	return res;
}

/* The Interrupt TASK is an aperiodic task designed for
	the INTDRIVE module. */

void interrupt_job()
{
	void (*tmp_fast)(int n);
	int no;

	n_intact++;

	no = get_interrupt_job();

#ifdef DEBUG_SHARK_GLUE
	kern_printf("(interrupt_job: no %d)",no);
#endif

	if (no != -1 && no < 16) {
		tmp_fast = handler_get_intdrive(no);

		/*extern void linux_intr(int irq);
		linux_intr(no);*/

		(tmp_fast)(no);
		irq_unmask(no);
	}

	if (no != -1 && no >= 16) {
		(noint_handler)(no);
	}
}

/*
 * When an handler is removed pending activations have to be removed.
 * This may cause interrupt losses, drivers should be aware of that.
 */
void invalidate_pending_jobs(int no)
{
	int i;

	i = next_execute_int;
	while (i != next_free_int) {
		if (int_list[i] == no)
			int_list[i] = -1;
		i = (i + 1) % MAX_INT_LIST;
	}
}

TASK Interrupt_Server(void *arg)
{
	while(1) {
		interrupt_job();

		task_endcycle();
	}

}

TASK Interrupt_Server_Prot(void *arg)
{
	while(1) {
		task_nopreempt();
		interrupt_job();
		task_preempt();

		task_endcycle();
	}

}

void set_noint_handler(void * new_handler)
{
	noint_handler = new_handler;
}
