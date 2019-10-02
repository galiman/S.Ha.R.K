
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
void RdWin_256(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void linGet8(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void WrWin_256(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void linPut8(WORD x1, WORD y1, WORD x2, WORD y2, BYTE *buf);
void ClrWin_256(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void linBox8(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void WrRect_256(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void linRect8(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void WrText_256(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
void linText8(char *text, WORD x, WORD y, DWORD fg, DWORD bg);
void WrLine_256(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void linLine8(WORD x1, WORD y1, WORD x2, WORD y2, DWORD color);
void WrPixel_256(WORD x, WORD y, DWORD color);
void linWr8(WORD x, WORD y, DWORD color);
DWORD RdPixel_256(WORD x, WORD y);
DWORD linRd8(WORD x, WORD y);
__END_DECLS
