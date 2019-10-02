
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

#include <FTrace_chunk.h>
#include <FTrace_OSD.h>
#include <FTrace_types.h>

#include <FTrace_udp.h>

#include <kernel/kern.h>
#include <stdlib.h>

SYS_FLAGS FTracef;

extern FTrace_Chunk_Ptr ActualChunk;
extern void *OSD_current_pointer;

/** 
 * This flag keeps track of the current chunk sending action.
 * It is 1 if the output is still ongoing.
 * It is reset to 0 if the output is finished.
 */
int chunk_sending = 0;

/** 
 * Pointer to the function that actually perform the chunk send.
 */
void (*FTrace_internal_send_chunk)(FTrace_Chunk_Ptr) = NULL;

/** 
 * Initializes the poiter to the function for actually sending the chunk.
 */
void FTrace_set_internal_chunk_sender(void (*ptr)(FTrace_Chunk_Ptr)) {
	FTrace_internal_send_chunk = ptr;
}

/** 
 * This function is called before starting the chunck sending.
 */
void FTrace_chunck_output_start() {
	chunk_sending = 1;
}

/** 
 * This function is called after the chunck has been entirely sent.
 */
void FTrace_chunck_output_end() {
	chunk_sending = 0;
}


void FTrace_fsave()
{

  __asm__("cpuid\n\t":::"eax","ebx","ecx","edx");
  FTracef = kern_fsave();

}

void FTrace_frestore()
{
  __asm__("cpuid\n\t":::"eax","ebx","ecx","edx");                                                                            
  kern_frestore(FTracef);
                                                                                                                             
}

void FTrace_lock()
{

}

void FTrace_unlock()
{
                                                                                                                             
}

void *FTrace_malloc(int size)
{

  return malloc(size);

}

void FTrace_free(void *ptr)
{
                                                                                                                             
  free(ptr);
                                                                                                                             
}

int FTrace_OSD_init()
{

  return 0;

}

/* OSD_pointers

   00 - Actual Chunk Position
   04 - Actual Chunk Start
   08 - Actual Chunk Size
   12 - Actual Chunk Emergency Size
   16 - Actual Chunk Flags
                                                                                                                             
   20 - Next Chunk OSD Pointer 
                                                                                                                             
*/

int FTrace_OSD_chunk_init(FTrace_Chunk_Ptr c, int size, int emergency_size, FTrace_flags flags)
{

  *(DWORD *)(c->osd) = (DWORD)(c->osd + FTRACE_OSD_CHUNK_HEAD);
  *(DWORD *)(c->osd + 4) = (DWORD)(c->osd + FTRACE_OSD_CHUNK_HEAD);
  *(DWORD *)(c->osd + 8) = (DWORD)(c->size);
  *(DWORD *)(c->osd + 12) = (DWORD)(c->emergency_size);
  *(DWORD *)(c->osd + 16) = (DWORD)(c->flags);
  *(DWORD *)(c->osd + 20) = 0;

  return 0;

}

/* OSD Chunk Link */
int FTrace_OSD_chunk_link(FTrace_Chunk_Ptr a, FTrace_Chunk_Ptr b)
{

  *(DWORD *)(a->osd + 20) = (DWORD)(b->osd);

  return 0;

}

int FTrace_OSD_update_chunk_flags(FTrace_Chunk_Ptr c)
{

  *(DWORD *)(c->osd + 16) = (DWORD)(c->flags);

  return 0;

}

int FTrace_OSD_compress_chunk(int number, void *temp_data, int *data_size)
{

  return 0;

}

int FTrace_OSD_send_chunk(FTrace_Chunk_Ptr c, int osd_flag) {

  // Tool: send the chunk using the selected method
  if (FTrace_internal_send_chunk != NULL)
  	FTrace_internal_send_chunk(c);

  
  struct timespec t;
  t.tv_sec = 2;
  t.tv_nsec = 0;
  //cprintf("Chunk sending");   // Tool: DEBUG
  while(chunk_sending) {
  	//cprintf(".");   // Tool: DEBUG
	nanosleep(&t,NULL);
  }
  //cprintf("\n");   // Tool: DEBUG

  return 0;
}

int FTrace_OSD_chunk_dump(FTrace_Chunk_Ptr c)
{

  FTrace_printf("Position : %x\n",*(int *)(c->osd));
  FTrace_printf("Start    : %x\n",*(int *)(c->osd+4));
  FTrace_printf("Size     : %d\n",*(int *)(c->osd+8));
  FTrace_printf("Em-Size  : %d\n",*(int *)(c->osd+12));
  FTrace_printf("Flags    : %x\n",*(int *)(c->osd+16));
  FTrace_printf("Next     : %x\n\n",*(int *)(c->osd+20));

  return 0;
 
}


