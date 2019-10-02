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
 CVS :        $Id: keyb.c,v 1.4 2005/02/25 11:04:03 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/02/25 11:04:03 $
 ------------

 Author:      Giuseppe Lipari
 Start date:  19/6/96

 File:        Keyb.C
 Revision:    1.5.1

 Last update  : 14/Apr/1999

 This file contains :
   -- A fast handler that get the scan code from the key interface      
   and put it in a port (non blocking) ; Inside this handler we can't   
   make any primitive call; it runs with interrupt enabled, so that the 
   timer cannot be blocked. Its priority is 1 (from hardware).          
   -- A sporadic soft process that gets the data from the port and      
   tries to recognize any scan code. With the help of a table that varies
   from keyboad to keyboard (e.g. italian vs american) it puts the ascii
   code intoanother port, with a byte that indicates the pression of any
   control key (Shift, Ctrl, Alt). I tried to make the initialization   
   of this table as standard as possible, so that one has to change only
   the include file with all the codes                                  
   -- Some functions (exported) to initialize the interface and get the 
   ascii code

   Revision 1.4 (Gerardo) Added support for italian keyboard; required
   	to add a new status variable rightalt to cope with italian keybs
	assignment! Added the keyb_set_map function to select the keymaps
	and finally build the english & italian keymaps

   Revision 1.5 (Giorgi)

   Split the file into keyb.c and 8042.c:
   8042.c low level hardware interface
   keyb.c user interface to kerboard routines
   (8042.c is the "source" of data that keyb.c must interpretate)

   Modification made:
   -- fast handler remove (now in 8042.c)
   -- all function that make direct I/O with hardware now in 8042.c
   -- keyb_init() modified (to interfaces with 8042.c)
   -- key_sendKeyCmd() removed; key_sendCtrCmd() removed
   -- scanCode() & KeyProc modified (keyboard ACK are tested in KeyProc)
   -- actled() removed (now in 8042.c with name 8042_keyboardleds); all
      reference changed
   -- Key_1 port removed (comunication in handled by functions in 8042.c)
   -- added functions to enable/disable keyboard

   PS: the modification are marked with "MG"

   Revision 1.5.1 (Giorgi)

   Modification to compile on 3.2.0
   -- added wcet time to all task
   -- RECEIVE port now are STREAM port

**/

/*
 * Copyright (C) 2000 Paolo Gai
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

//#define __KEYB_DEBUG__ 1

//#include <string.h>
//#include <stdlib.h>
//#include <cons.h>

#include <kernel/kern.h>
#include <signal.h>
#include <hartport/hartport/hartport.h>

#include <drivers/keyb.h>
#include <drivers/keycode.h>
#include "8042.h"

char engMap[] = {
  '1','2','3','4','5','6','7','8','9','0',
  '!','@','#','$','%','^','&','*','(',')',
  '-','_','=','+','[','{',']','}',';',':','\'','\"','`','~','/','?',',',
  '<','.','>','\\','|',' ',' ',8,8,9,9,27,27,13,13,24,
  '1','2','3','4','5','6','7','8','9','0','.','+','*','/','-','+','*','-',
  'a','A','b','B','c','C','d','D','e','E','f','F','g','G','h','H','i',
  'I','j','J','k','K','l','L','m','M','n','N','o','O','p','P','q','Q',
  'r','R','s','S','t','T','u','U','v','V','w','W','x','X','y','Y','z','Z',
  /*
    These are the strange keys used in the italian keyboard
    When you press Alt-Right to obtain [,], @ or #. They are left
    unbind with English Keyboard
    */
  ' ',' ',' ',' '
};

char itaMap[] = {
  '1','2','3','4','5','6','7','8','9','0',
  '!','\"','ú','$','%','&','/','(',')','=',
  '\'','?','ç','^','ä','Ç','+','*','ï','á','Ö','¯','\\','|','<','_',',',
  ':','.',';','ó','ı',' ',' ',8,8,9,9,27,27,13,13,24,
  '1','2','3','4','5','6','7','8','9','0','.','+','*','/','-','+','*','-',
  'a','A','b','B','c','C','d','D','e','E','f','F','g','G','h','H','i',
  'I','j','J','k','K','l','L','m','M','n','N','o','O','p','P','q','Q',
  'r','R','s','S','t','T','u','U','v','V','w','W','x','X','y','Y','z','Z',
  /*
    These are the strange keys used in the italian keyboard
    When you press Alt-Right to obtain [,], @ or #
    */
  '[',']','@','#'
};

#define MAX_KEY_EXC     25

static struct keyexctable {
  KEY_EVT evt;
  void (*func)(KEY_EVT *k);
} keyExcTable[MAX_KEY_EXC];

static int lastExc;

/* the following tables contains the ascii code corresponding to the    */
/* scan codes, with shift & no shift...                                 */
static char keyTable[NUM_OF_KEY];
static char keyShiftTable[NUM_OF_KEY];
static char keyRightAltTable[NUM_OF_KEY];

static char *actualMap = engMap;

/* Status variables */
static BYTE e0 = FALSE;
static BYTE sh = FALSE;
static BYTE rightalt = FALSE;
static BYTE cps = FALSE;
static BYTE numlock = TRUE;
static BYTE scrolllock = FALSE;
static int keyb_installed = FALSE;

/* Keyboard Flags (if shift alt or control are pressed...); */
static BYTE keyFlag = 0x00;

/* Communication port between the handler & the extern process  */
/* MG: port not needed! */
//static PORT keyHandPort, keyProcPort;

/* Communication port between the extern process & the user     */
static PORT pkeyPort, ukeyPort;

/* This function get a scan code, return an ascii code and set          */
/* the status vars in order to handle the special keys of the AT keyboard */
/* (I write this code several time !!). When it finds a special code    */
/* (Insert, Del, Up, Down, ecc) it return a code with the first byte    */
/* at ffh and the second byte with the scan code, while normally it     */
/* return only the ascii code                                           */
WORD scanCode(BYTE c)
{
  /* if it's an ack, advice the actLed routine */
  /* MG: test removed (it's in "KeyProc") */
  //if (c == 0xfa) actLed(0);

  /* If the previous key was an 0xe0...       */
  if (e0) {
    /* throw away the pairs 0xe0-0x2a and 0xe0-0xaa */
    if (c == 0x2a || c == 0xaa) {
      e0 = FALSE;
      return 0;
    }
    /* throw away the pairs 0xe0-0x36 and 0xe0-0xb6 */
    if (c == 0x36 || c == 0xb6) {
      e0 = FALSE;
      return 0;
    }
    /* if it's a break code ... */
    else if (c & 0x80) {
      e0 = FALSE;
      /* Right Alt */
      if (c == (KEY_ALTL | 0x80)) {
	keyFlag = keyFlag & (!ALTR_BIT);
	rightalt = FALSE;
      }
      /* Right Ctrl */
      else if (c == (KEY_CTRLL | 0x80)) {
	keyFlag = keyFlag & (!CNTR_BIT);
      }
      return 0;
    }
    else {
      e0 = FALSE;
      /* Right Alt */
      if (c == KEY_ALTL) {
	keyFlag |= ALTR_BIT ;
	rightalt = TRUE;
	return 0;
      }
      /* Right Ctrl */
      else if (c == KEY_CTRLL) {
	keyFlag |= CNTR_BIT ;
	return 0;
      }
      /* Numeric keypad */
      else if (c == PAD_SCRLOCK) return keyTable[c];
      else if (c == KEY_ENT) return keyTable[c];
      /* simply return scan code */
      else return(0xff00 | c);
    }
  }
  else {
    /* it's an 0xe0, we must remember */
    if (c == 0xe0) {
      e0 = TRUE;
      return 0;
    }
    else if (c == KEY_SHL || c == KEY_SHR) {
      /* it's the shift : adjust flags */
      sh = TRUE;
      if (c == KEY_SHL) keyFlag = keyFlag | SHFL_BIT ;
      else if (c == KEY_SHR) keyFlag = keyFlag | SHFR_BIT ;
      return 0;
    }
    else if (c == (KEY_SHL | 0x80) || c == (KEY_SHR | 0x80)) {
      /* it's the shift break code */
      sh = FALSE;
      if (c == (KEY_SHL | 0x80)) keyFlag = keyFlag & (!SHFL_BIT);
      else if (c == (KEY_SHR | 0x80)) keyFlag = keyFlag & (!SHFR_BIT);
      return 0;
    }
    else if (c == KEY_SCRLOCK) {
      if (scrolllock == TRUE) scrolllock = FALSE;
      else scrolllock = TRUE;
      /* light the scroll lock led */
      /* MG: changed */
      C8042_keyboardleds(numlock,cps,scrolllock);
      return 0;
    }
  }

  /* Hovewer ...*/
  if (c == KEY_CPSLOCK) {
    if (cps == TRUE) cps = FALSE;
    else cps = TRUE;	
    /* MG: changed */
    C8042_keyboardleds(numlock,cps,scrolllock);
    return 0;
  }
  if (c == PAD_NUMLOCK) {
    if (numlock == TRUE) numlock = FALSE;
    else numlock = TRUE;
    /* MG: changed */
    C8042_keyboardleds(numlock,cps,scrolllock);
    return 0;
  }
  if (c == KEY_CTRLL) {
    keyFlag = keyFlag | CNTL_BIT ;
    return 0;
  }
  else if (c == KEY_ALTL) {
    keyFlag = keyFlag | ALTL_BIT ;
    return 0;
  }
  else if (c == (KEY_CTRLL | 0x80)) {
    keyFlag = keyFlag & (!CNTL_BIT);
    return 0;
  }
  else if (c == (KEY_ALTL | 0x80)) {
    keyFlag = keyFlag & (!ALTL_BIT);
    return 0;
  }
  else if (c & 0x80) {
    return 0;
  }
  else if ((c == PAD_INS) || (c == PAD_END) || (c == PAD_DEL) ||
	   (c == PAD_DOWN) || (c == PAD_PGDW) || (c == PAD_LEFT) ||
	   (c == PAD_5) || (c == PAD_RIGHT) || (c == PAD_HOME) ||
	   (c == PAD_UP) || (c == PAD_PGUP)) {
    if (numlock || sh) return keyShiftTable[c];
    else return (0xff00 | c);
  }
  else if (cps && 
	   (((c >= KEY_Q) && (c <= KEY_P)) ||
	    ((c >= KEY_A) && (c <= KEY_L)) ||
	    ((c >= KEY_Z) && (c <= KEY_M))) ) {
    if (sh) return keyTable[c];
    else return keyShiftTable[c];
  }
  else if (sh) return keyShiftTable[c];
  else if (rightalt) return keyRightAltTable[c];
  else return keyTable[c];
}

TASK keyProc(void)
{
  WORD code;
  BYTE dato,found;
  KEY_EVT dt;
  int i,res;
    
  while (1) {
//    kern_printf("K");
    /* MG: used C8042_keyboardget() */
    res=C8042_keyboardget(&dato,NON_BLOCK); 
    if (res) {
      code = scanCode(dato);
      if (code != 0) {
	/* if it's a scan code , set the right bit ...*/
	if (code & 0xff00) dt.flag = (keyFlag | SCAN_BIT);
	/* ... else simply send the keyFlag status*/
	else dt.flag = keyFlag;
	dt.ascii = (BYTE)(code & 0x00FF);
	dt.scan  = dato;
	found = FALSE;
	for (i = 0; i < lastExc; i++)
	  if ((keyExcTable[i].evt.scan == dt.scan) && (keyExcTable[i].evt.flag == dt.flag)) {
	    keyExcTable[i].func(&dt);
	    found = TRUE;
	  }
	/* when the port is full, data is lost */
	if (!found) port_send(pkeyPort,(BYTE *)(&dt),NON_BLOCK);
      }
    }
    task_endcycle();
  }
}

void keyb_set_map(char *t)
{
  actualMap = t;
    
  keyTable[KEY_1] = actualMap[0];
  keyTable[KEY_2] = actualMap[1];
  keyTable[KEY_3] = actualMap[2];
  keyTable[KEY_4] = actualMap[3];
  keyTable[KEY_5] = actualMap[4];
  keyTable[KEY_6] = actualMap[5];
  keyTable[KEY_7] = actualMap[6];
  keyTable[KEY_8] = actualMap[7];
  keyTable[KEY_9] = actualMap[8];
  keyTable[KEY_0] = actualMap[9];

  keyShiftTable[KEY_1] = actualMap[10];
  keyShiftTable[KEY_2] = actualMap[11];
  keyShiftTable[KEY_3] = actualMap[12];
  keyShiftTable[KEY_4] = actualMap[13];
  keyShiftTable[KEY_5] = actualMap[14];
  keyShiftTable[KEY_6] = actualMap[15];
  keyShiftTable[KEY_7] = actualMap[16];
  keyShiftTable[KEY_8] = actualMap[17];
  keyShiftTable[KEY_9] = actualMap[18];
  keyShiftTable[KEY_0] = actualMap[19];
    
  keyTable[KEY_SUB] = actualMap[20];
  keyShiftTable[KEY_SUB] = actualMap[21];
  keyTable[KEY_PLUS] = actualMap[22];
  keyShiftTable[KEY_PLUS] = actualMap[23];
  keyTable[KEY_BRL] = actualMap[24];
  keyShiftTable[KEY_BRL] = actualMap[25];
  keyTable[KEY_BRR] = actualMap[26];
  keyShiftTable[KEY_BRR] = actualMap[27];
  keyTable[KEY_COL] = actualMap[28];
  keyShiftTable[KEY_COL] = actualMap[29];
  keyTable[KEY_API] = actualMap[30];
  keyShiftTable[KEY_API] = actualMap[31];
  keyTable[KEY_TIL] = actualMap[32];
  keyShiftTable[KEY_TIL] = actualMap[33];
  keyTable[KEY_SLH] = actualMap[34];
  keyShiftTable[KEY_SLH] = actualMap[35];
  keyTable[KEY_LT] = actualMap[36];
  keyShiftTable[KEY_LT] = actualMap[37];
  keyTable[KEY_GT] = actualMap[38];
  keyShiftTable[KEY_GT] = actualMap[39];
    
  keyTable[KEY_BSL] = actualMap[40];
  keyShiftTable[KEY_BSL] = actualMap[41];
  keyTable[KEY_SPC] = actualMap[42];
  keyShiftTable[KEY_SPC] = actualMap[43];

  keyTable[KEY_BKS] = actualMap[44];
  keyShiftTable[KEY_BKS] = actualMap[45];
  keyTable[KEY_TAB] = actualMap[46];
  keyShiftTable[KEY_TAB] = actualMap[470];
  keyTable[KEY_ESC] = actualMap[48];
  keyShiftTable[KEY_ESC] = actualMap[49];
  keyTable[KEY_ENT] = actualMap[50];
  keyShiftTable[KEY_ENT] = actualMap[51];
   
  keyShiftTable[KEY_DEL] = actualMap[52];
  keyShiftTable[PAD_END] = actualMap[53];
  keyShiftTable[PAD_DOWN] = actualMap[54];
  keyShiftTable[PAD_PGDW] = actualMap[55];
  keyShiftTable[PAD_LEFT] = actualMap[56];
  keyShiftTable[PAD_5] = actualMap[57];
  keyShiftTable[PAD_RIGHT] = actualMap[58];
  keyShiftTable[PAD_HOME] = actualMap[59];
  keyShiftTable[PAD_UP] = actualMap[60];
  keyShiftTable[PAD_PGUP] = actualMap[61];
  keyShiftTable[PAD_INS] = actualMap[62];
  keyShiftTable[PAD_DEL] = actualMap[63];

  keyTable[PAD_PLUS] = actualMap[64];
  keyTable[PAD_AST] = actualMap[65];
  keyTable[PAD_SCRLOCK] = actualMap[66];
  keyTable[PAD_SUB] = actualMap[67];

  keyShiftTable[PAD_PLUS] = actualMap[68];
  keyShiftTable[PAD_AST] = actualMap[69];
  keyShiftTable[PAD_SUB] = actualMap[70];
    
  keyTable[KEY_A] = actualMap[71];
  keyShiftTable[KEY_A] = actualMap[72];
  keyTable[KEY_B] = actualMap[73];
  keyShiftTable[KEY_B] = actualMap[74];
  keyTable[KEY_C] = actualMap[75];
  keyShiftTable[KEY_C] = actualMap[76];
  keyTable[KEY_D] = actualMap[77];
  keyShiftTable[KEY_D] = actualMap[78];
  keyTable[KEY_E] = actualMap[79];
  keyShiftTable[KEY_E] = actualMap[80];
  keyTable[KEY_F] = actualMap[81];
  keyShiftTable[KEY_F] = actualMap[82];
  keyTable[KEY_G] = actualMap[83];
  keyShiftTable[KEY_G] = actualMap[84];
  keyTable[KEY_H] = actualMap[85];
  keyShiftTable[KEY_H] = actualMap[86];
  keyTable[KEY_I] = actualMap[87];
  keyShiftTable[KEY_I] =  actualMap[88];
  keyTable[KEY_J] = actualMap[89];
  keyShiftTable[KEY_J] = actualMap[90];
  keyTable[KEY_K] = actualMap[91];
  keyShiftTable[KEY_K] = actualMap[82];
  keyTable[KEY_L] = actualMap[93];
  keyShiftTable[KEY_L] = actualMap[94];
  keyTable[KEY_M] = actualMap[95];
  keyShiftTable[KEY_M] = actualMap[96];
  keyTable[KEY_N] = actualMap[97];
  keyShiftTable[KEY_N] = actualMap[98];
  keyTable[KEY_O] = actualMap[99];
  keyShiftTable[KEY_O] = actualMap[100];
  keyTable[KEY_P] = actualMap[101];
  keyShiftTable[KEY_P] = actualMap[102];
  keyTable[KEY_Q] = actualMap[103];
  keyShiftTable[KEY_Q] = actualMap[104];
  keyTable[KEY_R] = actualMap[105];
  keyShiftTable[KEY_R] = actualMap[106];
  keyTable[KEY_S] = actualMap[107];
  keyShiftTable[KEY_S] = actualMap[108];
  keyTable[KEY_T] = actualMap[109];
  keyShiftTable[KEY_T] = actualMap[110];
  keyTable[KEY_U] = actualMap[111];
  keyShiftTable[KEY_U] = actualMap[112];
  keyTable[KEY_V] = actualMap[113];
  keyShiftTable[KEY_V] = actualMap[114];
  keyTable[KEY_W] = actualMap[115];
  keyShiftTable[KEY_W] = actualMap[116];
  keyTable[KEY_X] = actualMap[117];
  keyShiftTable[KEY_X] = actualMap[118];
  keyTable[KEY_Y] = actualMap[119];
  keyShiftTable[KEY_Y] = actualMap[120];
  keyTable[KEY_Z] = actualMap[121];
  keyShiftTable[KEY_Z] = actualMap[122];

  keyRightAltTable[KEY_BRL] = actualMap[123];
  keyRightAltTable[KEY_BRR] = actualMap[124];
  keyRightAltTable[KEY_COL] = actualMap[125];
  keyRightAltTable[KEY_API] = actualMap[126];
}

/* default function called on ctrl-c */
void default_ctrlChandler(KEY_EVT *k)
{
  set_active_page(0);
  set_visual_page(0);
  cputs("Ctrl-C pressed!\n");
  exit(1);
}

/* This is the interface to application */
/* Initialize keyboard driver */
/* MG: this function return: */
/*  0  - OK */
/*  <0 - error code */

/* TRUE when the 8042 keyboard initialization done */
static int init8042flag=FALSE;


/* keyboard task PID */
static PID keybpid;

int KEYB_init(KEYB_PARMS *s)
{
  KEYB_PARMS kparms=BASE_KEYB;
  WORD i;
  int status=0;

  SOFT_TASK_MODEL base_m;
  TASK_MODEL *m;

  if (keyb_installed) return 0;
  
  for (i = 0; i < NUM_OF_KEY; i++) {
    keyTable[i] = 0;
    keyShiftTable[i] = 0;
  }

  /* if a NULL is passed */
  if (s==NULL) s=&kparms;

  /* keymap */
  if (s->keybmap==(char*)KEYB_DEFAULT) s->keybmap=engMap;
  keyb_set_map(s->keybmap);
	
  /* MG: Key_1 port not needed! */
  //keyHandPort = port_create("Key_1",1,50,RECEIVE,WRITE);
  //keyProcPort = port_connect("Key_1",1,RECEIVE,READ);

  /* MG: changed RECEIVE to STREAM port - added a failure test */
  pkeyPort = port_create("KeybPort",3,20,STREAM,WRITE);
  if (pkeyPort==-1) return -2;
  ukeyPort = port_connect("KeybPort",3,STREAM,READ);
  if (ukeyPort==-1) {
    port_delete(pkeyPort);
    return -3;
  }

  /* remove all key pressed handlers */
  lastExc = 0;
  /* and add a ctrl-c handler if requested */
  if (s->ctrlcfunc == (void*)KEYB_DEFAULT)
    s->ctrlcfunc=(void*)default_ctrlChandler;
  if (s->ctrlcfunc!=NULL) {
    KEY_EVT emerg;
    emerg.ascii = 'c';
    emerg.scan = KEY_C;
    emerg.flag = CNTL_BIT;
    keyb_hook(emerg,s->ctrlcfunc);
    emerg.flag = CNTR_BIT;
    keyb_hook(emerg,s->ctrlcfunc);
  }


  /* keyb task */
  if (s->tm == (TASK_MODEL *)KEYB_DEFAULT) {
    soft_task_default_model(base_m);
    soft_task_def_wcet(base_m,2000);
    soft_task_def_met(base_m,800);
    soft_task_def_period(base_m,25000);
    soft_task_def_system(base_m);
    soft_task_def_nokill(base_m);
    soft_task_def_aperiodic(base_m);
    m = (TASK_MODEL *)&base_m;
  }
  else
    m = s->tm;

  //cprintf("keyb task: %li %li %li %li\n",
  //	    (long)s->pclass,(long)APERIODIC,(long)s->dline,(long)s->wcet);

  keybpid = task_create("KeyTask", keyProc, m, NULL);
  if (keybpid==-1) {
    port_delete(pkeyPort);
    port_delete(ukeyPort);
    return -1;
  }

  /* MG: 8042 keyboard controller initialization */
  if (!init8042flag) {
    status=C8042_keyboardinit(keybpid);
    if (status) {
      port_delete(pkeyPort);
      port_delete(ukeyPort);
      task_kill(keybpid);
      return -4;
    }
    init8042flag=TRUE;
  } else {
    C8042_keyboardenable();
  }

  /* MG: and keyboard led management */
  C8042_keyboardleds(numlock,cps,scrolllock);

  keyb_installed = TRUE;
  return status;
}

/* KEYB module cleanup 
 * (must be called if it's needed to re-call keyb_init() with other parameters)
 * -1 -> ERROR
 */
int keyb_end(void)
{
  if (!keyb_installed) return -1;
  C8042_keyboarddisable();
  task_kill(keybpid);
  port_delete(pkeyPort);
  port_delete(ukeyPort);
  return 0;
}

/* Function that returns the ascii code */
int keyb_getch(BYTE wait)
{
  KEY_EVT c;
  BYTE fl;

  fl = port_receive(ukeyPort,&c,wait);
  if (fl && !isScanCode(c)) return (c.ascii);
  else return 0;
}

/* Function that returns a structure containing the flags status, the ascii
   code, and the scan code */
int keyb_getcode(KEY_EVT *k,BYTE wait)
{
  return(port_receive(ukeyPort,(BYTE *)(k),wait));
}

void keyb_hook(KEY_EVT k, void (*f)(KEY_EVT *k))
{
  if (lastExc >= MAX_KEY_EXC) return;
  keyExcTable[lastExc].evt = k;
  keyExcTable[lastExc++].func = f;
  return;
}

/* MG: this function disable the keyboard */
int keyb_disable(void)
{
  return C8042_keyboarddisable();
}

/* MG: this function enable the keyboard */
int keyb_enable(void)
{
  return C8042_keyboardenable();
}
