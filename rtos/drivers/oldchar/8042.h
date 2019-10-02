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
 CVS :        $Id: 8042.h,v 1.1 2003/03/24 10:54:16 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 10:54:16 $
 ------------

 8042.h

 Interface between high level drivers
 and the 8042 keyboard and PS/2 mouse controller

 Revision:    1.0
 Last update: 22/Mar/1999

 Created by Massimiliano Giorgi, modified by Paolo Gai to support the
 kernel 4.0.0

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


#ifndef __8042_H__
#define __8042_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define C8042_KEYBOARDIRQ  1
#define C8042_PS2IRQ      12

/*
 * init, enable, disable, get data for the keyboard
 */
int C8042_keyboardinit(PID task);
int C8042_keyboarddisable(void);
int C8042_keyboardenable(void);
int C8042_keyboardget(BYTE *data,BYTE access);

/*
 * init, enable, disable, get data for the ps/2 mouse (auxiliary port)
 */
int C8042_auxinit(PID task);
int C8042_auxportdisable(void);
int C8042_auxportenable(void);
int C8042_auxget(BYTE *data,BYTE access);
int C8042_auxend(void);

/* test if there is a PS/2 mouse on the aux port */
int C8042_ps2mousepresent(void);

/*
 * keyboard led controll
 */
void C8042_keyboardleds(BYTE numlock, BYTE cps, BYTE scrolllock);

/*
 *
 * from Linux OS kernel 2.2.2
 * (see GPL license)
 * with some modification
 */

/*
 *	Keyboard Controller Registers
 */

#define KBD_STATUS_REG		0x64	/* Status register (R) */
#define KBD_CNTL_REG		0x64	/* Controller command register (W) */
#define KBD_DATA_REG		0x60	/* Keyboard data register (R/W) */

/*
 *	Keyboard Controller Commands
 */

#define KBD_CCMD_READ_MODE	0x20	/* Read mode bits */
#define KBD_CCMD_WRITE_MODE	0x60	/* Write mode bits */
#define KBD_CCMD_GET_VERSION	0xA1	/* Get controller version */
#define KBD_CCMD_MOUSE_DISABLE	0xA7	/* Disable mouse interface */
#define KBD_CCMD_MOUSE_ENABLE	0xA8	/* Enable mouse interface */
#define KBD_CCMD_TEST_MOUSE	0xA9	/* Mouse interface test */
#define KBD_CCMD_SELF_TEST	0xAA	/* Controller self test */
#define KBD_CCMD_KBD_TEST	0xAB	/* Keyboard interface test */
#define KBD_CCMD_KBD_DISABLE	0xAD	/* Keyboard interface disable */
#define KBD_CCMD_KBD_ENABLE	0xAE	/* Keyboard interface enable */
#define KBD_CCMD_WRITE_AUX_OBUF	0xD3    /* Write to output buffer as if
					   initiated by the auxiliary device */
#define KBD_CCMD_WRITE_MOUSE	0xD4	/* Write the following byte to the mouse */

/* added by me (MG) */

#define KBD_CCMD_GET_MODE       0xCA    /* Get controller mode (AT or PS/2) */

#define KBD_CREPLY_PS2MODE      0x01    /* PS2 mode */
#define KBD_CREPLY_ATMODE       0x00    /* AT mode */
#define KBD_CREPLY_GETMODEMASK  0x01    /* mask value for CCMD_GET_MODE */

/*
 *	Keyboard Commands
 */

#define KBD_CMD_SET_LEDS	0xED	/* Set keyboard leds */
#define KBD_CMD_SET_RATE	0xF3	/* Set typematic rate */
#define KBD_CMD_ENABLE		0xF4	/* Enable scanning */
#define KBD_CMD_DISABLE		0xF5	/* Disable scanning */
#define KBD_CMD_RESET		0xFF	/* Reset */

/* added by me (MG) */
#define KBD_CMD_SET_ALL_TYPEMATIC 0xF7  /* Set all key typematic */

#define KBD_LED_NUMLOCK         0x02
#define KBD_LED_CAPSLOCK        0x04
#define KBD_LED_SCROLLLOCK      0x01

#define KBD_LED_MASK            0x07

/*
 *	Keyboard Replies
 */

#define KBD_REPLY_POR		0xAA	/* Power on reset */
#define KBD_REPLY_ACK		0xFA	/* Command ACK */
#define KBD_REPLY_RESEND	0xFE	/* Command NACK, send the cmd again */

/* added by me (MG) */
#define KBD_REPLY_KISTOK        0x00    /* Keyboard Interface Self Test OK */
#define KBD_REPLY_CSTOK         0x55    /* Controller self test OK */
#define KBD_REPLY_MISTOK        0x00    /* Mouse Interface Self Test OK */

/*
 *	Status Register Bits
 */

#define KBD_STAT_OBF 		0x01	/* Keyboard output buffer full */
#define KBD_STAT_IBF 		0x02	/* Keyboard input buffer full */
#define KBD_STAT_SELFTEST	0x04	/* Self test successful */
#define KBD_STAT_CMD		0x08	/* Last write was a command write (0=data) */
#define KBD_STAT_UNLOCKED	0x10	/* Zero if keyboard locked */
#define KBD_STAT_MOUSE_OBF	0x20	/* Mouse output buffer full */
#define KBD_STAT_GTO 		0x40	/* General receive/xmit timeout */
#define KBD_STAT_PERR 		0x80	/* Parity error */

#define AUX_STAT_OBF (KBD_STAT_OBF | KBD_STAT_MOUSE_OBF)

/*
 *	Controller Mode Register Bits
 */

#define KBD_MODE_KBD_INT	0x01	/* Keyboard data generate IRQ1 */
#define KBD_MODE_MOUSE_INT	0x02	/* Mouse data generate IRQ12 */
#define KBD_MODE_SYS 		0x04	/* The system flag (?) */
#define KBD_MODE_NO_KEYLOCK	0x08	/* The keylock doesn't affect the keyboard if set */
#define KBD_MODE_DISABLE_KBD	0x10	/* Disable keyboard interface */
#define KBD_MODE_DISABLE_MOUSE	0x20	/* Disable mouse interface */
#define KBD_MODE_KCC 		0x40	/* Scan code conversion to PC format */
#define KBD_MODE_RFU		0x80

/*
 *	Mouse Commands
 */

#define AUX_SET_RES		0xE8	/* Set resolution */
#define AUX_SET_SCALE11		0xE6	/* Set 1:1 scaling */
#define AUX_SET_SCALE21		0xE7	/* Set 2:1 scaling */
#define AUX_GET_SCALE		0xE9	/* Get scaling factor */
#define AUX_SET_STREAM		0xEA	/* Set stream mode */
#define AUX_SET_SAMPLE		0xF3	/* Set sample rate */
#define AUX_ENABLE_DEV		0xF4	/* Enable aux device */
#define AUX_DISABLE_DEV		0xF5	/* Disable aux device */
#define AUX_RESET		0xFF	/* Reset aux device */
#define AUX_ACK			0xFA	/* Command byte ACK. */

#define AUX_BUF_SIZE		2048	/* This might be better divisible by
					   three to make overruns stay in sync
					   but then the read function would
					   need
					   */

__END_DECLS
#endif




