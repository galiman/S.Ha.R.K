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
 CVS :        $Id: gd.h,v 1.1 2003/03/24 13:26:13 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 13:26:13 $
 ------------

**/

/*
 * Copyright (C) 2000 Luca Abeni
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

#ifndef __GD_H__
#define __GD_H__

#include <ll/sys/types.h>
/* Std 16 colors... are in cons.h !!!*/
#include <ll/i386/cons.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* 15bpp, 16bpp, 24bpp and 32bpp colors... */
#define color15(r, g, b) (((WORD)(r & 0x1F) << 10) | ((WORD)(g & 0x1F) << 5) | ((WORD)(b & 0x1F)))
#define color16(r, g, b) (((WORD)(r & 0x1F) << 11) | ((WORD)(g & 0x3F) << 5) | ((WORD)(b & 0x1F)))
#define color24(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))
#define color32(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))

#define rgb15(r, g, b) ((((WORD)(r & 0xF8)>>3) << 10) | (((WORD)(g & 0xF8)>>3) << 5) | ((WORD)(b & 0xF8)>>3))
#define rgb16(r, g, b) ((((WORD)(r & 0xF8)>>3) << 11) | (((WORD)(g & 0xFC)>>2) << 5) | ((WORD)(b & 0xF8)>>3))
#define rgb24(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))
#define rgb32(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))

/* Flags */
#define NOLINEAR 1

struct gmode {
    WORD xdim;
    WORD ydim;
    WORD bpr;
    BYTE bpp;
    WORD modenum;
};

#include "comp.h"

int gd_init(WORD flags);
void gd_showinfo(void);
int gd_setmode(WORD mode);
int gd_bsetmode(WORD mode);
WORD gd_getbpr(void);
DWORD gd_getflb(void);
DWORD gd_getmem(void);
void gd_showmodeinfo(void);
int gd_modenum(WORD x, WORD y, BYTE depth);
int gd_getmodeinfo(grx_vga_modeinfo *m);

void Load_Write_Bank_256(BYTE bank);

void Seg_Off_256(WORD x, WORD y, WORD pitch, WORD *offs, WORD *seg);
void Seg_Off_Hi(WORD x, WORD y, WORD pitch, WORD *offs, WORD *seg);


void gd_getcolor(BYTE ind, BYTE *r, BYTE *g, BYTE *b);
void gd_setcolor(BYTE ind,BYTE r,BYTE g,BYTE b);
void gd_getpalette(BYTE start, BYTE num, BYTE *pal);
void gd_setpalette(BYTE start, BYTE num, BYTE *pal);

__END_DECLS
#endif
