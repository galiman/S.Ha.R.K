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

/**
 ------------
 CVS :        $Id: glib.h,v 1.2 2006/03/09 15:57:59 tullio Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2006/03/09 15:57:59 $
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

#ifndef __GLIB_H__
#define __GLIB_H__

#include <ll/sys/types.h>
#include <drivers/gd.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

int grx_init(void);
int grx_setmode(WORD mode);
int grx_close(void);
extern void (*grx_plot)(WORD x, WORD y, DWORD color);
extern DWORD (*grx_getpixel)(WORD x, WORD y);
extern void (*grx_getimage)(WORD x1, WORD y1, WORD x2, WORD y2, WORD *img);  // Tool
extern void (*grx_putimage)(WORD x1, WORD y1, WORD x2, WORD y2, WORD *img);  // Tool
extern void (*grx_box)(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
extern void (*grx_rect)(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
extern void (*grx_line)(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
extern void (*grx_text)(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
extern void (*grx_circle)(WORD x, WORD y, WORD r, DWORD col);
extern void (*grx_disc)(WORD x, WORD y, WORD r, DWORD col);

void grx_modeinfo(void);
int grx_getmode(WORD x, WORD y, BYTE depth);
void grx_cardinfo(void);

void grx_setcolor(BYTE ind,BYTE r,BYTE g,BYTE b);
void grx_getcolor(BYTE ind, BYTE *r, BYTE *g, BYTE *b);
void grx_setpalette(BYTE start, BYTE num, BYTE *pal);
void grx_getpalette(BYTE start, BYTE num, BYTE *pal);

int grx_open(WORD x, WORD y, BYTE depth);

void grx_clear(DWORD color);

__END_DECLS
#endif
