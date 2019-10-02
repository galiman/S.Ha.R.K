/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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

/* Glue Layer Header Linux Frame Buffer 2.6 Driver*/

#ifndef __SHARK_FB26__
#define __SHARK_FB26__

/* Init the FB 2.6 Driver */
int FB26_init();

/* Open FB */
int FB26_open(int num);

/* Close the FB */
int FB26_close(int num);

/* Set mode */
int FB26_setmode(int num, unsigned char *modeopt);

/* Use old GRX library to draw */
int FB26_use_grx(int num);

/* GRX emulation under FrameBuffer */
                                                                                                                             
void grx_plot(WORD x, WORD y, DWORD color);
DWORD grx_getpixel(WORD x, WORD y);
void grx_getimage(WORD x1, WORD y1, WORD x2, WORD y2, WORD *buf);
void grx_putimage(WORD x1, WORD y1, WORD x2, WORD y2, WORD *buf);
void grx_box(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void grx_clear(DWORD color);
void grx_rect(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void grx_line(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void grx_text(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
void grx_circle(WORD x, WORD y, WORD r, DWORD col);
void grx_disc(WORD x, WORD y, WORD r, DWORD col);

int vga16color[16];

/* 15bpp, 16bpp, 24bpp and 32bpp colors... */
#define color15(r, g, b) (((WORD)(r & 0x1F) << 10) | ((WORD)(g & 0x1F) << 5) | ((WORD)(b & 0x1F)))
#define color16(r, g, b) (((WORD)(r & 0x1F) << 11) | ((WORD)(g & 0x3F) << 5) | ((WORD)(b & 0x1F)))
#define color24(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))
#define color32(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))
                                                                                                                             
#define rgb15(r, g, b) ((((WORD)(r & 0xF8)>>3) << 10) | (((WORD)(g & 0xF8)>>3) << 5) | ((WORD)(b & 0xF8)>>3))
#define rgb16(r, g, b) ((((WORD)(r & 0xF8)>>3) << 11) | (((WORD)(g & 0xFC)>>2) << 5) | ((WORD)(b & 0xF8)>>3))
#define rgb24(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))
#define rgb32(r, g, b) (((DWORD)(r & 0xFF) << 16) | ((DWORD)(g & 0xFF) << 8) | (DWORD)(b & 0xFF))

#endif

