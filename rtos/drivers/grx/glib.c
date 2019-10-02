
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
#include <ll/i386/string.h>
#include <ll/i386/error.h>
#include <ll/sys/ll/ll-func.h>

#include <drivers/gd.h>
#include <drivers/glib.h>

#include "drawfun/fun8.h"
#include "drawfun/fun16.h"

/*#define FORCEBANK		// 4 dbg purpose*/

int videocard;
DWORD flbaddr = 0;

void (*grx_plot)(WORD x, WORD y, DWORD color);
DWORD (*grx_getpixel)(WORD x, WORD y);
void (*grx_getimage)(WORD x1, WORD y1, WORD x2, WORD y2, WORD *buf);  // Tool
void (*grx_putimage)(WORD x1, WORD y1, WORD x2, WORD y2, WORD *buf);  // Tool
void (*grx_box)(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void (*grx_rect)(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void (*grx_line)(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void (*grx_text)(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
void (*grx_circle)(WORD x, WORD y, WORD r, DWORD col);
void (*grx_disc)(WORD x, WORD y, WORD r, DWORD col);

void dummyfun(void)
{
    error("Not yet implemented...\n");
    ll_abort(500);
}

static void circlepixels(WORD x, WORD y, WORD sx, WORD sy, DWORD c)
{
    grx_plot(sx + x, sy + y, c);
    grx_plot(sx - x, sy + y, c);
    grx_plot(sx + x, sy - y, c);
    grx_plot(sx - x, sy - y, c);
    grx_plot(sx + y, sy + x, c);
    grx_plot(sx - y, sy + x, c);
    grx_plot(sx + y, sy - x, c);
    grx_plot(sx - y, sy - x, c);
}

void circle(WORD sx, WORD sy, WORD r, DWORD c)
{
    int x, y, d;
    
	if (r < 1) {
	grx_plot(sx, sy, c);
	return;
    }
    x = 0;
    y = r;
    d = 1 - r;
    circlepixels(x, y, sx, sy, c);
    while (x < y) {
	if (d < 0)
	    d += x * 2 + 3;
	else {
	    d += x * 2 - y * 2 + 5;
	    y--;
	}
	x++;
	circlepixels(x, y, sx, sy, c);
    }
}

/* grx_disc by Massy */

static __inline__ void discpixels(WORD x, WORD y, WORD sx, WORD sy, DWORD c)
{
    grx_line(sx + x, sy + y, sx + x, sy - y, c);
    grx_line(sx - x, sy + y, sx - x, sy - y, c);
    grx_line(sx + y, sy + x, sx + y, sy - x , c);
    grx_line(sx - y, sy + x, sx - y, sy - x , c);
}

void disc(WORD sx, WORD sy, WORD r, DWORD c)
{
    int x, y, d;
    
	if (r < 1) {
	grx_plot(sx, sy, c);
	return;
    }
    x = 0;
    y = r;
    d = 1 - r;
    discpixels(x, y, sx, sy, c);
    while (x < y) {
	if (d < 0)
	    d += x * 2 + 3;
	else {
	    d += x * 2 - y * 2 + 5;
	    y--;
	}
	x++;	
	discpixels(x, y, sx, sy, c);
    }
}

int grx_init(void)
{
#ifdef FORCEBANK
    videocard = gd_init(NOLINEAR);
#else 
    videocard = gd_init(0);
#endif
    if (videocard < 0) {
	cprintf("Graphic Driver Info error...\n");
	return -1;
    }
    return 1;
}

int grx_close(void)
{
    gd_setmode(0);
    return 1;
}

int grx_setmode(WORD mode)
{
    int lin;
    grx_vga_modeinfo m;

    if ((lin = gd_setmode(mode)) < 0){
	return -1;
    }
    gd_getmodeinfo(&m);

    switch (m.bytesperpixel) {
	case 1 : if (lin == TRUE){
		flbaddr = gd_getflb();
		grx_plot = linWr8;
		grx_getpixel = linRd8;
		grx_rect = linRect8;
		grx_box = linBox8;
		grx_text = linText8;
		grx_line = linLine8;
		grx_putimage = linPut8;
		grx_getimage = linGet8;
    	    } else {
		grx_plot = WrPixel_256;
		grx_getpixel = RdPixel_256;
		grx_rect = WrRect_256;
		grx_box = ClrWin_256;
		grx_text = WrText_256;
		grx_line = WrLine_256;
		grx_putimage = WrWin_256;
		grx_getimage = RdWin_256;
    	    }
	    break;
	case 2 : if (lin == TRUE){
		flbaddr = gd_getflb();
		grx_plot = linWr16;
		grx_getpixel = linRd16;
		grx_rect = linRect16;
		grx_box = linBox16;
		grx_text = linText16;
		grx_line = linLine16;
		grx_putimage = linPut16;
		grx_getimage = linGet16;
    	    } else {
		grx_plot = WrPixel_Hi;
		grx_getpixel= RdPixel_Hi;
		grx_rect = WrRect_Hi;
		grx_box = ClrWin_Hi;
		grx_text = WrText_Hi;
		grx_line = WrLine_Hi;
		grx_putimage = WrWin_Hi;
		grx_getimage = RdWin_Hi;
    	    }
	    break;
	default :
	    grx_plot = (void (*)(WORD, WORD, DWORD))dummyfun;
	    grx_getpixel = (DWORD (*)(WORD, WORD))dummyfun;
	    grx_rect = (void (*)(WORD, WORD, WORD, WORD, DWORD))dummyfun;
	    grx_box = (void (*)(WORD, WORD, WORD, WORD, DWORD))dummyfun;
	    grx_text = (void (*)(char *, WORD, WORD, DWORD, DWORD))dummyfun;
	    grx_line = (void (*)(WORD, WORD, WORD, WORD, DWORD))dummyfun;
	    grx_putimage = (void (*)(WORD, WORD, WORD, WORD, WORD *))dummyfun;  // Tool
	    grx_getimage = (void (*)(WORD, WORD, WORD, WORD, WORD *))dummyfun;  // Tool
	    break;
    }
	grx_circle = circle;
	grx_disc = disc;
    return 1;
}

void grx_modeinfo(void)
{
    gd_showmodeinfo();
}
int grx_getmode(WORD x, WORD y, BYTE depth)
{
    return gd_modenum(x, y, depth);
}
void grx_cardinfo(void)
{
    gd_showinfo();
}

void grx_getcolor(BYTE ind, BYTE *r, BYTE *g, BYTE *b)
{
    gd_getcolor(ind, r, g, b);
}
void grx_setcolor(BYTE ind,BYTE r,BYTE g,BYTE b)
{
    gd_setcolor(ind, r, g, b);
}
void grx_getpalette(BYTE start, BYTE num, BYTE *pal)
{
    gd_getpalette(start, num, pal);
}
void grx_setpalette(BYTE start, BYTE num, BYTE *pal)
{
    gd_setpalette(start, num, pal);
}


int grx_open(WORD x, WORD y, BYTE depth)
{
    int mode;
    
    mode = grx_getmode(x, y, depth);

    if (grx_setmode(mode) < 0) {
	return -1;
    }

    return 1;
}

void grx_clear(DWORD color)
{
    grx_vga_modeinfo m;

    gd_getmodeinfo(&m);
    grx_box(0, 0, m.width, m.height, color);
}
