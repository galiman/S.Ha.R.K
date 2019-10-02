
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
#include <ll/i386/mem.h>
#include <ll/stdlib.h>

#include <drivers/gd.h>

#include "font.h"
#include "fun8.h"

#define fontaddr       0xffa6eL     /* indirizzo set caratteri         */
extern DWORD flbaddr;

void RdWin_256(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf)
{
    WORD b_bank, b_offset, display_ptr;
    int dx, len1, len2, bank_num, row, sx[12];
/*    int bank, enmem;*/
    DWORD end_of_line, display_offset;
    WORD bytes_per_row;
    LIN_ADDR p;

    bytes_per_row = gd_getbpr();
    p = buf;

    if (bytes_per_row == 1024) {
/*	ClrWin_1024(x1,y1,x2,y2,color);*/
	Seg_Off_256(x1, y1, bytes_per_row, &b_offset, &b_bank);
	dx = (x2 - x1) + 1;
	display_offset = b_offset;
	bank_num = b_bank;
	Load_Write_Bank_256(bank_num);
	row = y1;
	do {
	    display_ptr = display_offset;
/*	    Clr_Part(color, dx, display_ptr);*/
	    memcpy(p, (LIN_ADDR)(0xA0000 + display_ptr), dx);
	    p += dx;
	    display_offset += bytes_per_row;
	    if (display_offset >= 65536L) {
		bank_num++;
		Load_Write_Bank_256(bank_num);
		display_offset = x1;
	    }
	    row++;
	} while (row <= y2);
	Load_Write_Bank_256(0);
    } else {
	if (bytes_per_row == 640) {
	    sx[0] = 0;
	    sx[1] = 256;
	    sx[2] = 512;
	    sx[3] = 128;
	    sx[4] = 384;
	    sx[5] = 0;
	}
	if (bytes_per_row == 800) {
	    sx[0] = 0;
	    sx[1] = 736;
	    sx[2] = 672;
	    sx[3] = 608;
	    sx[4] = 544;
	    sx[5] = 480;
	    sx[6] = 416;
	    sx[7] = 352;
	}
	Seg_Off_256(x1, y1, bytes_per_row, &b_offset, &b_bank);
	dx = (x2 - x1) + 1;
	row = y1;
	display_offset = b_offset;
	display_ptr = display_offset;
	bank_num = b_bank;
	Load_Write_Bank_256(bank_num);
	do {
	    end_of_line = display_ptr + (x2 - x1);
	    if (end_of_line > 65536L) {
		len2 = (1 + end_of_line - 65536L);
		len1 = dx - len2;
/*		Clr_Part(color, length_1, display_ptr);*/
		memcpy(p, (LIN_ADDR)(0xA0000 + display_ptr), len1);
		p += len1;
		bank_num++;
		Load_Write_Bank_256(bank_num);
		display_ptr = 0;
/*		Clr_Part(color, length_2, display_ptr);*/
		memcpy(p, (LIN_ADDR)(0xA0000 + display_ptr), len2);
		p += len2;
		display_ptr = bytes_per_row - len1;
	    } else {
/*		Clr_Part(color, dx, display_ptr);*/
		memcpy(p, (LIN_ADDR)(0xA0000 + display_ptr), dx);
		p += dx;
		display_offset = display_ptr + bytes_per_row;
		if (display_offset > 65536L) {
		    bank_num++;
		    Load_Write_Bank_256(bank_num);
		    if (sx[bank_num] <= x1) {
			display_ptr = x1 - sx[bank_num];
		    } else {
			display_ptr = x1 + (bytes_per_row - sx[bank_num]);
		    }
		} else {
		    display_ptr += bytes_per_row;
		}
	    }
	    row++;
	} while (row <= y2);
	Load_Write_Bank_256(0);
    }
}

void WrWin_256(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf)
{
    WORD b_bank, b_offset, display_ptr;
    int dx, len1, len2, bank_num, row, sx[12];
/*    int bank, enmem;*/
    DWORD end_of_line, display_offset;
    WORD bytes_per_row;
    LIN_ADDR p;

    bytes_per_row = gd_getbpr();
    p = buf;

    if (bytes_per_row == 1024) {
/*	ClrWin_1024(x1,y1,x2,y2,color);*/
	Seg_Off_256(x1, y1, bytes_per_row, &b_offset, &b_bank);
	//dx = (x2 - x1);
	dx = (x2 - x1 + 1);
	display_offset = b_offset;
	bank_num = b_bank;
	Load_Write_Bank_256(bank_num);
	row = y1;
	do {
	    display_ptr = display_offset;
/*	    Clr_Part(color, dx, display_ptr);*/
	    memcpy((LIN_ADDR)(0xA0000 + display_ptr), p, dx);
	    p += dx;
	    display_offset += bytes_per_row;
	    if (display_offset >= 65536L) {
		bank_num++;
		Load_Write_Bank_256(bank_num);
		display_offset = x1;
	    }
	    row++;
	} while (row <= y2);
	Load_Write_Bank_256(0);
    } else {
	if (bytes_per_row == 640) {
	    sx[0] = 0;
	    sx[1] = 256;
	    sx[2] = 512;
	    sx[3] = 128;
	    sx[4] = 384;
	    sx[5] = 0;
	}
	if (bytes_per_row == 800) {
	    sx[0] = 0;
	    sx[1] = 736;
	    sx[2] = 672;
	    sx[3] = 608;
	    sx[4] = 544;
	    sx[5] = 480;
	    sx[6] = 416;
	    sx[7] = 352;
	}
	Seg_Off_256(x1, y1, bytes_per_row, &b_offset, &b_bank);
	dx = (x2 - x1 + 1);
	row = y1;
	display_offset = b_offset;
	display_ptr = display_offset;
	bank_num = b_bank;
	Load_Write_Bank_256(bank_num);
	do {
	    end_of_line = display_ptr + (x2 - x1);
//	    if (end_of_line >= 65536L) {
	    if (end_of_line > 65536L) {
		len2 = (1 + end_of_line - 65536L);
		len1 = dx - len2;
/*		Clr_Part(color, length_1, display_ptr);*/
		memcpy((LIN_ADDR)(0xA0000 + display_ptr), p, len1);
		p += len1;
		bank_num++;
		Load_Write_Bank_256(bank_num);
		display_ptr = 0;
/*		Clr_Part(color, length_2, display_ptr);*/
		memcpy((LIN_ADDR)(0xA0000 + display_ptr), p, len2);
		p += len2;
		display_ptr = bytes_per_row - len1;
	    } else {
/*		Clr_Part(color, dx, display_ptr);*/
		memcpy((LIN_ADDR)(0xA0000 + display_ptr), p, dx);
		p += dx;
		display_offset = display_ptr + bytes_per_row;
		if (display_offset > 65536L) {
		//if (display_offset >= 65536L) {
		    bank_num++;
		    Load_Write_Bank_256(bank_num);
		    if (sx[bank_num] <= x1) {
			display_ptr = x1 - sx[bank_num];
		    } else {
			display_ptr = x1 + (bytes_per_row - sx[bank_num]);
		    }
		} else {
		    display_ptr += bytes_per_row;
		}
	    }
	    row++;
	} while (row <= y2);
	Load_Write_Bank_256(0);
    }
}

void linGet8(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf)
{
    WORD bpr;
    LIN_ADDR addr, p;
    WORD dx, y;
    
    bpr = gd_getbpr();
    p = buf;
    addr = (void *)(flbaddr + x1 + bpr * y1);
    //dx = x2 - x1;
    dx = x2 - x1 +1;

    for (y = y1; y <= y2; y++) {
	memcpy(p, addr, dx);
	p += dx;
	addr += bpr;
    }
}

void linPut8(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf)
{
    WORD bpr;
    LIN_ADDR addr, p;
    WORD dx, y;
    
    bpr = gd_getbpr();
    p = buf;
    addr = (void *)(flbaddr + x1 + bpr * y1);
    dx = x2 - x1 +1;

    for (y = y1; y <= y2; y++) {
	memcpy(addr, p, dx);
	p += dx;
	addr += bpr;
    }
}

void ClrWin_256(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color)
{
    WORD b_bank, b_offset, display_ptr;
    int dx, len1, len2, bank_num, row, sx[12];
/*    int bank, enmem;*/
    DWORD end_of_line, display_offset;
    WORD bytes_per_row;

    bytes_per_row = gd_getbpr();

    if (bytes_per_row == 1024) {
/*	ClrWin_1024(x1,y1,x2,y2,color);*/
	Seg_Off_256(x1, y1, bytes_per_row, &b_offset, &b_bank);
	dx = (x2 - x1) + 1;
	display_offset = b_offset;
	bank_num = b_bank;
	Load_Write_Bank_256(bank_num);
	row = y1;
	do {
	    display_ptr = display_offset;
/*	    Clr_Part(color, dx, display_ptr);*/
	    memset((LIN_ADDR)(0xA0000 + display_ptr), color, dx);
	    display_offset += bytes_per_row;
	    if (display_offset >= 65536L) {
		bank_num++;
		Load_Write_Bank_256(bank_num);
		display_offset = x1;
	    }
	    row++;
	} while (row <= y2);
	Load_Write_Bank_256(0);
    } else {
	if (bytes_per_row == 640) {
	    sx[0] = 0;
	    sx[1] = 256;
	    sx[2] = 512;
	    sx[3] = 128;
	    sx[4] = 384;
	    sx[5] = 0;
	}
	if (bytes_per_row == 800) {
	    sx[0] = 0;
	    sx[1] = 736;
	    sx[2] = 672;
	    sx[3] = 608;
	    sx[4] = 544;
	    sx[5] = 480;
	    sx[6] = 416;
	    sx[7] = 352;
	}
	Seg_Off_256(x1, y1, bytes_per_row, &b_offset, &b_bank);
	dx = (x2 - x1) + 1;
	row = y1;
	display_offset = b_offset;
	display_ptr = display_offset;
	bank_num = b_bank;
	Load_Write_Bank_256(bank_num);
	do {
	    end_of_line = display_ptr + (x2 - x1);
	    if (end_of_line >= 65536L) {
		len2 = (1 + end_of_line - 65536L);
		len1 = dx - len2;
/*		Clr_Part(color, length_1, display_ptr);*/
		memset((LIN_ADDR)(0xA0000 + display_ptr), color, len1);
		bank_num++;
		Load_Write_Bank_256(bank_num);
		display_ptr = 0;
/*		Clr_Part(color, length_2, display_ptr);*/
		memset((LIN_ADDR)(0xA0000 + display_ptr), color, len2);
		display_ptr = bytes_per_row - len1;
	    } else {
/*		Clr_Part(color, dx, display_ptr);*/
		memset((LIN_ADDR)(0xA0000 + display_ptr), color, dx);
		display_offset = display_ptr + bytes_per_row;
		if (display_offset >= 65536L) {
		    bank_num++;
		    Load_Write_Bank_256(bank_num);
		    if (sx[bank_num] <= x1) {
			display_ptr = x1 - sx[bank_num];
		    } else {
			display_ptr = x1 + (bytes_per_row - sx[bank_num]);
		    }
		} else {
		    display_ptr += bytes_per_row;
		}
	    }
	    row++;
	} while (row <= y2);
	Load_Write_Bank_256(0);
    }
}

void linBox8(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color)
{
    WORD bpr;
    DWORD addr;
    WORD dx, y;
    
    bpr = gd_getbpr();
    addr = flbaddr + x1 + bpr * y1;
    dx = x2 - x1 + 1;

    for (y = y1; y <= y2; y++) {
	memset((LIN_ADDR)addr, color, dx);
	addr += bpr;
    }
}

void WrRect_256(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color)
{
    WORD bank, offs, ebank, eoffs;
    WORD bpr;
    WORD line_segment, len;
    WORD y;

    bpr = gd_getbpr();
    len = 1 + x2 - x1;
    Seg_Off_256(x2, y1, bpr, &eoffs, &ebank);
    Seg_Off_256(x1, y1, bpr, &offs, &bank);
    Load_Write_Bank_256(bank);
    if (bank == ebank) {
	memset((LIN_ADDR)(0xA0000 + offs), color, len);
    } else {
	line_segment = 65536L - offs;
	memset((LIN_ADDR)(0xA0000 + offs), color, line_segment);
	Load_Write_Bank_256(ebank);
	memset((LIN_ADDR)0xA0000, color, eoffs);
    }
    for (y = y1 + 1; y < y2 + 1; y++) {
	Seg_Off_256(x1, y, bpr, &offs, &bank);
	Load_Write_Bank_256(bank);
	lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
	Seg_Off_256(x2, y, bpr, &offs, &bank);
	Load_Write_Bank_256(bank);
	lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
    }
    Seg_Off_256(x2, y2, bpr, &eoffs, &ebank);
    Seg_Off_256(x1, y2, bpr, &offs, &bank);
    Load_Write_Bank_256(bank);
    if (bank == ebank) {
	memset((LIN_ADDR)(0xA0000 + offs), color, len);
    } else {
	line_segment = 65536L - offs;
	memset((LIN_ADDR)(0xA0000 + offs), color, line_segment);
	Load_Write_Bank_256(ebank);
	memset((LIN_ADDR)0xA0000, color, eoffs);
    }
}

void linRect8(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color)
{
    WORD bpr;
    DWORD addr;
    WORD dx, y;
    
    bpr = gd_getbpr();
    addr = flbaddr + x1 + bpr * y1;
    dx = x2 - x1;

    memset((LIN_ADDR)addr, color, dx+1);
    addr += bpr;

    for (y = y1 + 1; y <= y2 - 1; y++) {
	lmempokeb((LIN_ADDR)addr, color);
	lmempokeb((LIN_ADDR)(addr + dx), color);
	addr += bpr;
    }
    memset((LIN_ADDR)addr, color, dx+1);
}

void WrText_256(char *text, WORD x, WORD y, DWORD fg, DWORD bg)
{
    DWORD fp;
    WORD offs, bank;
    WORD bpr;
    int r, c, bits;

    bpr = gd_getbpr();
    while (*text) {
	fp = (DWORD)&(font_table[*(BYTE *)text][0]);
	for (r=0; r<8; r++) {
	    bits = lmempeekb((LIN_ADDR)(fp++));
	    for (c=0; c<8; c++)
		if (bits & (0x80>>c)) {
		    Seg_Off_256(x + c, y + r, bpr, &offs, &bank);
		    Load_Write_Bank_256(bank);
		    lmempokeb((LIN_ADDR)(0xA0000 + offs), fg);
		}
/*		    lmempokeb((LIN_ADDR)(addr + (y + r) * bpr + x + c), fg);*/
/*		    vbe_putpixel((x+c),(y+r),fg);*/
		else {
		    Seg_Off_256(x + c, y + r, bpr, &offs, &bank);
		    Load_Write_Bank_256(bank);
		    lmempokeb((LIN_ADDR)(0xA0000 + offs), bg);
		}
/*		    lmempokeb((LIN_ADDR)(addr + (y + r) * bpr + x + c), bg);*/
/*		    vbe_putpixel((x+c), (y+r), bg);*/
	}
	text++;
	x += 8;
    }
}

void linText8(char *text, WORD x, WORD y, DWORD fg, DWORD bg)
{
    DWORD fp, addr;
    WORD bpr;
    int r, c, bits;

    bpr = gd_getbpr();
    addr = gd_getflb();
    while (*text) {
	/*DOSSetAddr(fp, (fontaddr + (8 * *(BYTE *)text)));*/
	/*DOSSetAddr(fp, (fontaddr + (8 * *(BYTE *)text)));*/
	fp = (DWORD)&(font_table[*(BYTE *)text][0]);
	for (r=0; r<8; r++) {
	    bits = lmempeekb((LIN_ADDR)fp++);
	    for (c=0; c<8; c++)
		if (bits & (0x80>>c))
		    lmempokeb((LIN_ADDR)(addr + (y + r) * bpr + x + c), fg);
/*		    vbe_putpixel((x+c),(y+r),fg);*/
		else
		    lmempokeb((LIN_ADDR)(addr + (y + r) * bpr + x + c), bg);
/*		    vbe_putpixel((x+c), (y+r), bg);*/
	}
	text++;
	x += 8;
    }
}

void WrLine_256(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color)
{
    int dx, dy, incr1, incr2, d, x, y, xend, yend, yinc, xinc;
    WORD bpr;
    WORD bank, offs, ebank, eoffs;
    WORD line_segment, len;

    bpr = gd_getbpr();
    if (y2 == y1) {
	y = y1;
	if (x1 > x2) {
	    xend = x1;
	    x1 = x2;
	    x2 = xend;
	}
/*	WrHline_256(x1, x2, y1, color, bpr);	*/
	len = 1 + x2 - x1;
	Seg_Off_256(x2, y, bpr, &eoffs, &ebank);
	Seg_Off_256(x1, y, bpr, &offs, &bank);
	Load_Write_Bank_256(bank);
	if (bank == ebank) {
	    memset((LIN_ADDR)(0xA0000 + offs), color, len);
	} else {
	    line_segment = 65536L - offs;
	    memset((LIN_ADDR)(0xA0000 + offs), color, line_segment);
	    Load_Write_Bank_256(ebank);
	    memset((LIN_ADDR)0xA0000, color, eoffs);
	}
    } else {
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	if (dx >= dy) {
	    if (x1 > x2) {
		x = x2;
		y = y2;
		xend = x1;
		if (dy == 0) {
		    yinc = 0;
		} else {
		    if (y2 > y1) {
			yinc = -1;
		    } else {
			yinc = 1;
		    }
		}
	    } else {
		x = x1;
		y = y1;
		xend = x2;
		if (dy == 0) {
		    yinc = 0;
		} else {
		    if (y2 > y1) {
			yinc =1;
		    } else {
			yinc = -1;
		    }
		}
	    }
	    incr1 = 2 * dy;
	    d = incr1 - dx;
	    incr2 = 2 * (dy - dx);
	    Seg_Off_256(x, y, bpr, &offs, &bank);
	    Load_Write_Bank_256(bank);
	    lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
	    while (x < xend) {
		x++;
		if (d < 0) {
		    d += incr1;
		} else {
		    y += yinc;
		    d += incr2;
		}
		Seg_Off_256(x, y, bpr, &offs, &bank);
		Load_Write_Bank_256(bank);
		lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
	    }
	} else {
	    if (y1 > y2) {
		x = x2;
		y = y2;
		yend = y1;
		if (dx == 0) {
		    xinc = 0;
		} else {
		    if (x2 > x1) {
			xinc = -1;
		    } else {
			xinc = 1;
		    }
		}
	    } else {
		x = x1;
		y = y1;
		yend = y2;
		if (dx == 0) {
		    xinc = 0;
		} else {
		    if (x2 > x1) {
			xinc = 1;
		    } else {
			xinc = -1;
		    }
		}
	    }
	    incr1 = 2 * dx;
	    d = incr1 - dy;
	    incr2 = 2 * (dx - dy);
	    Seg_Off_256(x, y, bpr, &offs, &bank);
	    Load_Write_Bank_256(bank);
	    lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
	    while (y < yend) {
		y++;
		if (d < 0) {
		    d += incr1;
		} else {
		    x += xinc;
		    d += incr2;
		}
		Seg_Off_256(x, y, bpr, &offs, &bank);
		Load_Write_Bank_256(bank);
		lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
	    }
	}
    }
}

void linLine8(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color)
{
    register int t, distance;
    register DWORD addr;
    register WORD bpr;
    int xerr=0, yerr=0, deltax, deltay;
    int incx, incy;

    bpr = gd_getbpr();
    addr = gd_getflb();
    deltax = x2 - x1;			/* compute both distances */
    deltay = y2 - y1;

    if (deltax > 0)			/* compute increments */
	incx = 1;
    else if (deltax == 0)
	incx = 0;
    else
    	incx = -1;

    if (deltay > 0)
	incy = 1;
    else if (deltay == 0)
	incy = 0;
    else
	incy = -1;

    deltax = abs(deltax);		/* determine greater distance */
    deltay = abs(deltay);
    if (deltax > deltay)
	distance = deltax;
    else
	distance = deltay;

    for (t=0; t<=distance+1; t++) {	/* draw the line */
/*	vbe_putpixel(x1, y1, colour);*/
	lmempokeb((LIN_ADDR)(addr + y1 * bpr + x1), color);
	xerr += deltax;
	yerr += deltay;
	if (xerr > distance) {
	    xerr -= distance;
	    x1 += incx;
	}
	if (yerr > distance) {
	    yerr -= distance;
	    y1 += incy;
	}
    }
}

void WrPixel_256(WORD x, WORD y, DWORD color)
{
    WORD bpr;
    WORD bank, offs;

    bpr = gd_getbpr();
    Seg_Off_256(x, y, bpr, &offs, &bank);
    Load_Write_Bank_256(bank);
    lmempokeb((LIN_ADDR)(0xA0000 + offs), color);
/*    Load_Write_Bank_256(0);*/
}

void linWr8(WORD x, WORD y, DWORD color)
{
    WORD bpr;

    bpr = gd_getbpr();
    lmempokeb((LIN_ADDR)(flbaddr + y * bpr + x), color);
}

DWORD RdPixel_256(WORD x, WORD y)
{
    WORD bpr;
    WORD bank, offs;
    DWORD rv;

    bpr = gd_getbpr();
    Seg_Off_256(x, y, bpr, &offs, &bank);
    Load_Write_Bank_256(bank);
    rv = lmempeekb((LIN_ADDR)(0xA0000 + offs));
    return rv;
}

DWORD linRd8(WORD x, WORD y)
{
    WORD bpr;
    DWORD rv;

    bpr = gd_getbpr();
    rv = lmempeekb((LIN_ADDR)(flbaddr + y * bpr + x));
    return rv;
}

