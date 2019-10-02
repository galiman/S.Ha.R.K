/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     :
 *   Giacomo Guidi 	 <giacomo@gandalf.sssup.it>
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

#include <intdrive/intdrive/intdrive.h>
#include <intdrive/intdrive/inttask.h>
#include <kernel/model.h>
#include <kernel/descr.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <tracer.h>

#include <ll/i386/64bit.h>

/*+ Status used in the level +*/
#define INTDRIVE_READY         MODULE_STATUS_BASE    /*+ - Ready status        +*/
#define INTDRIVE_WCET_VIOLATED MODULE_STATUS_BASE+2  /*+ when wcet is finished +*/
#define INTDRIVE_IDLE          MODULE_STATUS_BASE+3  /*+ to wait the replenish +*/
#define INTDRIVE_WAIT	       MODULE_STATUS_BASE+4  /*+ to wait the activation */

//#define INTDRIVE_DEBUG

/*+ the level redefinition for the IntDrive +*/
typedef struct {
	level_des l;     /*+ the standard level descriptor          +*/

	TIME replenish_period;
	TIME capacity;
	TIME q_theta;

	struct timespec act_time;

	int avail;
	int replenish_timer;

	//struct timespec replenish_expires;
	//int wcet_timer;

	int act_number;   /*+ the activation number                  +*/

	int flags;        /*+ the init flags...                      +*/

	bandwidth_t U;    /*+ the used bandwidth                     +*/

} INTDRIVE_level_des;

PID INTDRIVE_task = NIL;

/* Replenish the capacity */
static void INTDRIVE_timer(void *arg)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(arg);

	lev->replenish_timer = NIL;

#ifdef INTDRIVE_DEBUG
	kern_printf("(INTD:TIMER)");
#endif

	if (INTDRIVE_task == NIL) return;

	lev->avail = lev->q_theta;
	
#ifdef INTDRIVE_DEBUG
	TRACER_LOGEVENT(FTrace_EVT_user_event_0, 0, lev->avail + INT_MAX);
#endif

	switch (proc_table[INTDRIVE_task].status) {
		case INTDRIVE_IDLE:
			if (lev->act_number) {
				proc_table[INTDRIVE_task].status = INTDRIVE_READY;
				event_need_reschedule();
			} else {
				proc_table[INTDRIVE_task].status = INTDRIVE_WAIT;
			}
			break;
	}
}

/*static void INTDRIVE_wcet_timer(void *arg)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(arg);

	lev->wcet_timer = NIL;

	kern_raise(XWCET_VIOLATION,INTDRIVE_task);
}*/

static PID INTDRIVE_public_scheduler(LEVEL l)
{
	if (INTDRIVE_task == NIL) return NIL;

	if (proc_table[INTDRIVE_task].status == INTDRIVE_READY ||
	    proc_table[INTDRIVE_task].status == EXE)
		return INTDRIVE_task;
	else
		return NIL;
}

static int INTDRIVE_public_create(LEVEL l, PID p, TASK_MODEL *m)
{
	INTERRUPT_TASK_MODEL *h;

	if (m->pclass != INTERRUPT_PCLASS) return -1;
	if (m->level != l) return -1; 
	h = (INTERRUPT_TASK_MODEL *)m;
	if (!h->wcet) return -1;

	if (INTDRIVE_task != NIL) return -1;

	INTDRIVE_task = p;

	proc_table[INTDRIVE_task].wcet = h->wcet;
	proc_table[INTDRIVE_task].avail_time = h->wcet;
	proc_table[INTDRIVE_task].status = INTDRIVE_WAIT;
	proc_table[INTDRIVE_task].control &= ~CONTROL_CAP; 

	return 0;
}

static void INTDRIVE_public_dispatch(LEVEL l, PID p, int nostop)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);
	//struct timespec time;

	kern_gettime(&(lev->act_time));

	/*TIMESPEC_ASSIGN(&time,&(lev->act_time));
	ADDUSEC2TIMESPEC(proc_table[INTDRIVE_task].wcet,&time);

	if (lev->flags == INTDRIVE_CHECK_WCET) 
		lev->wcet_timer = kern_event_post(&time,INTDRIVE_wcet_timer,(void *)lev);*/  
}

static void INTDRIVE_public_epilogue(LEVEL l, PID p)
{
	struct timespec time;

	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);

	/*if (lev->wcet_timer != NIL)
		kern_event_delete(lev->wcet_timer);*/

	SUBTIMESPEC(&schedule_time, &(lev->act_time), &time);
	lev->avail -= TIMESPEC2USEC(&time);
	
#ifdef INTDRIVE_DEBUG
	TRACER_LOGEVENT(FTrace_EVT_user_event_0, 0, lev->avail + INT_MAX);
#endif

	if (proc_table[INTDRIVE_task].wcet < TIMESPEC2USEC(&time)) {
		kern_raise(XWCET_VIOLATION,INTDRIVE_task);    
	}
}

static void INTDRIVE_public_activate(LEVEL l, PID p, struct timespec *t)
{
	struct timespec acttime;
	TIME time, delta_capacity;

	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);

	if (proc_table[INTDRIVE_task].status == INTDRIVE_WAIT) {
		proc_table[INTDRIVE_task].status = INTDRIVE_READY;
		lev->act_number++;	
	} else {
		if (proc_table[INTDRIVE_task].status == INTDRIVE_IDLE ||
		    proc_table[INTDRIVE_task].status == INTDRIVE_READY ||
		    proc_table[INTDRIVE_task].status == EXE) {
#ifdef INTDRIVE_DEBUG
			kern_printf("(INTD:WAIT_REC)");
#endif
			lev->act_number++;
		}
	}

	if (lev->replenish_timer == NIL) {
		delta_capacity = lev->q_theta - lev->avail;
		mul32div32to32(delta_capacity, MAX_BANDWIDTH, lev->U, time);
		kern_gettime(&acttime);
		ADDUSEC2TIMESPEC(time,&acttime);
		lev->replenish_timer = kern_event_post(&acttime,INTDRIVE_timer,(void *)lev);

		/*kern_gettime(&(lev->replenish_expires));
		ADDUSEC2TIMESPEC(lev->replenish_period,&(lev->replenish_expires));
		lev->replenish_timer = kern_event_post(&(lev->replenish_expires),INTDRIVE_timer,(void *)lev);*/
	}
}

static void INTDRIVE_public_unblock(LEVEL l, PID p)
{
	/* Insert task in the correct position */
	proc_table[INTDRIVE_task].status = INTDRIVE_READY;
}

static void INTDRIVE_public_block(LEVEL l, PID p)
{

}

static int INTDRIVE_public_message(LEVEL l, PID p, void *m)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);
	struct timespec time, acttime;
	//int delta_time;
	TIME delta_capacity, delta_time;

	lev->act_number--;

	/*if (lev->wcet_timer != NIL)
	kern_event_delete(lev->wcet_timer);*/

	kern_gettime(&acttime);
	SUBTIMESPEC(&acttime, &(lev->act_time), &time);
	delta_time = TIMESPEC2USEC(&time);
	mul32div32to32(delta_time, (MAX_BANDWIDTH-lev->U), MAX_BANDWIDTH, delta_capacity);
	lev->avail -= delta_capacity;

	//lev->avail -= TIMESPEC2USEC(&time);

#ifdef INTDRIVE_DEBUG
	TRACER_LOGEVENT(FTrace_EVT_user_event_0, 0, lev->avail + INT_MAX);
#endif

#ifdef INTDRIVE_DEBUG
	kern_printf("(INTD:AV:%d)",(int)(lev->avail));
#endif

	if (lev->avail < 0) {
		proc_table[INTDRIVE_task].status = INTDRIVE_IDLE;

		if (lev->replenish_timer != NIL)
			kern_event_delete(lev->replenish_timer);

		delta_capacity = lev->q_theta - lev->avail;
		mul32div32to32(delta_capacity, MAX_BANDWIDTH, lev->U, delta_time);
		kern_gettime(&acttime);
		ADDUSEC2TIMESPEC(delta_time,&acttime);
		lev->replenish_timer = kern_event_post(&acttime,INTDRIVE_timer,(void *)lev);

		/*temp = -lev->avail;
		mul32div32to32(temp,lev->replenish_period,lev->capacity,delta_time)
		ADDUSEC2TIMESPEC(delta_time,&(lev->replenish_expires));
		lev->replenish_timer = kern_event_post(&(lev->replenish_expires),INTDRIVE_timer,(void *)lev);*/

#ifdef INTDRIVE_DEBUG
		kern_printf("(INTD:IDLE:%d)",delta_time);
#endif
	} else {
		if (lev->act_number) {
			proc_table[INTDRIVE_task].status = INTDRIVE_READY;
#ifdef INTDRIVE_DEBUG
			kern_printf("(INTD:NEXT_ACT)");
#endif
		} else {
#ifdef INTDRIVE_DEBUG
			kern_printf("(INTD:WAIT_ACT)");
#endif
			proc_table[INTDRIVE_task].status = INTDRIVE_WAIT;
		}
	}

	TRACER_LOGEVENT(FTrace_EVT_task_end_cycle, (unsigned short int)proc_table[INTDRIVE_task].context,(unsigned int)l); 

	return 0;
}

static void INTDRIVE_public_end(LEVEL l, PID p)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);

	if (lev->replenish_timer != NIL)
		kern_event_delete(lev->replenish_timer);

	/*if (lev->wcet_timer != NIL)
		kern_event_delete(lev->wcet_timer);*/

	proc_table[INTDRIVE_task].status = INTDRIVE_IDLE;
}

/* Registration functions */

PID intr_server = NIL;
extern TASK Interrupt_Server(void *arg);
extern TASK Interrupt_Server_Prot(void *arg);

static void intdrive_create(void *l)
{
	INTERRUPT_TASK_MODEL ht;
	int level;
	int wcet = 10000;   // TODO: should be a parameter

	level = (LEVEL)l;

	interrupt_task_default_model(ht);
	interrupt_task_def_wcet(ht, wcet);
	interrupt_task_def_system(ht);
	interrupt_task_def_nokill(ht);

	if (level > 0)
		intr_server = task_create("Interrupt Server (Protected)",Interrupt_Server_Prot,&ht,NULL);
	else
		intr_server = task_create("Interrupt Server",Interrupt_Server,&ht,NULL);
		
	if (intr_server == NIL)
    printk("\nPanic!!! Can't create IntDrive task...\n");
}

/*+ Registration function: +*/
LEVEL INTDRIVE_register_level(TIME capacity, TIME q_theta, int U, int flags)
{
	LEVEL l;            /* the level that we register */
	INTDRIVE_level_des *lev;

	printk("INTDRIVE_register_level\n");

	/* request an entry in the level_table */
	l = level_alloc_descriptor(sizeof(INTDRIVE_level_des));

	lev = (INTDRIVE_level_des *)level_table[l];

	lev->l.public_scheduler = INTDRIVE_public_scheduler;
	lev->l.public_guarantee = NULL;
	lev->l.public_create    = INTDRIVE_public_create;
	lev->l.public_end       = INTDRIVE_public_end;
	lev->l.public_dispatch  = INTDRIVE_public_dispatch;
	lev->l.public_epilogue  = INTDRIVE_public_epilogue;
	lev->l.public_activate  = INTDRIVE_public_activate;
	lev->l.public_unblock   = INTDRIVE_public_unblock;
	lev->l.public_block     = INTDRIVE_public_block;
	lev->l.public_message   = INTDRIVE_public_message;

	NULL_TIMESPEC(&(lev->act_time));

	lev->capacity = capacity;
	lev->replenish_timer = NIL;
	lev->flags = flags;
	lev->act_number = 0;
	lev->avail = 0;
	lev->q_theta = q_theta;
	lev->U = U;
	//mul32div32to32(MAX_BANDWIDTH,U,10000,lev->U);
#ifdef INTDRIVE_DEBUG
	printk("INTDRIVE Init: %d %d %d\n", lev->capacity, lev->q_theta, lev->U);
#endif

	sys_atrunlevel(intdrive_create, (void *) l, RUNLEVEL_INIT);

	return l;
}

bandwidth_t INTDRIVE_usedbandwidth(LEVEL l)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);

	return lev->U;
}

TIME INTDRIVE_set_q_theta(LEVEL l, TIME new_q_theta)
{
	INTDRIVE_level_des *lev = (INTDRIVE_level_des *)(level_table[l]);

	lev->q_theta = new_q_theta;
	if (lev->q_theta < 0) lev->q_theta = 0;
	if (lev->q_theta > lev->capacity) lev->q_theta = lev->capacity;

	return lev->q_theta;
}
