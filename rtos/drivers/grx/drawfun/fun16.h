
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

#include "ll/sys/cdefs.h"

__BEGIN_DECLS
#define memsetw(addr, val, n) {LIN_ADDR i;	for(i = addr; i < addr + n; i += 2) lmempokew(i, val);}


void RdWin_Hi(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void linGet16(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void WrWin_Hi(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void linPut16(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void ClrWin_Hi(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void linBox16(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void WrRect_Hi(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void linRect16(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void WrText_Hi(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
void linText16(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
void WrLine_Hi(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void linLine16(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void WrPixel_Hi(WORD x, WORD y, DWORD color);
void linWr16(WORD x, WORD y, DWORD color);
DWORD RdPixel_Hi(WORD x, WORD y);
DWORD linRd16(WORD x, WORD y);
__END_DECLS
