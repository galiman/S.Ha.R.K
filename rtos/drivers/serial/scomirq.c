/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Michele Cirinei
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * Copyright (C) 2000 Michele Cirinei
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
 */

#include <kernel/kern.h>
#include <drivers/scomirq.h>

/****************************
 Variables
****************************/
static unsigned com_irq[] = {COM1_IRQ, COM2_IRQ, COM3_IRQ, COM4_IRQ};

struct SCom_Access_Type SCom_Access[4];

unsigned SCom_Error[4];

struct TX_data_t{
    int ptr;
    int len;
    BYTE buf[100];
};

static struct TX_data_t SCom_TX_data[4];

/*****************************
* Functions
*****************************/

void RBRF_handler(unsigned port);

void THRE_handler(unsigned port);

void LSR_handler(unsigned port);

void com_send_msg(unsigned port, BYTE len, BYTE *m);

void com_irq_sel(int no)
{
    BYTE b, v;
    unsigned i;

    for(i=0;i<4;i++)
      if (com_irq[i] == no) break;

    while(PENDIRQ(v = com_read(i, IIR))) {

      b = DECODE(v);

      switch (b)
      {
	case LS_CHANGED: 
	    LSR_handler(i);
	break;
	    
        case RX_FULL:
	    RBRF_handler(i);
	break;
	
        case TX_EMPTY:
    	    THRE_handler(i);
        break;
      }

    }

}	

void com_irq_enable(unsigned port, unsigned irq)
{
    SYS_FLAGS f;

    f = kern_fsave();

    com_read(port, LSR);
    com_read(port, IIR);
    com_write(port, IER, com_read(port, IER) | irq); /* Enable  irq */

    kern_frestore(f);

}

void com_irq_disable(unsigned port, unsigned irq)
{

    SYS_FLAGS f;
                                                                                                                             
    f = kern_fsave();

    com_read(port, LSR);
    com_read(port, IIR);
    com_write(port, IER, com_read(port, IER) & ~irq); /* Disable irq */

    kern_frestore(f);

}

void com_close_irq(unsigned port)
{
    SYS_FLAGS f;
                                                                                                                             
    f = kern_fsave();

    com_write(port, IER, 0);
    com_read(port, LSR);
    com_read(port, IIR);

    handler_remove(com_irq[port]);

    kern_frestore(f);

}

void com_init_irq(unsigned port) 
{ 
    SYS_FLAGS f;

    f = kern_fsave();

    SCom_Error[port] = 0;
    handler_set(com_irq[port], com_irq_sel, TRUE, NIL, NULL);
    com_irq_disable(port, ALL_IRQ);	/* Disable interrupts */

    kern_frestore(f);

}

void com_set_functions(unsigned port, void (*confirm)(unsigned port, BYTE msg_status),void (*indication)(unsigned port, BYTE data))
{
    SYS_FLAGS f;

    f = kern_fsave();

    SCom_Access[port].confirm    = confirm;
    SCom_Access[port].request    = com_send_msg;
    SCom_Access[port].indication = indication;

    kern_frestore(f);

}

void com_send_msg(unsigned port, BYTE len, BYTE *m)
{
    SYS_FLAGS f;

    f = kern_fsave();

    /* Transmit first byte of message  */
    SCom_TX_data[port].ptr = 0;
    SCom_TX_data[port].len = len;
    if (len < 100) {
      memcpy(SCom_TX_data[port].buf, m, len);
      com_write(port, THR, SCom_TX_data[port].buf[SCom_TX_data[port].ptr++]);
    }

    kern_frestore(f);

}

__inline__ void com_irq_send(unsigned port, BYTE len, BYTE *m) {

  com_send_msg(port, len, m);

}

void THRE_handler (unsigned port) /* Transmit Data Register Ready for next byte */
{
    if (SCom_TX_data[port].ptr < SCom_TX_data[port].len) 
	com_write(port, THR, SCom_TX_data[port].buf[SCom_TX_data[port].ptr++]);
    else SCom_Access[port].confirm(port, COM_OK);
}

void RBRF_handler (unsigned port) /* Receive Data Register Full */
{
    SCom_Access[port].indication(port, com_read(port, RBR));
}

void LSR_handler (unsigned port)    /* break = 8, frame = 4, parity = 2, RBR overrun = 1*/
{
    SCom_Error[port] = (com_read(port, LSR) & (bit4|bit3|bit2|bit1) ) >> 1;
    SCom_Access[port].confirm(port, COM_ERROR);
}

