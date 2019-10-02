
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

#ifndef __FTRACE_OSD__
#define __FTRACE_OSD__

/* OS Dependent functions */

#include <FTrace_types.h>

#define FTRACE_OSD_CHUNK_HEAD 24

#define FTrace_printf cprintf

void FTrace_OSD_save_event(WORD type, WORD par1);

/* OSD Tracer Init */
int FTrace_OSD_init();

/* OSD Chunk Init */
int FTrace_OSD_chunk_init(FTrace_Chunk_Ptr c, int normal_size, int emergency_site, FTrace_flags flags);

int FTrace_OSD_update_chunk_flags(FTrace_Chunk_Ptr c);

/* OSD Chunk Link */
int FTrace_OSD_chunk_link(FTrace_Chunk_Ptr a, FTrace_Chunk_Ptr b);

/* OSD Chunk compress function */
int FTrace_OSD_compress_chunk(int number, void *temp_data, int *data_size);

/* OSD Send Chunk out */
int FTrace_OSD_send_chunk(FTrace_Chunk_Ptr c, int osd_flags);

/* OSD Chunk Dump */
int FTrace_OSD_chunk_dump(FTrace_Chunk_Ptr c);

#endif

