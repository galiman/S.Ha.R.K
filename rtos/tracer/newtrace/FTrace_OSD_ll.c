
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

#include <FTrace_types.h>
#include <FTrace_OSD.h>
#include <FTrace.h>

#include <ll/i386/hw-instr.h>

/* OSD_pointers

   00 - Actual Chunk Position
   04 - Actual Chunk Start
   08 - Actual Chunk Size
   12 - Actual Chunk Emergency Size
   16 - Actual Chunk Flags

   20 - Next OSD pointer

*/

extern WORD FTrace_filter_mask;

/**
 * This is the function that actually store the event into the selected chunk.
 */
void FTrace_safe_ipoint(WORD type, WORD par1, DWORD par2)
{

  extern void *OSD_current_pointer;
  extern int FTraceEnable;

  DWORD tsclow, tschigh;
  DWORD current;
  DWORD start;
  DWORD size;
  DWORD flags;
  
  SYS_FLAGS f;

  f = ll_fsave();

  if (FTraceEnable) {
  
    /** Tool: do not filter the "filter" event family. */
    if ((type & 0xF0) != 0xF0)  
      if (FTrace_filter_mask & (0x01 << (type & FTrace_family_mask)))
        return;
    
    current = *(DWORD *)(OSD_current_pointer);
    start   = *(DWORD *)(OSD_current_pointer + 4);
    size    = *(DWORD *)(OSD_current_pointer + 8);
    flags   = *(DWORD *)(OSD_current_pointer + 16);

    if (current == NULL) {
      ll_frestore(f);
      return;
    }

    if (type != FTrace_EVT_next_chunk) {

      if ((flags & FTRACE_CHUNK_FLAG_FULL) == FTRACE_CHUNK_FLAG_FULL) {
	ll_frestore(f);
	return;
      }
      
      /** Cyclical Buffer */
      if (current + 16 >= (start + size)) {
	if ((flags & 0x0C) == FTRACE_CHUNK_FLAG_CYC) {
          current = start;
	} else {
	  *(DWORD *)(OSD_current_pointer + 16) |= FTRACE_CHUNK_FLAG_FULL;
	  if ((flags & 0x0C) ==  FTRACE_CHUNK_FLAG_JTN) {
		OSD_current_pointer = (void *)(*(DWORD *)(OSD_current_pointer + 20));
		ll_frestore(f);
		return;
          }
	}
      }

      __asm__("cpuid\n\t"
              "rdtsc\n\t"
	      : "=a" (tsclow), "=d" (tschigh) : : "ecx","ebx");
      
      *(DWORD *)current = (DWORD)((DWORD)(par1) << 16 | (DWORD)(type));
      *(DWORD *)(current + 4) = tschigh;
      *(DWORD *)(current + 8) = tsclow;
      *(DWORD *)(current + 12) = par2;

      *(DWORD *)(OSD_current_pointer) = current + 16;

    } else {

      *(WORD *)(OSD_current_pointer + 16) |= FTRACE_CHUNK_FLAG_FULL;
      OSD_current_pointer = (void *)(*(DWORD *)(OSD_current_pointer + 20));
      ll_frestore(f);
      return;	

    }

  }

  ll_frestore(f);

}

void FTrace_unsafe_ipoint(WORD type, WORD par1)
{

  extern void *OSD_current_pointer;

  __asm__("pushfl\n\t" /* Critical Section */
          "cli\n\t" 
	  "cpuid\n\t"
	  "rdtsc\n\t"
	  "movl (%%edi),%%ebx\n\t"
          "movl %%esi,(%%ebx)\n\t"   /* Save TYPE + WORD par1 */
          "movl %%edx,4(%%ebx)\n\t"  /* Save TSC HIGH */
          "movl %%eax,8(%%ebx)\n\t"  /* Save TSC LOW */
	  "addl $16,(%%edi)\n\t"     /* Inc Position */
	  "popfl\n\t"
          :: "D" ((DWORD *)(OSD_current_pointer)), "S" (par1 << 16 | (type | 0x8000)));

}

