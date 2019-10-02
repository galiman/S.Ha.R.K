/* Project:     OSLib
 * Description: The OS Construction Kit
 * Date:                1.6.2000
 * Idea by:             Luca Abeni & Gerardo Lamastra
 *
 * OSLib is an SO project aimed at developing a common, easy-to-use
 * low-level infrastructure for developing OS kernels and Embedded
 * Applications; it partially derives from the HARTIK project but it
 * currently is independently developed.
 *
 * OSLib is distributed under GPL License, and some of its code has
 * been derived from the Linux kernel source; also some important
 * ideas come from studying the DJGPP go32 extender.
 *
 * We acknowledge the Linux Community, Free Software Foundation,
 * D.J. Delorie and all the other developers who believe in the
 * freedom of software and ideas.
 *
 * For legalese, check out the included GPL license.
 */

/* Added Advanced Timer Code
 *
 * Date:   8.4.2003
 * Author: Giacomo Guidi <giacomo@gandalf.sssup.it>
 *
 */

/*	Time Event routines	(one shot mode)	*/

#include <ll/i386/stdlib.h>
#include <ll/i386/mem.h>
#include <ll/i386/pit.h>
#include <ll/i386/apic.h>
#include <ll/i386/advtimer.h>
#include <ll/i386/error.h>
#include <ll/i386/64bit.h>
#include <ll/i386/limits.h>

#include <ll/sys/ll/ll-data.h>
#include <ll/sys/ll/ll-func.h>
#include <ll/sys/ll/time.h>
#include <ll/sys/ll/event.h>

#include <tracer.h>
extern unsigned short int currCtx;

FILE(EventOneShot);

extern int activeInt;
int activeEvent;

extern BYTE frc;

extern struct event eventlist[MAX_EVENT];
extern WORD lastTime;
extern struct pitspec globalCounter;

extern struct event *freeevents;
extern struct event *firstevent;

extern void (*evt_prol) (void);
extern void (*evt_epil) (void);

extern unsigned int apic_clk_per_msec;
extern unsigned char use_tsc, use_apic;

/* Switched to timespec */
int oneshot_event_post(struct timespec time, void (*handler) (void *p), void *par)
{
	struct event *p;
	struct event *p1, *t;
	struct timespec now, tmp;
	int done;
	DWORD tnext;

	TRACER_LOGEVENT(FTrace_EVT_timer_post, 0, 0);

	if (!freeevents) {
		message("NO FREE EVENTS !\n");
		ll_abort(20);
		return -1;
	}
	/* Extract from the ``free events'' queue */
	p = freeevents;
	freeevents = p->next;

	/* Fill the event fields */
	p->handler = handler;
	TIMESPEC_ASSIGN(&(p->time), &time);
	p->par = par;

	/* ...And insert it in the event queue!!! */

	t = NULL;
	done = 0;
	/* walk through list, finding spot, adjusting ticks parameter */
	for (p1 = firstevent; p1; p1 = t->next) {
		if (TIMESPEC_A_GT_B((&time), (&p1->time))) {
		t = p1;
		} else
		break;
	}

	/* adjust next entry */
	if (t) {
		t->next = p;
	} else {
		firstevent = p;
		if (!activeEvent) {
			if (!use_tsc) {
				ll_gettime(TIME_NEW, &now);
			} else {
				ll_read_timespec(&now);
			}
			if (TIMESPEC_A_GT_B(&now, &(firstevent->time))) {
				NULL_TIMESPEC(&tmp);
			} else {
				SUBTIMESPEC(&(firstevent->time), &now, &tmp);
			}
			tnext = TIMESPEC2USEC(&tmp);
			if (!use_apic) {
				mul32div32to32(tnext,1193182,1000000,tnext);
				pit_setconstant(0, tnext);
			} else {
				mul32div32to32(tnext,apic_clk_per_msec,1000,tnext);
				set_APIC_timer(tnext);
			}
		}
	}
	p->next = p1;

	return p->index;
}

void oneshot_wake_up(void)
{
	/* CHANGE the NAME, please... */
	struct event *p = NULL, *pp;
	struct timespec now, ttmp;
	WORD tmp;
	DWORD tnext;
	DWORD max_tnext;




	TRACER_LOGEVENT(FTrace_EVT_timer_wakeup_start, 0, 0);

	if (!use_tsc) {
		tmp = pit_read(frc);
		ADDPITSPEC((WORD) (lastTime - tmp), &globalCounter);
		lastTime = tmp;

		PITSPEC2TIMESPEC(&globalCounter, &now);
	} else {
		ll_read_timespec(&now);
	}

	if (firstevent != NULL) {
		activeEvent = 1;
		if (TIMESPEC_A_GT_B(&now, &(firstevent->time))) {
			if (!activeInt && evt_prol != NULL) {
				evt_prol();
			}
			activeInt++;

			for (p = firstevent; p != NULL; p = pp) {
				if ((p->time.tv_sec > now.tv_sec) ||
				   ((p->time.tv_sec == now.tv_sec)
				   && (p->time.tv_nsec > now.tv_nsec))) {
					break;
				}
				pp = p->next;
				p->next = freeevents;
				freeevents = p;
				firstevent = pp;
				p->handler(p->par);
			}
	
			if (activeInt == 1 && evt_epil != NULL) {
				evt_epil();
			}
			activeInt--;
		}

		if (!use_tsc) {
			tmp = pit_read(frc);
			ADDPITSPEC((WORD) (lastTime - tmp), &globalCounter);
			lastTime = tmp;
	
			PITSPEC2TIMESPEC(&globalCounter, &now);
		} else {
			ll_read_timespec(&now);
		}

		if (firstevent) {
		  if (TIMESPEC_A_GT_B(&now, &(firstevent->time))) {
		    NULL_TIMESPEC(&ttmp);
		  } else {
		    SUBTIMESPEC(&(firstevent->time), &now, &ttmp);
		  }
		  /*  SUBTIMESPEC(&(firstevent->time), &now, &ttmp); */
		  tnext = TIMESPEC2USEC(&ttmp);
		  
		  if (!use_apic) {
		    mul32div32to32(INT_MAX,1000000,1193182,max_tnext);		       
		  } else {
		    mul32div32to32(INT_MAX,1000,apic_clk_per_msec,max_tnext);		       
		  }
		  if (tnext>max_tnext) {
		    message("(************TIME IN THE FUTURE************)"); 
		  }
		  
		  if (!use_apic) {
		    mul32div32to32(tnext,1193182,1000000,tnext);
		    pit_setconstant(0, tnext);
		  } else {
			mul32div32to32(tnext,apic_clk_per_msec,1000,tnext);
			set_APIC_timer(tnext);
		  }
		} else {
		  if (!use_apic)
		    pit_setconstant(0, 0xFFFF);
		  else
		    set_APIC_timer(0xFFFFFFFF);		  
		}
		  activeEvent = 0;
	} else {
		if (!use_apic)
			pit_setconstant(0, 0xFFFF);
		else
			set_APIC_timer(0xFFFFFFFF);
	}

	TRACER_LOGEVENT(FTrace_EVT_timer_wakeup_end, (unsigned short int)currCtx, 0);
}

int oneshot_event_delete(int index)
{
	struct event *p1, *t;
	struct timespec tmp, now;
	DWORD tnext;
	int firstdeleted = FALSE;

	TRACER_LOGEVENT(FTrace_EVT_timer_delete, 0, 0);

	if (index == -1)
		return -1;
	t = NULL;
	/* walk through list, finding spot, adjusting ticks parameter */
	
	for (p1 = firstevent; (p1) && (index != p1->index); p1 = t->next) {
		t = p1;
	}
	
	if (p1 == NULL) {
		return -1;
	}
	if (t == NULL) {
		firstevent = p1->next;
		firstdeleted = TRUE;
	} else {
		t->next = p1->next;
	}
	
	p1->next = freeevents;
	freeevents = p1;
	
	if (!activeEvent) {
		if (firstevent == NULL) {
			if (!use_apic)
				pit_setconstant(0, 0xFFFF);
			else
				set_APIC_timer(0xFFFFFFFF);
		} else {
			if (firstdeleted) {
				if (!use_tsc)
					ll_gettime(TIME_NEW, &now);
				else
					ll_read_timespec(&now);
				if (TIMESPEC_A_GT_B(&now, &(firstevent->time))) {
					NULL_TIMESPEC(&tmp);
				} else {
					SUBTIMESPEC(&(firstevent->time), &now, &tmp);
				}
				/*SUBTIMESPEC(&now, &(firstevent->time), &tmp); */
				tnext = TIMESPEC2USEC(&tmp);
				if (!use_apic) {
					mul32div32to32(tnext,1193182,1000000,tnext);
					pit_setconstant(0, tnext);
				} else {
					mul32div32to32(tnext,apic_clk_per_msec,1000,tnext);
					set_APIC_timer(tnext);
				}
			}
		}
	}
	return 1;
}
