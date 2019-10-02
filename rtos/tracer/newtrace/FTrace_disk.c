/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Tullio Facchinetti  <tullio.facchinetti@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 * http://robot.unipv.it
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
 */

#include <ll/sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>
#include <unistd.h>

#include <FTrace_disk.h>
#include <FTrace_OSD.h>
#include <FTrace_chunk.h>
#include <FTrace_types.h>

#include "ll/i386/x-dos.h"

#include <kernel/kern.h>

#define TRACER_DISK_DEBUG

void write_all_to_disk();
 void write_to_disk(FTrace_Chunk_Ptr c, char *my_fname);

int TracerDiskInit = 0;

char fname[255];

/** 
 * Contains the data of all the chunks that is wanted to write to file.
 */
FTrace_Chunk_Ptr chunk_to_disk[MAX_CHUNK];

/** 
 * Keeps track of the number of chunks to be written on disk.
 */
int n_chunks_to_disk = 0;

/**
 * Initialize the Tracer chunk dumper to disk.
 * It sets the internal chunk sender to the function that writes
 * the chunk on disk. It initializes the filename that will be used
 * to open the file for saving the chunk.
 */
int FTrace_init_disk_writer(char *fn, int flag, char *l_ip, char *t_ip) {
	FTrace_set_internal_chunk_sender(FTrace_disk_writer);
	memcpy(fname, fn, sizeof(fname));
	TracerDiskInit = 1;
	return 0;
}

/** 
 * This function is called by the application when it asks to write a chunk on disk.
 * It saves the chunk data into the chunk_to_disk array. At the runlevel after the exit,
 * all the saved chunks will be written to disk.
 */
 void FTrace_disk_writer(FTrace_Chunk_Ptr c) {
	chunk_to_disk[n_chunks_to_disk++] = c;
	sys_atrunlevel(write_all_to_disk, NULL, RUNLEVEL_AFTER_EXIT);
}

/** 
 * This function is invoked at the runlevel after the exit .
 * It calls write_to_disk for each chunk that has to be wrote on disk.
 * To avoid to use the append flag of the fopen, that seems to not to work properly, 
 * a new filename is generated for each different chunk (). An incremental number is
 * put before the filename and passed to the write_to_disk function.
 */
void write_all_to_disk() {
  int i;
  char fname1[225];
  
  for (i = 0; i < n_chunks_to_disk; i++) {
    sprintf(fname1, "%d%s", i, fname);
    write_to_disk(chunk_to_disk[i], fname1);
  }
}

/** 
 * This function is called by the tracer to write the num-th chunk
 * on disk.
 */
 void write_to_disk(FTrace_Chunk_Ptr c, char *my_fname) {
	DWORD total_size = (DWORD)(c->size) + (DWORD)(c->emergency_size);
	DWORD start = (DWORD)(c->osd + FTRACE_OSD_CHUNK_HEAD);
	DWORD current = start;
	int writtenbytes;  /* number of bytes written */
	int err;
	long count = 0;
  
	DOS_FILE *f;
	
	if (!TracerDiskInit) {
		cprintf("FTrace_disk_writer: disk writing not initialized.\n");
		return;
	}
	
	/* open the DOS file for writing */
	f = DOS_fopen(my_fname, "w");
	
	/* check for open errors */
	if (!f) {
		/* error!! */
		err = DOS_error();
	
		/* note that if you call DOS_error() here, it return 0!!! */
		printk("Error %d opening %s...\n", err, fname);
		return;
	}
	
	FTrace_chunck_output_start();
	
	/** Writes blocks of 16 bytes (a single event) on the disk. */
	int block_size = 16;
	while (current + block_size <= start + total_size) {
		if (*(WORD *)(current) != 0) {
			writtenbytes = DOS_fwrite(current, block_size, 1, f);
			count++;
		}
		current += block_size;
		
#ifdef TRACER_DISK_DEBUG
	if (!(count % 10))
		printk("count %d\n", count);
#endif
	}
	
	FTrace_chunck_output_end();

#ifdef TRACER_DISK_DEBUG
	printk("Wrote: %d bytes in file %s (count = %ld)\n", total_size, fname, count);
	printk("FTrace_OSD_disk_writer: end function\n");
#endif

	/* Close the file */
	DOS_fclose(f);
	
	return;
} 
