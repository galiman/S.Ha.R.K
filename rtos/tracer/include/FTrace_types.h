
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

#ifndef __FTRACE_TYPES__
#define __FTRACE_TYPES__

/* FTrace types and structures */

#include <ll/sys/types.h>
#include <ll/i386/mem.h>
#include <ll/math.h>

#include "FTrace.h"

#define FTRACE_CHUNK_ID 0xFFAAFFBB

#define MAX_CHUNK 256

#define FTRACE_CHUNK_FLAG_FREE  0x01 /* Chunk is free */
#define FTRACE_CHUNK_FLAG_FULL  0x02 /* Chunk is full */
#define FTRACE_CHUNK_FLAG_CYC   0x00 /* Mode: Cyclical */
#define FTRACE_CHUNK_FLAG_JTN   0x04 /* Mode: Jump To Next */
#define FTRACE_CHUNK_FLAG_STOP  0x08 /* MOde: Stop if full */
#define FTRACE_CHUNK_FLAG_BUSY  0x10 /* Chunk is busy */
#define FTRACE_CHUNK_FLAG_NODEL 0x20 /* Chunk is protected from delete */
#define FTRACE_CHUNK_FLAG_COMPR 0x40 /* Chunk is compressed */

typedef BYTE FTrace_flags;

struct FTrace_Chunk {
  DWORD id;
  DWORD number;
  FTrace_flags flags;
  DWORD size;
  DWORD emergency_size;
  DWORD osd;
};

typedef struct FTrace_Chunk *FTrace_Chunk_Ptr;

#endif
