/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
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

//#define __KEYB_DEBUG__
#define KEYB_TASK

#include <kernel/kern.h>
#include <kernel/int_sem.h>
#include <signal.h>

#include "../include/drivers/shark_input26.h"
#include "../include/drivers/shark_keyb26.h"

/* Devices */
extern int  atkbd_init(int soft);
extern int  atkbd_exit(void);

/* Handlers */
extern int  kbd_init(void);
extern int  kbd_exit(void);

/* Functions */
extern int  kbd_enable(void);
extern int  kbd_disable(void);
extern int  kbd_get(unsigned int *data, BYTE access);
extern void kbd_setleds(unsigned int led);
extern int  kbd_rate(unsigned int *delay, unsigned int *period);
extern void kbd_mksound(unsigned int hz, unsigned int ticks);

/*
 * The following tables contains the ascii code corresponding to the
 * scan codes, with shift & no shift...
 */
char keyTable     [TABLE_KEY_SIZE];
char keyShiftTable[TABLE_KEY_SIZE];
char keyAltGrTable[TABLE_KEY_SIZE];

BYTE useAltGr = FALSE;

/* Status variables */
static BYTE shift = FALSE;
static BYTE ctrl  = FALSE;
static BYTE alt   = FALSE;
static BYTE altgr = FALSE;

static BYTE capslock   = FALSE;
static BYTE numlock    = TRUE;
static BYTE scrolllock = FALSE;

/* Keyboard Flags (if shift, alt or control are pressed...); */
static BYTE keyFlag = 0x00;
static BYTE status;

/* Keyboard driver currently installed */
static int keyb_installed = FALSE;


#define MAX_KEY_EXC     50

static struct keyexctable
{
	KEY_EVT evt;
	void (*func) (KEY_EVT * k);
	unsigned char lock;
} keyExcTable[MAX_KEY_EXC];

static int lastExc;

/* Buffer used for the communication between the extern process & the user */

/* Buffer Ssize */
#define KEYBUFF_SIZE 256
/* Buffer Mask ( i=(i+1)&MASK is better than i=(i+1)%SIZE ) */
#define KEYBUFF_MASK 0xff

static KEY_EVT keybuff[KEYBUFF_SIZE];
static int keybuff_head = 1;
static int keybuff_tail = 0;
static internal_sem_t keybuff_mutex;
static int keybuff_mutex_init = 0;

#ifdef KEYB_TASK
/* keyboard task PID */
static PID keybpid;
#else
static void keyProc(void);
#endif

/*
 * Start keyProc Task or exec it as function
 */
void shark_kbd_exec(void)
{
#ifdef KEYB_TASK
	task_activate(keybpid);
#else
	keyProc();
#endif
}

/* 
 * This function get a scan code, return an ascii code and set
 * the status vars in order to handle the special keys of the AT keyboard
 */
WORD scanCode(unsigned int c, int d)
{
	//printk("scanCode: c (%x) - d (%d)\n", c, d);

	/* KEY_EVT status setting */
	status = 1 << d;
	
	switch (c) {
		/* CapsLock pressed*/
		case KEY_CPSLOCK:
			if (d == KEY_PRESSED) {
				capslock = capslock ? FALSE : TRUE;
				/* light the caps lock led */
				kbd_setleds(scrolllock + (numlock << 1) + (capslock << 2));
			}
			return 0;
		/* NumLock pressed */
		case PAD_NUMLOCK:
			if (d == KEY_PRESSED) {
				numlock = numlock ? FALSE : TRUE;
				/* light the num lock led */
				kbd_setleds(scrolllock + (numlock << 1) + (capslock << 2));
			}
			return 0;
		/* ScrollLock pressed*/
		case EXT_SCRLOCK:
			if (d == KEY_PRESSED) {
				scrolllock = scrolllock ? FALSE : TRUE;
				/* light the scroll lock led */
				kbd_setleds(scrolllock + (numlock << 1) + (capslock << 2));
			}
			return 0;
		/* Shift pressed or released */
		case KEY_SHL:
			if (d) {
				shift = TRUE;
				if (c == KEY_SHL)
					keyFlag |= SHFL_BIT;
			} else {
				keyFlag &= (!SHFL_BIT);
				if (!(keyFlag & SHFR_BIT))
					shift = FALSE;
			}
			return 0;
		/* Shift pressed or released */
		case KEY_SHR:
			if (d) {
				shift = TRUE;
				if (c == KEY_SHR)
					keyFlag |= SHFR_BIT;
			} else {
				keyFlag &= (!SHFR_BIT);
				if (!(keyFlag & SHFL_BIT))
					shift = FALSE;
			}
			return 0;
		/* Control pressed or released */
		case KEY_CTRLL:
			if (d) {
				ctrl = TRUE;
				keyFlag |= CNTL_BIT;
			} else {
				keyFlag &= (!CNTL_BIT);
				if (!(keyFlag & CNTR_BIT))
					ctrl = FALSE;
			}
			return 0;
		/* Control pressed or released */
		case KEY_CTRLR:
			if (d) {
				ctrl = TRUE;
				keyFlag |= CNTR_BIT;
			} else {
				keyFlag &= (!CNTR_BIT);
				if (!(keyFlag & CNTL_BIT))
					ctrl = FALSE;
			}
			return 0;
		/* Alt Left pressed */
		case KEY_ALTL:
			if (d) {
				alt = TRUE;
				keyFlag |= ALTL_BIT;
			} else {
				alt = FALSE;
				keyFlag &= (!ALTL_BIT);
			}
			return 0;
		/* Alt Right (AltGr) pressed */
		case KEY_ALTR:
			if (d) {
				altgr = TRUE;
				keyFlag |= ALTR_BIT;
			} else {
				altgr = FALSE;
				keyFlag &= (!ALTR_BIT);
				if ((!useAltGr) && (!(keyFlag & ALTL_BIT)))
					alt = FALSE;
			}
			return 0;
		/* Delete */
		case EXT_DEL:
			return DELETE;
		case PAD_DEL:
			if (numlock || shift)
				return 0x18;
			else
				break;
		/* Pad Enter */
		case PAD_ENT:
			return 0x0d;
		/* Pad Add */
		case PAD_PLUS:
			return 0x2b;
		/* Pad Sub */
		case PAD_SUB:
			return 0x2d;
		/* Pad Slash */
		case PAD_SLH:
			return 0x2f;
		/* Numbers & simbols */
		case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5:
		case KEY_6: case KEY_7: case KEY_8: case KEY_9: case KEY_0:
		case KEY_SUB: case KEY_PLUS: case KEY_BRL: case KEY_BRR:
		case KEY_COL: case KEY_API:  case KEY_TIL: case KEY_BSL:
		case KEY_LT:  case KEY_GT:   case KEY_SLH:
			/* AlrGR enabled & pressed */
			if (altgr && useAltGr) {
				return keyAltGrTable[c];
			}
			/* Control Shift status */
			if (shift)
				return keyShiftTable[c];
			else
				return keyTable[c];
	}
	
	/* Pad Keys */
	if (numlock || shift)
		switch (c) {
			case PAD_DEL:
				return 0x18;
			case PAD_INS:
				return 0x30;
			case PAD_END:
				return 0x31;
			case PAD_DOWN:
				return 0x32;
			case PAD_PGDW:
				return 0x33;
			case PAD_RIGHT:
				return 0x34;
			case PAD_5:
				return 0x35;
			case PAD_LEFT:
				return 0x36;
			case PAD_HOME:
				return 0x37;
			case PAD_UP:
				return 0x38;
			case PAD_PGUP:
				return 0x39;
		}
	
	/* Characters keys */
	if (((c >= KEY_Q) && (c <= KEY_P)) ||
	    ((c >= KEY_A) && (c <= KEY_L)) ||
	    ((c >= KEY_Z) && (c <= KEY_M))) {
		/* AlrGR enabled & pressed */
		if (altgr && useAltGr) {
			return keyAltGrTable[c];
		}
		/* Control CapsLock & Shift status */
		if (capslock) {
			if (shift)
				return keyTable[c];
			else
				return keyShiftTable[c];
		} else {
			if (shift)
				return keyShiftTable[c];
			else
				return keyTable[c];
		}
	}

	/* Remaining keys */
	if (c < TABLE_KEY_SIZE)
		/* Left 'low' keys (Esc, BackSpace, Space, ...) */
		return keyTable[c];
	else
		/* Default - Return as keycode */
		return (0xff00 | c);
}

#ifdef KEYB_TASK
TASK keyProc(void)
#else
static void keyProc(void)
#endif
{
	WORD code;
	BYTE found;
	KEY_EVT dt;
	int i, res;
	unsigned int dato;

#ifdef KEYB_TASK
	while (1) {
#endif
		res = kbd_get(&dato, NON_BLOCK);
		if (res >= 0) {
			code = scanCode(dato, res);
			if (code != 0) {
				if (code & 0xff00)
					/* It's a scan code, set the right bit */
					dt.flag = (keyFlag | SCAN_BIT);
				else
					/* Simply send the keyFlag status */
					dt.flag = keyFlag;
				dt.status = status;
				dt.ascii  = (BYTE) (code & 0x00FF);
				dt.scan   = dato;
#ifdef __KEYB_DEBUG__
				printk("shark_keyb.c: KEY_EVT ( %2x - %c - %2x - %1d)\n", dt.scan, dt.ascii, dt.flag, dt.status);
#endif
				found = FALSE;
				for (i = 0; i < MAX_KEY_EXC; i++) {
					if (keyExcTable[i].func == NULL)
						continue;
					if (((keyExcTable[i].evt.flag & ~SCAN_BIT) == (dt.flag & ~SCAN_BIT)) && 
					    ((keyExcTable[i].evt.status & dt.status) != 0)) {
						if ((dt.flag & SCAN_BIT) != 0) {
							if ((keyExcTable[i].evt.scan == dt.scan) && (keyExcTable[i].evt.scan != 0)) {
#ifdef __KEYB_DEBUG__
								printk("shark_keyb.c: Key_Hook ( %2x - %2x - %1d) -> ( %2x - %2x - %1d)\n",
								dt.scan, dt.ascii, dt.flag, dt.status, 
								keyExcTable[i].evt.scan, keyExcTable[i].evt.ascii, keyExcTable[i].evt.flag, keyExcTable[i].evt.status);
#endif
								keyExcTable[i].func(&dt);
								if (keyExcTable[i].lock == TRUE)
									found = TRUE;
							}
						} else {
							if ( ((keyExcTable[i].evt.scan == dt.scan) || (keyExcTable[i].evt.scan == 0)) &&
							((keyExcTable[i].evt.ascii == dt.ascii) || (keyExcTable[i].evt.ascii == 0)) ){
#ifdef __KEYB_DEBUG__
								printk("shark_keyb.c: Key_Hook ( %2x - %2x - %1d) -> ( %2x - %2x - %1d)\n",
								dt.scan, dt.ascii, dt.flag, dt.status, 
								keyExcTable[i].evt.scan, keyExcTable[i].evt.ascii, keyExcTable[i].evt.flag, keyExcTable[i].evt.status);
#endif
								keyExcTable[i].func(&dt);
								if (keyExcTable[i].lock == TRUE)
									found = TRUE;
							}
						}
					}
				}
				/* when the port is full, data is lost */
				if (!found) {
					if (keybuff_tail != keybuff_head) {
						internal_sem_wait(&keybuff_mutex);
						memcpy(&keybuff[keybuff_head], &dt, sizeof(KEY_EVT));
						keybuff_head = (keybuff_head+1) & KEYBUFF_MASK;
						internal_sem_post(&keybuff_mutex);
					}
				}
			}
		}
#ifdef KEYB_TASK
		task_endcycle();
	}
#endif
}

/* default function called on ctrl-c */
void default_ctrlChandler(KEY_EVT * k) {
	/* Useful or not to set the page to 0? */
	/*set_active_page(0);
	set_visual_page(0);
	cputs("Ctrl-C pressed!\n");*/
	exit(0);
}

/**** Start User Functions ****/

/* Function that returns the ascii code */
BYTE keyb_getch(BYTE wait)
{
	KEY_EVT ev;

	while(1) {
		if ( ((keybuff_tail+1) & KEYBUFF_MASK) != ((keybuff_head) & KEYBUFF_MASK) ) {
			internal_sem_wait(&keybuff_mutex);
			keybuff_tail = (keybuff_tail+1) & KEYBUFF_MASK;
			memcpy(&ev, &keybuff[keybuff_tail], sizeof(KEY_EVT));
			internal_sem_post(&keybuff_mutex);

#ifdef __KEYB_DEBUG__
			printk("shark_keyb.c: GetChar ( %2x - %c - %2x - %1d)\n", ev.scan, ev.ascii, ev.flag, ev.status);
#endif
			if (!isScanCode(&ev) && !isReleased(&ev)) {
				return (ev.ascii);
			} else {
				if (wait != BLOCK)
					return 0;
			}
		} else {
			if (wait != BLOCK)
				return 0;
		}
	}
}

/*
 * Function that returns a structure containing the flags status, the ascii
 * code, and the scan code
 */
int keyb_getcode(KEY_EVT * k, BYTE wait)
{
	while (1) {
		if ( ((keybuff_tail+1) & KEYBUFF_MASK) != ((keybuff_head) & KEYBUFF_MASK) ) {
			internal_sem_wait(&keybuff_mutex);
			keybuff_tail = (keybuff_tail+1) & KEYBUFF_MASK;
			memcpy(k, &keybuff[keybuff_tail], sizeof(KEY_EVT));
			internal_sem_post(&keybuff_mutex);

			return(TRUE);
		} else {
			if (wait != BLOCK)
				return(FALSE);
		}
	}
}

/*
 * This call is used to link a function to a KEY_EVT
 */
int keyb_hook(KEY_EVT k, void (*f) (KEY_EVT * k), unsigned char l)
{
	int id = 0;

	if ((k.ascii == 0) && (k.scan == 0))
		return -1;

	while (keyExcTable[id].func != NULL) {
		id++;
		if (id >= MAX_KEY_EXC)
			return -1;
	}

	keyExcTable[lastExc].func = f;
	keyExcTable[lastExc].evt = k;
	if (keyExcTable[lastExc].evt.status == 0)
		keyExcTable[lastExc].evt.status = KEY_PRESSED;

	keyExcTable[lastExc++].lock = l;

	return (lastExc-1);
}

/* Free a keyb_hook slot */
int keyb_unhook(int index)
{
	if (keyExcTable[index].func != NULL) {
		keyExcTable[index].func = NULL;
		return 0;
	}
	return -1;
}

/* This function disable the keyboard */
void inline keyb_disable(void)
{
	kbd_disable();
}

/* This function enable the keyboard */
void inline keyb_enable(void)
{
	kbd_enable();
}

/**** End User Functions ****/
int inline KEYB26_installed(void)
{
	return keyb_installed;
}

int KEYB26_init(KEYB_PARMS *s)
{
	KEYB_PARMS kparms = BASE_KEYB;
	WORD i;
	int status = 0;

#ifdef KEYB_TASK
	SOFT_TASK_MODEL base_m;
	TASK_MODEL *m;
#endif

	if (keyb_installed == TRUE) return 0;

	/* if a NULL is passed */
	if (s == NULL)
		s = &kparms;

	/* Reset keymap structures */
	for (i = 0; i < TABLE_KEY_SIZE; i++) {
		keyTable[i] = 0;
		keyShiftTable[i] = 0;
		keyAltGrTable[i] = 0;
	}

	/* keymap */
	if (s->keymap == (unsigned char)KEYB_DEFAULT) s->keymap = KEYMAP_US;
	keyb_set_map(s->keymap);

	/* KeyEvt Buffer Initialization */
	keybuff_head = 1;
	keybuff_tail = 0;
	if (keybuff_mutex_init==0) {
		internal_sem_init(&keybuff_mutex, 1);
		keybuff_mutex_init = 1;
	}

	/* remove all key pressed handlers */
	lastExc = 0;

	/* and add a ctrl-c handler if requested */
	if (s->ctrlcfunc == (void *) KEYB_DEFAULT)
		s->ctrlcfunc = (void *) default_ctrlChandler;
	if (s->ctrlcfunc != NULL) {
		KEY_EVT emerg;
		emerg.ascii  = 'c';
		emerg.scan   = KEY_C;
		emerg.status = KEY_PRESSED;
		emerg.flag   = CNTL_BIT;
		keyb_hook (emerg, s->ctrlcfunc, FALSE);
		emerg.flag   = CNTR_BIT;
		keyb_hook (emerg, s->ctrlcfunc, FALSE);
	}

#ifdef KEYB_TASK
	/* keyb task */
	if (s->tm == (TASK_MODEL *) KEYB_DEFAULT) {
		soft_task_default_model (base_m);
		soft_task_def_wcet (base_m, 2000);
		soft_task_def_met (base_m, 800);
		soft_task_def_period (base_m, 30000);
		soft_task_def_system (base_m);
		soft_task_def_nokill (base_m);
		soft_task_def_aperiodic (base_m);
		m = (TASK_MODEL *) & base_m;
	} else
		m = s->tm;

#ifdef __KEYB_DEBUG__
	printk(KERN_DEBUG "keyb task: %li %li %li %li\n", (long)s->pclass, (long)APERIODIC, (long)s->dline, (long)s->wcet);
#endif

	keybpid = task_create ("KeyTask", keyProc, m, NULL);
	if (keybpid == -1)
		return -1;
#endif

	if (INPUT26_installed() == FALSE)
		if (INPUT26_init()) {
			printk(KERN_ERR "shark_keyb.c: Unable to open Input SubSystem.\n");
#ifdef KEYB_TASK
			task_kill (keybpid);
#endif
			return -1;
		}

	status = kbd_init();
	if (status) {
		printk(KERN_ERR "shark_keyb.c: Kbd_Init return: %d\n", status);
#ifdef KEYB_TASK
		task_kill (keybpid);
#endif
		return -1;
	}

	
	if (s->softrepeat == (int) KEYB_DEFAULT) {
		status = atkbd_init(1);
	} else {
		status = atkbd_init(s->softrepeat);
	}
	if (status) {
		printk(KERN_ERR "shark_keyb.c: AtKbd_Init return: %d\n", status);
#ifdef KEYB_TASK
		task_kill (keybpid);
#endif
		return -1;
	}

	kbd_setleds(scrolllock + (numlock << 1) + (capslock << 2));
	kbd_enable();

	keyb_installed = TRUE;
	return status;
}

/* 
 * KEYB module cleanup 
 * (must be called if it's needed to re-call keyb_init() with other parameters)
 * -1 -> ERROR
 */
int KEYB26_close(void)
{
#ifdef KEYB_TASK
	int free;
	SYS_FLAGS f;
#endif

	if (!keyb_installed)
		return -1;

	kbd_disable();

	atkbd_exit();
	kbd_exit();

#ifdef KEYB_TASK
	f = kern_fsave();
	free = (proc_table[keybpid].status == FREE);
	kern_frestore(f);
#ifdef __KEYB_DEBUG__
	printk(KERN_DEBUG "shark_keyb.c: KeyTask is %s.\n", free ? "killed" : "alive");
#endif
	if (free)
		task_kill (keybpid);
#endif

	keyb_installed = FALSE;

	return 0;
}
