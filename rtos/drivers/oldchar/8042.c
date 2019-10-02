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
 CVS :        $Id: 8042.c,v 1.7 2005/05/10 17:20:10 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.7 $
 Last update: $Date: 2005/05/10 17:20:10 $
 ------------

 8042.h

 Interface between high level drivers
 and the 8042 keyboard and PS/2 mouse controller

 Revision:    1.0
 Last update: 22/Mar/1999

 Revision 1.0

 This file contains :
 -- A fast handler that get the scan code from the key interface
 -- A fast handler for the ps2 mouse port
 -- Some functions (exported) to initialize the interface
 -- Some functions (exported) to enable/disable and get data from
    the keyboard or ps/2 mouse

 Created by Massimiliano Giorgi, modified by Paolo Gai to support the
 kernel 4.0.0

 30/Apr/2003
 -- added __KEYB_NO_INIT_CHECK__ to support some strange behavior of our 
 portable Toshiba 1900. That is, it seems that two checks does not work
 on that machine. Disabling them, all works (at least when executed from
 the eXtender

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

// This define skips some initialization checks on the keyboard driver
//that fails when using Toshiba Satellite 1900
//#define __KEYB_NO_INIT_CHECK__

/* if defined: show on first two lines BYTE received from cotroller */
//#define __DEBUG_8042__ 1

//#define __DEBUG_INIT__ 1

/* if defined: show messages during exit */
//#define __TRACE_EXIT__ 1

/* if defined: trace ps/2 enable/disable */
//#define __TRACE_PS2ENDIS__ 1

//#include <config.h>
//#include <string.h>
//#include <stdlib.h>
//#include <cons.h>

//#include "vm.h"
#include <kernel/kern.h>
//#include "exc.h"
//#include "port.h"

#include "8042.h"

//#ifdef __SAFEXBIOS__
//#include "x86/xsys.h"
//#endif

#ifdef __DEBUG_8042__
static char debug_buffer[256];
static void debug_output(char *s,int v)
{
  if (v>=0) cprintf("%s: %d\n",s,v);
  else      cprintf("%s: -%d\n",s,-v);
}
#endif

/* there is a ps2mouse in the system? */
static BYTE ps2mouse_present=0;

/* has been activated? */ 
static BYTE ps2mouse_active=0;

/*
 * Wait (to write)
 * 
 * return:
 * 0    -> input buffer empty (OK to write)
 * else -> input buffer full
 */
static int C8042_wait_towrite(void)
{
  unsigned long counter;
  BYTE          status;
 
  counter=0;
  while (counter++<0xffffff) {
    status=ll_in(KBD_STATUS_REG);
    /*if ((!(status&KBD_STAT_OBF))&&(!(status&KBD_STAT_IBF))) return 0;*/
    if(!(status&KBD_STAT_IBF)) return 0;
  }
  return -1;
}

/*
 * Wait (to read)
 *
 * return:
 * 0    -> output buffer full (OK to read)
 * else -> output buffer empty
 */
static int C8042_wait_toread(void)
{
  unsigned long counter;
  BYTE          status;

  counter=0;
  while (counter++<0xffffff) {
    status=ll_in(KBD_STATUS_REG);
    if (status&KBD_STAT_OBF) return 0;
  }
  return -1;
}

/*
 * Send data to the keyboard
 *
 * return:
 * 0    -> OK
 * else -> "I can't send!"
 */
static __inline__ int C8042_sendkeybdata(BYTE data)
{
    if (!C8042_wait_towrite()) {
      ll_out(KBD_DATA_REG,data);
      return 0;
    }
    return -1;
}

/*
 * Send data to the controller
 *
 * return:
 * 0    -> OK
 * else -> "I can't send!"
 */
static __inline__ int C8042_sendctrldata(BYTE data)
{
    if (!C8042_wait_towrite()) {
      ll_out(KBD_CNTL_REG,data);
      return 0;
    }
    return -1;
}

/*
 * Send data to PS/2 mouse
 *
 * return:
 * 0    -> OK
 * else -> error
 */
static __inline__ int C8042_sendauxdata(BYTE data)
{
  if (C8042_sendctrldata(KBD_CCMD_WRITE_MOUSE)) return -1;
  if (C8042_sendkeybdata(data)) return -2;
  return 0;
}

/*
 * Read data from the conroller
 *
 * return:
 * else -> BYTE read
 * -1   -> "I can't read!"
 */
static __inline__ int C8042_readdata(void)
{
    if (!C8042_wait_toread()) return (int)ll_in(KBD_DATA_REG);
    return -1;
}

/*
 * Read aux data from the controller
 *
 * return:
 * else -> BYTE read
 * -1   -> "I can't read!"
 */
static __inline__ int C8042_readauxdata(void)
{
  BYTE status;

  if (!C8042_wait_toread()) {
    status=ll_in(KBD_STATUS_REG);
    if (status&KBD_STAT_MOUSE_OBF) return ll_in(KBD_DATA_REG);
  }
  return -1;
}

/*
 * Clear output buffer
 *
 */
static void C8042_clearkeyboardbuffer(void)
{
  unsigned long counter;
  BYTE          status;
 
  counter=0;
  while (counter++<0xffff) {
    status=ll_in(KBD_STATUS_REG);
    if (!(status&KBD_STAT_OBF)) return;
    ll_in(KBD_DATA_REG);
  }
}

/*
 * Send data and receive ACK (with the keyboard)
 *
 * return:
 * 0  -> OK
 * -1 -> can't send command
 * -2 -> can't read reply
 * -3 -> reply unknown
 * -4 -> max retries
 */
static int C8042_keyboardhandshake(BYTE cmd)
{
  int maxretries=50;
  int c;

  while (maxretries-->0) {
    if (C8042_sendkeybdata(cmd)) return -1;
    c=C8042_readdata();
    if (c==-1) return -2;
    if (c==KBD_REPLY_ACK) return 0;
    if (c!=KBD_REPLY_RESEND) return -3;
  }
  return -4;
}

/*
 * Send data and receive ACK (with PS/2 mouse)
 */
static int C8042_auxhandshake(BYTE cmd)
{
  int c;

  if (C8042_sendauxdata(cmd)) return -1;
  c=C8042_readauxdata();
  if (c<0) return c;
  if (c!=AUX_ACK) return -2;
  return 0;
}

/*
 * Values for controller "command register"
 *
 * CMDREG_NORMAL:
 *   normal operation (PS/2 mouse interrupts disabled)
 *
 * CMDREG_PS2:
 *   keyboard and PS/2 mouse enabled
 */

#define CMDREG_NORMAL (KBD_MODE_KBD_INT |\
		       KBD_MODE_SYS |\
		       KBD_MODE_DISABLE_MOUSE |\
		       KBD_MODE_KCC \
		       )

#define CMDREG_PS2    (KBD_MODE_KCC |\
		       KBD_MODE_SYS |\
		       KBD_MODE_MOUSE_INT |\
		       KBD_MODE_KBD_INT \
		       )

/*
 * Some default values
 */

/* default typematic keyboard rate (maximum - I hope) */
/* (I don't know how to coding this information) */
#define DEFAULT_KEYBRATE 0x00

/* 100 samples/sec */
#define DEFAULT_PS2_SAMPLESEC 100

/* 8 count per mm */
#define DEFAULT_PS2_RES       3

/* 2:1 scaling */
#define DEFAULT_PS2_SCALE AUX_SET_SCALE21    

/*
 *
 * Controller initialization
 *
 */

/*
 * Reset keyboard
 *
 * Reset the keyboard and set it to a known state
 * (Must be called after exiting from S.Ha.R.K.)
 *
 * return:
 * 0    -> OK
 * else -> Error
 */
     
#ifdef __TRACE_EXIT__
#define trace(x) kern_printf("%s\n",(x))
#else
#define trace(x)
#endif
     
static int C8042_reset(void)
{
  int c=0;
  int retries=16;

  trace("8042 reset START");
  
  /* Reset keyboard */
  /* If there is not retries some machines hang */
  while (retries-->0) {
    trace("Sending keyboard reset command...");
    c=C8042_keyboardhandshake(KBD_CMD_RESET);
    if (c) return -3;
    trace("Waiting reply...");
    c=C8042_readdata();
    if (c==KBD_REPLY_POR) break;
    trace("Reset fail!!!\n");
  }
  if (c!=KBD_REPLY_POR) return -4;
  
  /* Disable keyboard (for the following phase) */
  trace("Disabling keyboard...");
  c=C8042_keyboardhandshake(KBD_CMD_DISABLE);
  if (c) return -5;

  /* Initialize controller "command register" */
  trace("Command register writing...");
  C8042_sendctrldata(KBD_CCMD_WRITE_MODE);
  trace("Sending command register value...");
  C8042_sendkeybdata(CMDREG_NORMAL);
  
  /* Enable keyboard */
  trace("Enabling keyboard...");
  c=C8042_keyboardhandshake(KBD_CMD_ENABLE);
  if (c) return -6;

  trace("8042 reset END");
  return 0;
}

/*
 * 8042 restore
 *
 * This function must be called after system sthudown
 * to proper restore keyboard handling by DOS
 */

#ifdef __TRACE_EXIT__
#define trace(x) ll_printf("%s\n",(x))
#else
#define trace(x)
#endif

void C8042_restore(void)
{
  int c;
  SYS_FLAGS f;

  trace("Restore in progress");

  /* disable interrupts */
  f=kern_fsave();

  /* Disable the PS/2 mouse port (if present) */
  if (ps2mouse_active) {
    trace("Deactivating aux port (ps/2)...");
    C8042_auxportdisable();
  }

  /* Reset */
  trace("Reseting start...");
  C8042_reset();
  trace("Resetting end");

  /* Set keyboard typematic rate */
  trace("Sending keyboard rate command...");
  c=C8042_keyboardhandshake(KBD_CMD_SET_RATE);
  if (c) {
    kern_frestore(f);
    return;
  }
  trace("Sending rate...");
  c=C8042_keyboardhandshake(DEFAULT_KEYBRATE);
  if (c) {
    kern_frestore(f);
    return;
  }

  /* restore interrupts status */
  kern_frestore(f);
  trace("Restore end");
}

/*
 * 8042 initialization
 *
 * test controller,set default values and search a PS/2 mouse
 *
 * return:
 * 0  -> OK
 * <0 -> error (-index into  initialize_msg[])
 *
 * set ps2mouse_present if a mouse is present!
 */

#if defined(__DEBUG_8042__)||defined(__DEBUG_INIT__)
static char *initialize_msg[]={
  /* 0 */
  "ok",
  /* 1..8 */
  "controller self test failed",
  "keyboard interface self test failed",
  "keyboard reset error",
  "keyboard reset (not POR)",
  "keyboard disable (not ACK)",
  "keyboard enable (not ACK)",
  "keyboard typematic rate code (not ACK)",
  "keyboard typematic rate (not ACK)",
  /* 9.. */
  "controller self test (can't send command)",
  "controller interface self test (can't send command)",
  "can't enable keyboard",
  "controller aux interface self test (can't send command)",
  "ps/2 mouse interface self test failed",
  /* 14.. */
  "can't send command GET_MODE",
  "can't read GET_MODE reply",
  "error sending CODE for mouse",
  "error sending data to mouse",
  /* 18 */
  "can't disable ps/2 mouse",
  "can't enable ps/2 mouse",
  /* 20 */
  "PS/2 mouse not present"
};
#endif

static int C8042_initialize(void)
{
  int c;

  /* Disable controller and clear keyboard output buffer */
  C8042_clearkeyboardbuffer();
  C8042_sendctrldata(KBD_CCMD_KBD_DISABLE);
  C8042_clearkeyboardbuffer();

  /* Controller self test */
  c=C8042_sendctrldata(KBD_CCMD_SELF_TEST);
  if (c) return -9;
  c=C8042_readdata();
  if (c!=KBD_REPLY_CSTOK) return -1;

  /* Controller to keyboard interface test */
  c=C8042_sendctrldata(KBD_CCMD_KBD_TEST);
  if (c) return -10;
  c=C8042_readdata();
#ifndef __KEYB_NO_INIT_CHECK__
  if (c!=KBD_REPLY_KISTOK) return -2;
#endif

  /* Enable data from/to keyboard */
  c=C8042_sendctrldata(KBD_CCMD_KBD_ENABLE);
  if (c) return -11;

  /*
   * Reset keyboard
   */
  c=C8042_reset();
  if (c) return c;

  /* Set keyboard typematic rate */
  c=C8042_keyboardhandshake(KBD_CMD_SET_RATE);
  if (c) return -7;
  c=C8042_keyboardhandshake(DEFAULT_KEYBRATE);
  if (c) return -8;

  /*
   * PS/2 mouse
   */
  ps2mouse_present=0;

  /* Test keyboard controller type */
  /*
  This cause a crash on some systems
  c=C8042_sendctrldata(KBD_CCMD_GET_MODE);
  if (c) return -14;
  c=C8042_readdata();
  if (c==-1) return -15;
  */
  /* if it isn't in PS/2 mode... exit */
  //if ((c&KBD_CREPLY_GETMODEMASK)!=KBD_CREPLY_PS2MODE) return 0;

  /* Mouse interface test */
  c=C8042_sendctrldata(KBD_CCMD_TEST_MOUSE);
  if (c) return -12;
  c=C8042_readdata();
#ifndef __KEYB_NO_INIT_CHECK__
  if (c!=KBD_REPLY_MISTOK) return -13;
#endif

  /* Enable mouse interface */
  c=C8042_sendctrldata(KBD_CCMD_MOUSE_ENABLE);
  if (c) return -11;

  /* Detect if a mouse is connected to PS/2 port */
  /* Send a dummy value to the mouse and wait a reply */
  /* (this code is from Linux) */
  c=C8042_sendctrldata(KBD_CCMD_WRITE_AUX_OBUF);
  if (c) return -16;
  c=C8042_sendkeybdata(0x5a);
  if (c) return -17;
  {
    int loop=0xffff;
    int v;
    
    while (loop--) {
      v=ll_in(KBD_STATUS_REG);
      if (v&KBD_STAT_OBF) {
	c=ll_in(KBD_DATA_REG);
	if (v&KBD_STAT_MOUSE_OBF&&c==0x5a) {
	  ps2mouse_present=1; /* => mouse present */
	  break;
	}
      }
    }
  }  

  /* Disable mouse interface */
  c=C8042_sendctrldata(KBD_CCMD_MOUSE_DISABLE);
  if (c) return -11;

  /* if there is a PS2 mouse... */
  if (ps2mouse_present) {
    int v;

    ps2mouse_present=0;

    /* Enable PS/2 mouse port (to initialize) */
    c=C8042_sendctrldata(KBD_CCMD_MOUSE_ENABLE);
    if (c) return -18;

    /* sample/sec */
    v=C8042_auxhandshake(AUX_SET_SAMPLE);
    v+=C8042_auxhandshake(DEFAULT_PS2_SAMPLESEC);
    /* resolution */
    v+=C8042_auxhandshake(AUX_SET_RES);
    v+=C8042_auxhandshake(DEFAULT_PS2_RES);
    /* scaling */
    v+=C8042_auxhandshake(AUX_SET_SCALE21);

    /* Disable PS/2 mouse port */
    c=C8042_sendctrldata(KBD_CCMD_MOUSE_DISABLE);
    if (c) return -19;

    if (!v) ps2mouse_present=1;
  }

  return 0;
}

#ifdef __SAFEXBIOS__
static void C8042_safe_after(void);
static void C8042_safe_before(void);
#endif

/* 8042 initialization */
static int C8042_init(void)
{
  static int initstatus=-1;
  static int first=1;

  if (!first) return initstatus;

  cli();
  initstatus=C8042_initialize();
  first=0;
  sti();

  #if defined(__DEBUG_8042__)||defined(__DEBUG_INIT__)
  #if defined(__DEBUG_INIT__)&&!defined(__DEBUG_8042__)
  if (initstatus) {
    #endif
    cprintf("8042 init : %d (%s)\n",-initstatus,initialize_msg[-initstatus]);
    cprintf("8042 mouse: %d (%s)\n",ps2mouse_present,
	    ps2mouse_present?"PS/2 mouse present":"PS/2 mouse not found"
	    );
    #if defined(__DEBUG_INIT)&&!defined(__DEBUG_8042__)
  }
  #endif
  #endif

  if (initstatus) {
    cli();
    C8042_reset();
    sti();
  }

  #ifdef __SAFEXBIOS__
  if (!initstatus) {
     xbios_regfun(C8042_safe_before,-1);
     xbios_regfun(C8042_safe_after,1);
  }
  #endif
   
  return initstatus;
}

/*
 *
 * Keyboard led manager
 *
 * a 1 swicth led on, a 0 off
 */

/* if are waiting an ack from the keyboard */
static int ackStatusLed=0;

/* last keyboard led values */
static unsigned ledsValue=0;

void C8042_keyboardleds(BYTE numlock, BYTE capslock, BYTE scrolllock)
{

  if (capslock) ledsValue = ledsValue | KBD_LED_CAPSLOCK;
  else ledsValue = ledsValue & (KBD_LED_CAPSLOCK ^ KBD_LED_MASK);
  if (numlock) ledsValue = ledsValue | KBD_LED_NUMLOCK;
  else ledsValue = ledsValue & (KBD_LED_NUMLOCK ^ KBD_LED_MASK);
  if (scrolllock) ledsValue = ledsValue | KBD_LED_SCROLLLOCK;
  else ledsValue = ledsValue & (KBD_LED_SCROLLLOCK ^ KBD_LED_MASK);
   
  /* Disable keyboard */
  C8042_sendkeybdata(KBD_CMD_DISABLE);
  /* send the command to the keyboard to light led */
  C8042_sendkeybdata(KBD_CMD_SET_LEDS);
  /* wait for the ack */
  ackStatusLed++;
}

/*
 *
 * ACK managers
 *
 */

/* numebr of ACK that we are waiting for */
static volatile int ackPending=0;

/*
 * this procedure is called when an ACK is received from the
 * keyboard
 * return:
 * 0  : OK (ACK processed)
 * -1 : no ACK waiting (return this ACK code to the caller)
 */

static int C8042_keyboardACK(void)
{
  /* if some ACK are waiting... */

  if (ackPending) {
    ackPending--;
    return 0;
  }

  /* if ACK for Led commands... */

  if (ackStatusLed == 1){
    /* send it to keyboard */
    C8042_sendkeybdata(ledsValue);
    /* wait for the ack */
    ackStatusLed++;
    return 0;
  }
  else if (ackStatusLed == 2) {
    /* ok we enable keyboard, and begin again */
    ackStatusLed = 0;
    C8042_sendkeybdata(KBD_CMD_ENABLE);
    return 0;
  }

  /* else .... nothing */
  return 1;
}

/*
 * this when an ACK is received from the PS/2 mouse
 * (return code: see C8042_keyboardACK)
 */

static volatile int auxackPending=0;

static int C8042_auxACK(void)
{
  if (auxackPending) {
    auxackPending--;
    return 0;
  }
  return -1;
}

/*
 *
 * Fast Handlers
 *
 */

/*
 * Fast keyboard handler
 */

/* keyboard buffer 
 * (without port, semaphores, ...)
 */

/* buffer size */
#define KBUFFERSIZE 256
/* buffer mask ( i=(i+1)&MASK is better than i=(i+1)%SIZE ) */
#define KBUFFERMASK 0xff
/* circular buffer */
static BYTE kbuffer[KBUFFERSIZE];
/* buffer pointers */
/* data is inserted to khead */
/* data is kept from ktail+1 */
/* (khead point to ktail+1 when buffer is empty) */
static unsigned ktail,khead;

/* keyboard fast handler */
static void keyb_handler(int dummy)
{
    BYTE data;
    SYS_FLAGS f;

    /* get data from the controller */
    data=ll_in(KBD_DATA_REG);
    
    /* insert into buffer */
    f=kern_fsave();
    if (ktail!=khead) {
      kbuffer[khead]=data;
      khead=(khead+1)&KBUFFERMASK;
    }
    kern_frestore(f);
}

/*
 * Fast PS/2 mouse handler
 */

/* buffer size */
#define ABUFFERSIZE 64
/* buffer mask */
#define ABUFFERMASK 0x3f
/* circula buffer */
static BYTE abuffer[ABUFFERSIZE];
/* buffer pointers */
static unsigned atail,ahead;

/* count bytes into a packet */
static int acount;
/* ps2server is activated when a packet is received */
static PID ps2server=NIL;

/* ps2 fast handler */
static void ps2mouse_handler(int dummy)
{
  BYTE data;
  SYS_FLAGS f;

  /* get data */ 
  data=ll_in(KBD_DATA_REG);

  /* check packet start or ACK */ 
  if (acount==0) {
     if (data==AUX_ACK) {
	auxackPending--;
	return;
     }
     if ((data&0xc0)!=0x00) return;
  }
  acount++;
  
  /* insert data into buffer */
  f=kern_fsave();
  if (atail!=ahead) {
    abuffer[ahead]=data;
    ahead=(ahead+1)&ABUFFERMASK;
  }
  else {
    ahead=(ahead-(acount-1)+ABUFFERSIZE)&ABUFFERMASK;      
    acount=0;
  }
  kern_frestore(f);

  /* if a packet is received activate the task */ 
  if (acount==3) {
    acount=0;
    task_activate(ps2server);
  }
}

/*
 *
 * Interface with high level drivers
 *
 */

/* keyboard initialization */
int C8042_keyboardinit(PID task)
{
  int status;

  /* initialize buffer variables */
  khead=1;
  ktail=0;
  
  /* hardware initialization */
  status=C8042_init();
  if (status) return status;

  /* set fast handler and task */
  handler_set(C8042_KEYBOARDIRQ, keyb_handler, TRUE, task, NULL);
  return 0;
}


/* PS/2 mouse port initialization */
int C8042_auxinit(PID task)
{
  int status;
  static int first=TRUE;

  /* initialize buffer variables */
  if (first) {
    ahead=1;
    atail=0;
    acount=0;
    first=FALSE;
  }
  ps2server=task;
  
  /* init hardware */
  status=C8042_init();
  if (status) return status;
  if (!ps2mouse_present) return -20;

  /* set fast handler and task */
  handler_set(C8042_PS2IRQ, ps2mouse_handler, TRUE, NIL, NULL);
  
  /* OK, now ps/2 mouse port is active! */
  ps2mouse_active=1;
  return 0;
}

/* PS/2 release resources */
int C8042_auxend(void)
{
  if (ps2mouse_active) C8042_auxportdisable();
  handler_remove(C8042_PS2IRQ);
  /* now ps/2 mouse port is disabled */
  ps2mouse_active=0;
  return 0;
}

/* test if there is a PS/2 mouse */
int C8042_ps2mousepresent(void)
{
  int status;

  status=C8042_init();
  if (status) return 0;

  return ps2mouse_present;
}

#ifdef __DEBUG_8042__

/* debug values for keyboadget() and auxget() */
#define YDEB     0
#define COLORDEB WHITE

static int keyx=2;
static int auxx=2;

#endif

/*
 * get data from the keyboard (primary port)
 *
 * it's follow the port_receive() semantic
 */
int C8042_keyboardget(BYTE *data,BYTE access)
{
  SYS_FLAGS f;
  

  f=kern_fsave();
  //if (khead!=(ktail+1)%KBUFFERMASK)  {	
    ktail=(ktail+1)&KBUFFERMASK;
    *data=kbuffer[ktail];
  //} 
  kern_frestore(f);
      
#ifdef __DEBUG_8042__
  /*
   * if debug...
   * show all data from the keyboard on YDEB line of the screen
   */
  {
    if (keyx+5>=80) {
      printf_xy(keyx,YDEB,COLORDEB," ");
      keyx=2;
    }
    if (keyx==2) printf_xy(0,YDEB,COLORDEB,"K ");
    if (*data==0) {
      /* why this if? seems to be a bug in ucvt() (the initial test)*/
      printf_xy(keyx,YDEB,COLORDEB,"00 > ");
    } else {
      printf_xy(keyx,YDEB,COLORDEB,"%-2x > ",(unsigned)*data);
    }
    keyx+=3;
  }
#endif

  /* check for ACK */
  if (*data==KBD_REPLY_ACK)
    return C8042_keyboardACK(); 
    
  return 1;  
}

/*
 * get data from PS/2 mouse port (auxiliary port)
 *
 * it's follows the port_receive() semantic
 */
int C8042_auxget(BYTE *data,BYTE access)
{
  SYS_FLAGS f;
  
  f=kern_fsave();
  atail=(atail+1)&ABUFFERMASK;
  *data++=abuffer[atail];
  atail=(atail+1)&ABUFFERMASK;
  *data++=abuffer[atail];
  atail=(atail+1)&ABUFFERMASK;
  *data++=abuffer[atail];    
  kern_frestore(f);
    
#ifdef __DEBUG_8042__
  /*
   * if debug...
   * show all data from the keyboard on YDEB line of the screen
   */
  {
    int i;
    for (i=-3;i<0;i++) {	
      if (auxx+5 >= 80) {
        printf_xy(auxx,YDEB+1,COLORDEB," ");
        auxx=2;
      }
      if (auxx==2) printf_xy(0,YDEB+1,COLORDEB,"M ");
      printf_xy(auxx,YDEB+1,COLORDEB,"%02x > ",(unsigned)*(data+i));
      auxx+=3;
    }
  }
#endif

  return 3; 
}

/*
 * Enable/Disable keyboard 
 */

int C8042_keyboarddisable(void)
{
  /* Disable keyboard */
  ackPending++;
  return C8042_sendkeybdata(KBD_CMD_DISABLE);
}

int C8042_keyboardenable(void)
{
  /* Enable keyboard */
  ackPending++;
  return C8042_sendkeybdata(KBD_CMD_ENABLE);
}

/*
 * Enable/Disable PS/2 mouse
 */

#ifdef __TRACE_PS2ENDIS__
#define trace(x) ll_printf("%s\n",(x))
#else
#define trace(x)
#endif

int C8042_auxportdisable(void)
{
  //int retries;
  SYS_FLAGS f;
  int c=0;

  trace("auxportdisable() START");

  /* DISABLE ps/2 mouse (not port)*/
  auxackPending++;
  C8042_sendauxdata(AUX_DISABLE_DEV);
  //retries=1000000;
  //while (auxackPending&&retries) {
  //  retries--;
  //}

  /* disable interrupts */
  trace("disabling interrupts");
  f=kern_fsave();

  /* Disable keyboard (for the following phase) */
  trace("disabling keyboard");
  c=C8042_keyboardhandshake(KBD_CMD_DISABLE);
  if (c) {
    kern_frestore(f);
    return -5;
  }

  /* Disable PS/2 mouse port */
  trace("disabling PS/2 mouse port");
  c+=C8042_sendctrldata(KBD_CCMD_MOUSE_DISABLE);
  
  /* Initialize controller "command register" */
  trace("senfing WRITE_MODE command");
  C8042_sendctrldata(KBD_CCMD_WRITE_MODE);
  trace("sending normal command mode");
  C8042_sendkeybdata(CMDREG_NORMAL);
  
  /* Enable keyboard */
  trace("enabling keyboard");
  c+=C8042_keyboardhandshake(KBD_CMD_ENABLE);

  /* restore interrupt mask */
  trace("enabling interrupts");
  kern_frestore(f);

  trace("auxportdisable() END");
  ps2mouse_active=0;
  return c;
}

int C8042_auxportenable(void)
{
  SYS_FLAGS f;
  int c;

  trace("auxportenabled() START");
  
  if (!ps2mouse_present) return -1;

  /* disable interrupts */
  trace("disabling interrupts");
  f=kern_fsave();
  
  /* Disable keyboard (for the following phase) */
  trace("disabling keyboard");
  c=C8042_keyboardhandshake(KBD_CMD_DISABLE);
  if (c) {
    kern_frestore(f);
    return -5;
  }
    
  /* Initialize controller "command register" */
  trace("senfing WRITE_MODE command");
  C8042_sendctrldata(KBD_CCMD_WRITE_MODE);
  trace("sending ps/2 command mode");
  C8042_sendkeybdata(CMDREG_PS2);

  /* Enable PS/2 mouse port */
  trace("enabling ps/2 mouse port");
  c=C8042_sendctrldata(KBD_CCMD_MOUSE_ENABLE);
  if (c) {
    kern_frestore(f);
    return -18;
  }

  /* Enable keyboard */
  trace("enabling keyboard");
  c=C8042_keyboardhandshake(KBD_CMD_ENABLE);
  if (c) {
    kern_frestore(f);
    return -37;
  }

  /* restore interrupt mask */
  trace("enabling interrupts");
  kern_frestore(f);
  
  /* Enable ps/2 mouse */
  /* PS: auxackPending is a share resource and shall be protected */
  /* by a semaphore */
  trace("enabling ps/2 mouse");
  auxackPending++;
  C8042_sendauxdata(AUX_ENABLE_DEV);

  trace("auxportenable() end");
  ps2mouse_active=1; 
  return 0;
}

/*
 * Safe procedure
 */

#ifdef __SAFEXBIOS__

static int ps2_flag;

static void C8042_safe_before(void)
{
   /* don't work */
   /*
   if (ps2mouse_active) {
     C8042_auxportdisable();
     ps2_flag=1;
   } else
     ps2_flag=0
   C8042_keyboarddisable();
   */
}

static void C8042_safe_after(void)
{
   /* don't work */
   /*
   BYTE status;
   SYS_FLAGS f;
   
   C8042_keyboardenable();
   if (ps2_flag) C8042_auxportenable();
   ps2_flag=0;
   */
}

#endif
