
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

#include <ll/i386/hw-data.h>
#include <ll/i386/hw-instr.h>
#include <drivers/gd.h>

void gd_getcolor(BYTE ind, BYTE *r, BYTE *g, BYTE *b)
{
    outp(0x3c7, ind);
    *r = inp(0x3c9);
    *g = inp(0x3c9);
    *b = inp(0x3c9);
}

void gd_setcolor(BYTE ind,BYTE r,BYTE g,BYTE b)
{
    outp(0x3c8, ind);
    outp(0x3c9, r);
    outp(0x3c9, g);
    outp(0x3c9, b);
}

void gd_getpalette(BYTE start, BYTE num, BYTE *pal)
{ 
    int i;

    for (i = start; i < start + num; i++) {
	outp(0x3c7, i);
	*pal++ = inp(0x3c9);
	*pal++ = inp(0x3c9);
	*pal++ = inp(0x3c9);
    }
}

void gd_setpalette(BYTE start, BYTE num, BYTE *pal)
{ 
    int i;

    for (i = start; i < start + num; i++) {
	outp(0x3c8, i);
	outp(0x3c9, *pal++);
	outp(0x3c9, *pal++);
	outp(0x3c9, *pal++);
    }
}
