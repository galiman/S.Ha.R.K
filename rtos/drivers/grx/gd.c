
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
#include <ll/sys/ll/ll-func.h>

#include "drivers/gd.h"

#include "drivers/s3.h"
#include "drivers/trident.h"
#include "drivers/vesa.h"
#include "drivers/chips.h"

BYTE bus, dev;
ModeInfoBlock vbemi;
BYTE actbank;
BYTE type;

DWORD flb=0;
int linear;
BYTE trylinear;
/*BYTE gmode = FALSE;*/

#define NMODES 13
static struct gmode allmodes[NMODES] = {
	{640, 400, 640, 8, 0x100},
	{640, 480, 640, 8, 0x101},
	{800, 600, 800, 8, 0x103},
	{1024, 768, 1024, 8, 0x105},
	{640, 480, 1280, 15, 0x110},
	{800, 600, 1600, 15, 0x113},
	{1024, 768, 2048, 15, 0x116},
	{640, 480, 1280, 16, 0x111},
	{800, 600, 1600, 16, 0x114},
	{1024, 768, 2048, 16, 0x117},
	{640, 480, 1920, 24, 0x112},
	{800, 600, 2400, 24, 0x115},
	{1024, 768, 3072, 24, 0x118}
};
static struct gmode actualmode;


static int myceil(float f)
{
		int i = (int) f;
		float f1 = f - i;

		if (f1 != 0) {
				return i + 1;
		} else {
				return i;
		}
}

/************************************************************************/
/*          GD Common Interface             */
/************************************************************************/
int gd_init(WORD flags)
{
	memset(&actualmode, 0, sizeof(struct gmode));
	type = UNKNOWN;

	/* VESA driver needs the V86 Mode... */
	vm86_init();

	/* First, check VESA... */
	if (vbe_getinfo() == 1) {
		type = VESA;
		if (flags & NOLINEAR) {
			trylinear = 0;
		} else {
			trylinear = 1;
		}
	        return 1;
	}

	/* Then, the other graph cards... Override VESA!!! */
	if (trident_test() == 1) {
		type = TRIDENT;
		trylinear = 0;
	}
	
	if (s3_test() == 1) {
		type = S3;
		if (flags & NOLINEAR) {
			trylinear = 0;
		} else {
			trylinear = 1;
		}
		return 1;
	}

	if (type == UNKNOWN) {
		return -1;
	}

	return -2;
}

void gd_showmodeinfo(void)
{
	cprintf("Mode number %x\n", actualmode.modenum);
	cprintf("MaxX: %u\n", actualmode.xdim);
	cprintf("MaxY: %u\n", actualmode.ydim);
	cprintf("BPR:  %u\n", actualmode.bpr);
}

void gd_showinfo(void)
{
	DWORD addr;

	addr = gd_getflb();

	if (addr != 0) {
		cprintf ("\t gd: Frame Linear Buffer @ %lx (%luM)\n", 
			addr, (addr / 0x1000000));
	}
	
	if (type == VESA) {
		cprintf("Vesa SVGA card\n");
		vbe_showinfo();
	}
	if (type == TRIDENT) {
		cprintf("Trident graphic card\n");
		trident_showinfo();
	}

	if (type == S3) {
		cprintf("S3 graphic card\n");
		s3_showinfo();
	}
}

int gd_setmode(WORD mode)
{
	WORD app;
	int i;
#ifndef VM86
	BYTE p1, p2;
#endif

	if (mode == 0) {
		X_REGS16 inregs, outregs;
		X_SREGS16 sregs;

		if (actualmode.modenum != 0) {
			inregs.x.ax = 0x03;
#ifndef VM86
			p1 = inp(0x21);
			p2 = inp(0xA1);
			outp(0x21,0xFF);
			outp(0xA1,0xFF);
			X_callBIOS(0x10, &inregs, &outregs, &sregs);
			outp(0x21,p1);
			outp(0xA1,p2);
#else
			vm86_callBIOS(0x10, &inregs, &outregs, &sregs);
#endif
		}
		return 1;
	}

	for (i = 0; i < NMODES; i++) {
		if ((mode == allmodes[i].modenum)) {
			memcpy(&actualmode, &(allmodes[i]), sizeof(struct gmode));
		}
	}
	 
	if (type == VESA) {

		if (vbe_getmodeinfo(&vbemi, mode) < 0) {
                        return -1;
                }

		app = mode;
		if (trylinear) {
			/*  try linear...   */
			mode = mode | 0x4000;
			linear = TRUE;
			if (vbe_setmode(mode) == 1) {
				return linear;
			}
		}
		actbank = 0;
		Load_Write_Bank_256(0);
		linear = FALSE;
		mode = app;
		if (vbe_setmode(mode) < 0) {
			return -1;
		}
		return linear;
	}

	if (type == S3) {
/* Still use VESA to open graph... */
		if (vbe_getmodeinfo(&vbemi, mode) < 0) {
			return -1;
		}
		if (vbe_setmode(mode) < 0) {
			return -1;
		}
		if ((flb != 0) && trylinear) {
			linear = TRUE;
			s3_linear(flb);
			return TRUE;
		}
		linear = FALSE;
		actbank = 0;
		Load_Write_Bank_256(0);
		return FALSE;
	}

	if (type == TRIDENT) {
/* Still use VESA to open graph... */
	if (vbe_getmodeinfo(&vbemi, mode) < 0) {
		return -1;
	}
	if (vbe_setmode(mode) < 0) {
		return -1;
	}

	actbank = 0;
	Load_Write_Bank_256(0);
	return FALSE;
	}

	return -2;
}

DWORD gd_getflb(void)
{
	if (type == VESA) {
	return vbe_getflb();
	}
	if (type == S3) {
	return flb;
	}
	return 0;
}

DWORD gd_getmem(void)
{
	if (type == VESA) {
	return vbe_getmem();
	}
	if (type == S3) {
	return s3_getmem();
	}
	if (type == TRIDENT) {
	return trident_getmem();
	}

	return 0;
}

WORD gd_getbpr(void)
{
	if (actualmode.bpr) return actualmode.bpr;
	return -2;
}

int gd_getmodeinfo(grx_vga_modeinfo *m)
{
	if (actualmode.modenum == 0) {
	return -1;
	}

	m->width = actualmode.xdim;
	m->height = actualmode.ydim;
	m->bytesperpixel = (myceil((float)actualmode.bpp / 8));
/*    m->colors = myexp(2, actualmode.bpp); */
	m->colors = 1 << actualmode.bpp;
	m->linewidth = gd_getbpr();
	m->maxlogicalwidth = gd_getbpr(); /* 4 the moment... */
	m->startaddressrange = 0;   /* don't know :( */
	m->maxpixels = gd_getmem() / m->bytesperpixel;
	m->haveblit = 0;
	m->flags = 0;

	/* Extended fields: */
	m->chiptype = type;     /* Chiptype detected */
	m->memory = gd_getmem();
	m->linewidth_unit = 0;  /* don't know :( */
	if (linear) {
	m->linear_aperture = (LIN_ADDR)gd_getflb();
	m->aperture_size = gd_getmem();
	} else {
	m->linear_aperture = 0;
	m->aperture_size = 0xFFFF;
	}
	m->set_aperture_page = NULL;
	m->extensions = NULL;
	return 1;
}
	
void Load_Write_Bank_256(BYTE bank)
{
	if (bank != actbank) {
	actbank = bank;
	if (type == VESA) {
		if (vbe_setbank(&vbemi, bank) < 0)
			ll_abort(259);
	}
	if (type == S3) {
		s3_setpage(bank);
	}
	if (type == TRIDENT) {
		trident_setpage(bank);
	}
	}
}

/************************************************************************/
/*          BANK HANDLING FUNCS             */
/************************************************************************/

LIN_ADDR start_address(WORD bpr, WORD x, WORD y)
{
	LIN_ADDR a;

	a = (LIN_ADDR)(y * bpr +x);
	return a;
}

void Seg_Off_256(WORD x, WORD y, WORD pitch, WORD *offs, WORD *seg)
{
	DWORD a;

	a = y * pitch + x;
	*offs = a & 0x0000FFFF;
	*seg = a >> 16;
}

void Seg_Off_Hi(WORD x, WORD y, WORD pitch, WORD *offs, WORD *seg)
{
	DWORD a;

	a = y * pitch + x * 2;
	*offs = a & 0x0000FFFF;
	*seg = a >> 16;
}

int gd_modenum(WORD x, WORD y, BYTE depth)
{
	int  mode, i;

	mode = -1;

	for (i = 0; i < NMODES; i++) {
	if ((depth == allmodes[i].bpp) && (x == allmodes[i].xdim) && (y == allmodes[i].ydim)) {
		mode = allmodes[i].modenum;
	} else {
	}
	}

	if ((mode != -1) && (vbe_checkmode(mode) != -1))
		return mode;
	return -1;
}
