
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

#ifndef __FTRACE_CHUNK__
#define __FTRACE_CHUNK__

/* FTrace chunk function header */

#include <FTrace_types.h>

/* Mutex for FTrace */
void FTrace_lock();
void FTrace_unlock();

void FTrace_fsave();
void FTrace_frestore();

/* Memory */
void *FTrace_malloc(int size);
void FTrace_free(void *ptr);

/* Enable/Disable Tracer */
int FTrace_enable();
int FTrace_disable();

/* Create a chunk of specified size (normal/emergency) */
int FTrace_chunk_create(int normal_size, int emergency_size, FTrace_flags flags);

/* Delete a Chunk */
int FTrace_chunk_delete(int number);

/* Set the chunk flags */
int FTrace_set_chunk_flags(int number, FTrace_flags flags);

/* Get the chunk flags */
int FTrace_get_chunk_flags(int number, FTrace_flags *flags);

/* Select the actual chunk */
int FTrace_actual_chunk_select(int number);

/* Link two chunks */
int FTrace_chunk_link(int chunk_A, int chunk_B);

/* Find the first chunk with specific flags*/
int FTrace_get_first_chunk(FTrace_flags flags);

/* Get one chunks status */
FTrace_Chunk_Ptr *FTrace_get_chunk_table();

/* Create a new memory region where the compressed data are stored */
int FTrace_compress_chunk(int number, FTrace_flags new_flags);

/* Send the chunk out from the memory */
int FTrace_send_chunk(int number, int osd_flags, FTrace_flags new_flags);

#endif

