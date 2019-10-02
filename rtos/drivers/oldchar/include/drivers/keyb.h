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
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: keyb.h,v 1.1 2003/03/24 10:54:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 10:54:17 $
 ------------

**/

/*
 * Copyright (C) 2000 Giuseppe Lipari
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

/* Project:     HARTIK 3.0                                      */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Giuseppe Lipari                                 */
/* Start date   : 19/6/96                                       */

/* CVS $Id: keyb.h,v 1.1 2003/03/24 10:54:17 pj Exp $ */

/* File:        Keyb.H                                          */
/* Revision:    1.4b                                            */

/* Last update  : 22/3/99                                       */

/* (MG)                                                         */
/* -- added keyb_enable() & keyb_disable()                      */
/* -- changed keyb_init() definition                            */

#ifndef __KEYB_H__
#define __KEYB_H__

#include <kernel/const.h>
#include <kernel/model.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <drivers/keycode.h>

/* Ascii Codes */
#define BACKSPACE       8
#define ENTER           13
#define DELETE          24
#define ESC             27
#define TAB             9

/* Scan Codes */
#define UP_KEY          72
#define DOWN_KEY        80
#define LEFT_KEY        75
#define RIGHT_KEY       77
#define PGUP_KEY        73
#define PGDW_KEY        81
#define HOME_KEY        71
#define END_KEY         79
#define INS_KEY         82

#define F1_KEY          59
#define F2_KEY          60
#define F3_KEY          61
#define F4_KEY          62
#define F5_KEY          63
#define F6_KEY          64
#define F7_KEY          65
#define F8_KEY          66
#define F9_KEY          67
#define F10_KEY         68

/* Flag Codes */
#define ALTR_BIT        0x001
#define ALTL_BIT        0x002
#define CNTR_BIT        0x004
#define CNTL_BIT        0x008
#define SHFL_BIT        0x010
#define SHFR_BIT        0x020
#define SCAN_BIT        0x040

typedef struct {
    BYTE flag;
    BYTE ascii;
    BYTE scan;
} KEY_EVT;

#define isScanCode(k)   (k.flag & SCAN_BIT)
#define isLeftShift(k)  (k.flag & SHFL_BIT)
#define isRightShift(k) (k.flag & SHFR_BIT)
#define isLeftCtrl(k)   (k.flag & CNTL_BIT)
#define isRightCtrl(k)  (k.flag & CNTR_BIT)
#define isLeftAlt(k)    (k.flag & ALTL_BIT)
#define isRightAlt(k)   (k.flag & ALTR_BIT)

#define keyb_getchar()  keyb_getch(BLOCK)

extern char engMap[];
extern char itaMap[];

typedef struct keyb_parms {
  TASK_MODEL *tm;
  char  *keybmap;
  void  (*ctrlcfunc)(KEY_EVT *k);
} KEYB_PARMS;


#define KEYB_DEFAULT ((unsigned long)(-1))   /*+ used for default params +*/

#define BASE_KEYB {(TASK_MODEL *)KEYB_DEFAULT, \
                   (char*)KEYB_DEFAULT,        \
                   (void *)KEYB_DEFAULT}

#define keyb_default_parm(m)   (m).tm = (TASK_MODEL *) KEYB_DEFAULT, \
                               (m).keybmap = (char *) KEYB_DEFAULT,  \
                               (m).ctrlcfunc = (void *) KEYB_DEFAULT
#define keyb_def_map(s,m)      (s).keybmap=(m)
#define keyb_def_ctrlC(s,f)    (s).ctrlcfunc=(f)
#define keyb_def_task(s,m)     (s).tm=(TASK_MODEL *)(m)

int  KEYB_init(KEYB_PARMS *s);
int  keyb_getch(BYTE wait);
int  keyb_getcode(KEY_EVT *k, BYTE wait);
void keyb_hook(KEY_EVT k, void (*f)(KEY_EVT *k));
int  keyb_enable(void);
int  keyb_disable(void);
int  keyb_end(void);
void keyb_set_map(char *t);

#ifdef __cplusplus
};
#endif

#endif
