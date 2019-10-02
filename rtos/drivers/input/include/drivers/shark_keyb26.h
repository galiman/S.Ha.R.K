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

#ifndef __KEYB_H__
#define __KEYB_H__

#include <kernel/const.h>
#include <kernel/model.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "keycode.h"

typedef struct {
	BYTE useAltGr;
	char keyMap     [TABLE_KEY_SIZE];
	char keyShiftMap[TABLE_KEY_SIZE];
	char keyAltGrMap[TABLE_KEY_SIZE];
} KEYB_MAP;

extern KEYB_MAP keyMaps[];

#define KEYMAP_US	0
#define KEYMAP_IT	1

/* Key Status */
#define KEY_RELEASED	1
#define KEY_PRESSED	2
#define KEY_REPEATED	4

/* Ascii Codes */
#define BACKSPACE	0x08
#define ENTER		0x0d
#define DELETE		0x18
#define ESC		0x1b
#define TAB		0x09

/* Flag Codes */
#define ALTR_BIT	0x01
#define ALTL_BIT	0x02
#define CNTR_BIT	0x04
#define CNTL_BIT	0x08
#define SHFL_BIT	0x10
#define SHFR_BIT	0x20
#define SCAN_BIT	0x40

typedef struct {
	BYTE ascii;
	BYTE scan;
	BYTE flag;
	BYTE status;
} KEY_EVT;

#define set_keyevt(k,a,c,f,s)	(k)->ascii  = a, \
				(k)->scan   = c, \
				(k)->flag   = f, \
				(k)->status = s

#define isReleased(k)	( ((k)->status & KEY_RELEASED) != 0 )
#define isPressed(k)	( ((k)->status & KEY_PRESSED) != 0 )
#define isRepeated(k)	( ((k)->status & KEY_REPEATED) != 0 )

#define isScanCode(k)	((k)->flag & SCAN_BIT)
#define isLeftShift(k)	((k)->flag & SHFL_BIT)
#define isRightShift(k)	((k)->flag & SHFR_BIT)
#define isLeftCtrl(k)	((k)->flag & CNTL_BIT)
#define isRightCtrl(k)	((k)->flag & CNTR_BIT)
#define isLeftAlt(k)	((k)->flag & ALTL_BIT)
#define isRightAlt(k)	((k)->flag & ALTR_BIT)

#define keyb_getchar()	keyb_getch(BLOCK)

/*
 * keyboard initialization
 */
 
/* the KEYB_PARMS structure used by KEYB26_init() */
typedef struct keyb_parms {
	TASK_MODEL *tm;
	unsigned char keymap;
	void  (*ctrlcfunc)(KEY_EVT *k);
	int softrepeat;
} KEYB_PARMS;

#define KEYB_DEFAULT ((unsigned long)(-1))   /*+ used for default params +*/

#define BASE_KEYB	{(TASK_MODEL *)KEYB_DEFAULT, \
			 (unsigned char)KEYB_DEFAULT, \
			 (void *)KEYB_DEFAULT, \
			 (int) KEYB_DEFAULT}

#define keyb_default_parm(m)	(m).tm = (TASK_MODEL *) KEYB_DEFAULT, \
				(m).keymap = (unsigned char) KEYB_DEFAULT,  \
				(m).ctrlcfunc = (void *) KEYB_DEFAULT, \
				(m).softrepeat = (int) KEYB_DEFAULT

#define keyb_def_map(s,m)	(s).keymap = (m)
#define keyb_def_ctrlC(s,f)	(s).ctrlcfunc = (f)
#define keyb_def_task(s,m)	(s).tm = (TASK_MODEL *)(m)
#define keyb_def_srepeat(s,r)	(s).softrepeat = (r)

int  KEYB26_installed(void);
int  KEYB26_init(KEYB_PARMS *s);
int  KEYB26_close(void);

BYTE keyb_getch(BYTE wait);
int  keyb_getcode(KEY_EVT *k, BYTE wait);
int  keyb_hook(KEY_EVT k, void (*f)(KEY_EVT *k), unsigned char l);
int  keyb_unhook(int index);
void keyb_enable(void);
void keyb_disable(void);
int  keyb_set_map(unsigned char m);
int  keyb_get_map(void);

#ifdef __cplusplus
};
#endif

#endif
