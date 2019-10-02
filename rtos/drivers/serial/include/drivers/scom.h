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

/*
 * Copyright (C) 2000 Gerardo Lamastra
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

#ifndef __SCOM_H__
#define __SCOM_H__

/* Number of available COM links */
#define COM_LINKS	4

#define COM1		0
#define COM2		1
#define COM3		2
#define COM4		3

/* These values identify interrupt type */
#define	RX_FULL		1
#define TX_EMPTY	2
#define LS_CHANGED	4
#define MS_CHANGED	8

/* This is set when the channel is open */
#define LINK_BUSY	128
/* This is set if the fast handler is installed */
#define FAST_INSTALLED	64

/* Register displacements */
#define	THR	0
#define RBR	0
#define IER	1
#define FCR	2
#define IIR	2
#define LCR	3
#define MCR	4
#define LSR	5
#define MSR	6
#define	SPad	7

/* Parity value */
#define NONE	0
#define ODD	1
#define EVEN	3

/* Used for decoding the IIR status */
extern const int IIRbits[];
#define DECODE(v)	IIRbits[((v >> 1) & 3)]
#define PENDIRQ(v)	!((v) & 1)

/* Bit setting macros */
#define bit_on(v,b)	v |= (b)
#define bit_off(v,b)	v &= (~(b))

int com_open(unsigned port, DWORD speed, BYTE parity, BYTE len, BYTE stop);
int com_close(unsigned port);
unsigned com_read(unsigned port, unsigned reg);
unsigned com_receive(unsigned port);
void com_write(unsigned port, unsigned reg, unsigned value);
unsigned com_send(unsigned port, BYTE b, unsigned wait);

#endif
