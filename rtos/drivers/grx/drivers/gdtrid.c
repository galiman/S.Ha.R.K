
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
#include <ll/i386/mem.h>
#include <ll/i386/cons.h>
/*
#include <ll/i386/x-dos.h>
#include <ll/i386/x-dosmem.h>
*/
#include <ll/sys/ll/ll-func.h>

#include "trident.h"
#include "chips.h"

enum {TRIDENT_8800, TRIDENT_8900, TRIDENT_9000, TRIDENT_9200, TRIDENT_9420,
    TRIDENT_9680,
    TRIDENT_UNKNOWN
};

static const char *trident_chipname[] =
{"8800", "8900", "9000", "9200", "9420", "9680",
 "Unknown Trident chip"};

int trident_chiptype;
int trident_flags;
int trident_memory;

static int trident_init(int par1, int par2)
{
    BYTE value;

    outp(0x3D4, 0x1f);
	/* this should detect up to 2M memory now */
    trident_memory = (inp(0x3D5) & 0x07) * 256 + 256;

    outp(0x3C4, 0x0B);
    outp(0x3C5, 0);

    value = inp(0x3C5);

    switch(value) {
	case 0x03:
	case 0x04:
	    trident_chiptype = TRIDENT_8900;
	    break;
	case 0x13:
	case 0x23:
	case 0x93:
	    trident_chiptype = TRIDENT_9000;
	    break;
	case 0x53:
	case 0x83:
	    trident_chiptype = TRIDENT_9200;
	    break;
	case 0x73:
	    trident_chiptype = TRIDENT_9420;
	    break;
	case 0xD3:
	    trident_chiptype = TRIDENT_9680;
	default:
	    trident_chiptype = TRIDENT_UNKNOWN;
/*	    trident_chiptype = TRIDENT_8800;  */
/*	    trident_chiptype = TRIDENT_9440;  */
	    break;
    }
    return 1;
}


int trident_test(void)
{
    BYTE old_value, old_misc_value, value, misc_value, power_up_value;
    int res;

    res = CHIP_NOT_FOUND;

    outp(0x3CE, 0x06);
    old_misc_value = inp(0x3CF);
    misc_value = old_misc_value & 0x03;
    misc_value |= 0x04;
    outp(0x3CF, misc_value);
    
    outp(0x3C4, 0x0E);
    old_value = inp(0x3C5);

    outp(0x3C5, 0x00);

    value = inp(0x3C5) & 0x0F;

    outp(0x3C5, old_value);

    outp(0x3CE, 0x06);
    outp(0x3CF, old_misc_value);

    if (value >= 0x02) {
	res = CHIP_FOUND;
    }else {
	outp(0x3C4, 0x0F);
	outp(0x3C5, 0x00);

	power_up_value = inp(0x3C5);

	if (power_up_value != 0) {
	    res = CHIP_FOUND;
	}
    }
    if (res == CHIP_FOUND) {
			trident_init(0,0);
			if (trident_chiptype != TRIDENT_UNKNOWN) {
					return 1;
			} else {
					return -1;
			}
    }
    return res;
}

void trident_showinfo()
{
    cprintf("\t Using Trident driver (%s, %dK).\n",
    	trident_chipname[trident_chiptype], trident_memory);
}

void trident_setpage(int page)
{
    WORD app;
    BYTE bb;

    bb = page;
    if ((bb & 0x02) == 0x02) {
	bb = (bb & 0xFD);
    } else {
	bb = (bb | 0x02);
    }

/*    outp(0x03C4, 0x0b);
    outp(0x03C5, inp(0x03C5));
    inp(0x3C5);	*/

/*    bb = page ^ 0x02;*/

    app = 0x0E || (bb << 8);
    
outp(0x3C4, 0x0E);
outp(0x3C5, bb);
/*    outw(0x3C4, app);*/

/*
    outp(0x03C4, 0x0b);
    outp(0x03C5, inp(0x03C5));
    inp(0x3C5);	

    outp(0x03C4, 0x0e);
    outp(0x03C5, page ^ 0x02);
*/
}

DWORD trident_getmem(void)
{
    return trident_memory * 1024;
}
