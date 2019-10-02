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
 *   Tullio Facchinetti  <tullio.facchinetti@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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

/* Serial communication device */
/* This implementation is capable of handling 4 distinct COM ports */
/* The COM port settings are the standard PC settings:		   */
/* PORT		ADDRESS		IRQ				   */
/*   1		 0x3F8		 4			  	   */
/*   2		 0x2F8		 3	 		  	   */
/*   3		 0x3E8		 4			  	   */
/*   4		 0x2E8		 2			  	   */

/* By Massy
 * I have modified the fast handler routines to support serial
 * mouse better (see below)
 */

#include <kernel/kern.h>
#include <drivers/scom.h>

/* Base address for each standard COM link */
static unsigned com_base[] = {0x03F8,0x02F8,0x03E8,0x02E8};

/* Used for decoding the IIR status */
const int IIRbits[] = {MS_CHANGED,TX_EMPTY,RX_FULL,LS_CHANGED};

int serial_initialized[4] = {0, 0, 0, 0};

/** 
 * Register level access functions.
 */
unsigned com_read(unsigned port, unsigned reg) {
    unsigned b;
    if (port > 3 || reg > 7 || (!serial_initialized[port])) return(0);
    b = ll_in(com_base[port]+reg);
    return(b);
}

void com_write(unsigned port, unsigned reg, unsigned value) {
    if (port > 3 || reg > 7 || (!serial_initialized[port])) return;
    ll_out(com_base[port]+reg,value);
}

/**
 * Write the character "b" to the port "port".
 * If wait is 0 then a busy port makes the fuction to return immediately.
 * If wait is 1 loops until the port become free.
 * Return 0 if the byte has been sent. Return 1 otherwise.
 */
unsigned com_send(unsigned port, BYTE b, unsigned wait) {
    if (wait) {
      while ((com_read(port,LSR) & 32) == 0);
    } else {
      if ((com_read(port,LSR) & 32) == 0) return(1);
    }
    com_write(port,THR,b);
    return(0);
}

/**
 * Read a character from the port "port".
 * Polls the port until a character is ready to be read.
 */
unsigned com_receive(unsigned port) {
    while ((com_read(port,LSR) & 1) == 0);
    return(com_read(port,RBR));
}

/** 
 * Initialize a serial channel.
 * Do not do anything if the port has been already initialized.
 */
int com_open(unsigned port,DWORD speed,BYTE parity,BYTE len,BYTE stop) {
    unsigned long div,b_mask;
    SYS_FLAGS f;        

    f = kern_fsave();
    
    if (serial_initialized[port]) {
      kern_frestore(f);
      return(0);
    }

		serial_initialized[port] = 1;

    /* Now set up the serial link */
    b_mask = (parity & 3) * 8 + ((stop - 1) & 1) * 4 + ((len - 5) & 3);
    div = 115200L / speed;
    /* Clear serial interrupt enable register */
    com_write(port,IER,0);
    /* Empty input buffer */
    com_read(port,RBR);
    /* Activate DLAB bit for speed setting */
    com_write(port,LCR,0x80);
    /* Load baud divisor */
    com_write(port,0,div & 0x00FF);
    div >>= 8;
    com_write(port,1,div & 0x00FF);
    /* Load control word (parity,stop bit,bit len) */
    com_write(port,LCR,b_mask);
    /* Attiva OUT1 & OUT2 */
    com_write(port,MCR,0x0C);

    kern_frestore(f);

    return(0);

}

/**
 * Close port channel & release the server.
 */
int com_close(unsigned port)
{    
    SYS_FLAGS f;

    f = kern_fsave();

    com_write(port,IER,0);
    com_read(port,IIR);
    com_read(port,RBR);
    
    serial_initialized[port] = 0;
    
    kern_frestore(f);

    return(0);
}

