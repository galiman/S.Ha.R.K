/* Project:     HARTIK 3.0 Sound Library                        */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        5/12/1997                                       */

/* File: 	Blaster.C					*/
/* Revision:    3.0						*/

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

/* Sound Blaster 16 low-level register access functions */
#include <kernel/kern.h>
#include "sbio.h"

/* Write in a SB16 mixer register */
void sbmixer_write(WORD base, BYTE index, BYTE c)
{
    ll_out(base + MIXERREGISTER, index);
    ll_out(base + MIXERDATA, c);
}

/* Read a byte from a SB16 mixer register */
BYTE sbmixer_read(WORD base, BYTE index)
{
    ll_out(base + MIXERREGISTER, index);
    return(ll_in(base + MIXERDATA));
}

/* Find a SB16 in the system: return the base I/O address or 0 if not found */
WORD sb_probe (void)
{
    WORD base;
    int done = 0;

    base = 0x0200;
    while ((!done) && (base < 0x0270)) {
	base += 0x10;
	done = sbdsp_reset(base);
    }
    if (done == 0) {
	base = 0;
    }
    return base;
}

/* Reset the SB16 DSP: return 1 if OK, 0 if fail */
BYTE sbdsp_reset(WORD base)
{
    int x;

    ll_in(base + DATAAVAILABLE);
    ll_out(base + RESET,0x01);
    ll_in(base + RESET);
    ll_in(base + RESET);
    ll_in(base + RESET);
    ll_in(base + RESET);
    ll_out(base + RESET,0x00);
    for(x = 0; x < 100; x++) {
	if(ll_in(base + DATAAVAILABLE)&0x80) {
	    if(ll_in(base + READDATA) == 0xAA) break;
	}
    }
    if(x == 100)
	return 0;
    else return 1;
}

/* Write in SB16 DSP register */
void sbdsp_write (WORD base, BYTE c )
{
    while(ll_in(base + WRITEBUFFERSTATUS)&0x80);
    ll_out(base + WRITEBUFFERSTATUS,c);
}

/* Read a byte from SB16 DSP register */
BYTE sbdsp_read (WORD base)
{
    while(!(ll_in(base + DATAAVAILABLE)&0x80));
    return(ll_in(base + READDATA));
}
