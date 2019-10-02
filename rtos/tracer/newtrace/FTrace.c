
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
#include <FTrace_types.h>
#include <FTrace_OSD.h>
#include <FTrace.h>
#include <tracer.h>

#define FTRACE_DEBUG

WORD FTrace_filter_mask = 0;

/* Globals */

FTrace_Chunk_Ptr ChunkTable[MAX_CHUNK]; /* Chunk array */
FTrace_Chunk_Ptr ActualChunk = NULL; /* Actual Chunk */

/* OSD Pointer */
void *OSD_current_pointer;

int              FTraceInit   = 0;
int	         FTraceEnable = 0;

/* Init a chunk with default value */
static int FTrace_chunk_init(FTrace_Chunk_Ptr c, int number, int size, int emergency_size, FTrace_flags flags)
{

  c->id       		= FTRACE_CHUNK_ID; /* Std ID */
  c->number   		= number; /* Chunk number */
  c->flags    		= flags; /* Chunk flags */
  c->size	 	= size; /* Chunk size */
  c->emergency_size	= emergency_size; /* Chunk emergency size */ 
  c->osd      		= (DWORD)((DWORD)(c) + sizeof(struct FTrace_Chunk));
  
  return FTrace_OSD_chunk_init(c, size, emergency_size, flags);

}

/**
 * Set the filter for a specific family of events.
 * Store the choice into the filter mask.
 * If status is 1 then enable the filter.
 * If status is 0 then disable the filter.
 */
void FTrace_set_filter(BYTE filter, int status) {
	if (status) FTrace_filter_mask |= (0x01 << (filter & FTrace_family_mask));
	if (!status) FTrace_filter_mask &= ~(0x01 << (filter & FTrace_family_mask));
	
	#ifdef FTRACE_DEBUG
	printk("FTrace_set_filter: %x\n", FTrace_filter_mask);
	#endif
	
	TRACER_LOGEVENT(filter, status, 0);
}

/* Find a free slot in ChunkTable */
static int FTrace_find_free_slot()
{

  int i;

  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    return -1;
  }

  for (i = 0;i < MAX_CHUNK;i++)
    if (ChunkTable[i] == NULL)
      return i;

  return -1;
    
}

/* Init the FTrace */
static int FTrace_Init()
{

  int i,err;

  /* Check if it's just initialized */
  if (FTraceInit == 1)
    return 0;

  FTrace_lock();

  OSD_current_pointer = NULL;

  for (i = 0;i < MAX_CHUNK;i++)
    ChunkTable[i] = NULL;

  /* Init System Dependet Part */
  err = FTrace_OSD_init();
  if (err != 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace OSD not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  FTraceInit = 1;

  FTrace_unlock();

  return 0;

}

/* Enable Tracer */
int FTrace_enable()
{

  if (FTraceInit == 0 || ActualChunk == NULL) return -1;

  FTrace_fsave();
    FTraceEnable = 1;
  FTrace_frestore(); 

  return 0;

} 

/* Disable Tracer */
int FTrace_disable()
{
                                                                                                                             
  if (FTraceInit == 0) return -1;
                                                                                                                             
  FTrace_fsave();
    FTraceEnable = 0;
  FTrace_frestore();
                                                                                                                             
  return 0;
                                                                                                                             
}

int FTrace_chunk_create(int size, int emergency_size, FTrace_flags flags)
{

  FTrace_Chunk_Ptr FT_temp;
  int number, err;

  FTrace_lock();

  if (FTraceInit == 0) {
    err = FTrace_Init();
    if (err != 0) {
      #ifdef FTRACE_DEBUG
        FTrace_printf("FTrace Error: Initialization fail\n");
      #endif
      FTrace_unlock();
      return -1;
    }
  }

  number = FTrace_find_free_slot();
  if (number == -1) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: cannot find free slot for chunk\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  FT_temp = (FTrace_Chunk_Ptr)FTrace_malloc(sizeof(struct FTrace_Chunk) + FTRACE_OSD_CHUNK_HEAD + size + emergency_size);
  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: cannot allocate memory for chunk\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  memset(FT_temp,0,sizeof(struct FTrace_Chunk) + FTRACE_OSD_CHUNK_HEAD + size + emergency_size);

  err = FTrace_chunk_init(FT_temp, number, size, emergency_size, flags);
  if (err != 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: cannot initialized the new chunk\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  /* Set the ChunkTable */
  ChunkTable[number] = FT_temp;

  #ifdef FTRACE_DEBUG
    FTrace_printf("FTrace Debug: Chunk %d created at addr %x\n",number,(int)FT_temp);
  #endif

  FTrace_unlock();
  return number;

}

/* Delete a Chunk */
int FTrace_chunk_delete(int number)
{

  FTrace_Chunk_Ptr FT_temp = ChunkTable[number]; 

  FTrace_lock();

  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  if (FT_temp->flags & FTRACE_CHUNK_FLAG_NODEL) {
    FTrace_unlock();
    return 0;
  }

  FTrace_free(FT_temp);
  ChunkTable[number] = NULL;

  FTrace_unlock();
  return 0;

}

/* Set the chunk flags */
int FTrace_set_chunk_flags(int number, FTrace_flags flags)
{
                                                                                                    
  FTrace_Chunk_Ptr FT_temp = ChunkTable[number];
                                                                                                    
  FTrace_lock();
                                                                                                    
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }
                                                                                                    
  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  FT_temp->flags = flags;
  FTrace_OSD_update_chunk_flags(FT_temp);

  return 0;

}

/* Get the chunk flags */
int FTrace_get_chunk_flags(int number, FTrace_flags *flags)
{
                                                                                                    
  FTrace_Chunk_Ptr FT_temp = ChunkTable[number];
                                                                                                    
  FTrace_lock();
                                                                                                    
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }
                                                                                                    
  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }
      
  FTrace_OSD_update_chunk_flags(FT_temp);                                                                                      *flags = FT_temp->flags;

  FTrace_unlock();

  return 0;
                                                                                                    
}

/* Select the actual chunk */
int FTrace_actual_chunk_select(int number)
{

  FTrace_Chunk_Ptr FT_temp = ChunkTable[number];
    
  FTrace_lock();
                                                                                                
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }
                                                                                                    
  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  if (FT_temp->flags & FTRACE_CHUNK_FLAG_FREE) {

    /* Set as used */
    FT_temp->flags &= ~FTRACE_CHUNK_FLAG_FREE;
    FTrace_OSD_update_chunk_flags(FT_temp);

    /* Update the actual_chunk and OSD_pointers */
    FTrace_fsave();
      ActualChunk = FT_temp;
      OSD_current_pointer = (void *)FT_temp->osd;
    FTrace_frestore();

  } else {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk is not free\n");
    #endif
    FTrace_unlock();
    return -1;
  }
    
  FTrace_unlock();

  return 0;

}

/* Find the first chunk with specific flags*/
int FTrace_get_first_chunk(FTrace_flags flags) {

  int i;

  FTrace_lock();
                                                                                                  
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }
                                                                                                    
  for (i = 0;i < MAX_CHUNK;i++) {

    if (ChunkTable[i]->flags & flags) {
      FTrace_unlock();
      return i;
    }

  }

 #ifdef FTRACE_DEBUG
   FTrace_printf("FTrace Error: Free chunk not found\n");
 #endif
 FTrace_unlock();

 return -1;

}

/* Get chunk table */
FTrace_Chunk_Ptr *FTrace_get_chunk_table()
{
                                                                                                    
  return ChunkTable;
                                                                                                    
}

/* Link two chunks */
int FTrace_chunk_link(int chunk_A, int chunk_B)
{

  FTrace_Chunk_Ptr ckA = ChunkTable[chunk_A];
  FTrace_Chunk_Ptr ckB = ChunkTable[chunk_B];

  int err;
                                                                                                                             
  FTrace_lock();
                                                                                                                             
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }
                                                                                                                             
  if (ckA == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk A not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  if (ckB == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk B not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  if (!(ckB->flags & FTRACE_CHUNK_FLAG_FREE)) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk B is not free\n");
    #endif
    FTrace_unlock();
    return -1;
  } else {
    ckB->flags &= ~FTRACE_CHUNK_FLAG_FREE;
    FTrace_OSD_update_chunk_flags(ckB);
  }

  /* FTrace Low Level Blocking Part */
  FTrace_fsave();
    err = FTrace_OSD_chunk_link(ckA,ckB);
    if (err != 0) {
      #ifdef FTRACE_DEBUG
        FTrace_printf("FTrace Error: Cannot link the chunks\n");
      #endif
      return -1;
    }
  FTrace_frestore();

  FTrace_unlock();

  return 0;

}

/* Create a new memory region where the compressed data are stored */
int FTrace_compress_chunk(int number, FTrace_flags new_flags)
{
  void *temp_data;
  FTrace_Chunk_Ptr FT_temp = ChunkTable[number];
  int err, data_size;
  FTrace_Chunk_Ptr New_chunk;

  FTrace_lock();
                                                                                                    
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  FT_temp->flags |= FTRACE_CHUNK_FLAG_BUSY;
  FTrace_OSD_update_chunk_flags(FT_temp);

  FTrace_unlock();

  /* Alloc temp memory for */
  temp_data = (void *)FTrace_malloc(FT_temp->size+FT_temp->emergency_size);
  if (temp_data == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Cannot allocate memory\n");
    #endif
    return -1;
  }

  /* Compress the chunk. Temp_data are a temporary region where
     store the compressed chunk. Data_size is the size of compressed
     data */
  err = FTrace_OSD_compress_chunk(number,temp_data,&data_size);
  if (err != 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: OSD Compressing function failed\n");
    #endif
    return -1;
  }

  New_chunk = (FTrace_Chunk_Ptr)FTrace_malloc(sizeof(struct FTrace_Chunk) + FTRACE_OSD_CHUNK_HEAD + data_size);
  if (New_chunk == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Cannot allocate memory\n");
    #endif
    return -1;
  }

  memcpy(New_chunk,temp_data,data_size);

  FTrace_free(temp_data);

  FTrace_lock();

  /* Free the memory of the old chunk */
  FTrace_free(FT_temp);

  /* Set the new chunk flags and update the main table */
  New_chunk->flags = new_flags;
  New_chunk->flags |= FTRACE_CHUNK_FLAG_COMPR;
  FTrace_OSD_update_chunk_flags(New_chunk);
  ChunkTable[number] = New_chunk;
  
  FTrace_unlock();

  return 0;

}

/* Send the chunk out from the memory */
int FTrace_send_chunk(int number, int osd_flags, FTrace_flags new_flags)
{
  FTrace_Chunk_Ptr FT_temp = ChunkTable[number];
  int err;                                 
                                                                   
  FTrace_lock();
                                                                                                    
  if (FTraceInit == 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: FTrace not initialized\n");
    #endif
    FTrace_unlock();
    return -1;
  }
                                                                                                    
  if (FT_temp == NULL) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Chunk not present\n");
    #endif
    FTrace_unlock();
    return -1;
  }

  FT_temp->flags |= FTRACE_CHUNK_FLAG_BUSY;
  FTrace_OSD_update_chunk_flags(FT_temp);

  FTrace_unlock();

  err = FTrace_OSD_send_chunk(FT_temp, osd_flags);
  if (err != 0) {
    #ifdef FTRACE_DEBUG
      FTrace_printf("FTrace Error: Cannot send the chunk\n");
    #endif
    return -1;
  }

  FTrace_lock();
                                                                                                    
  /* Set the new chunk flags */
  FT_temp->flags = new_flags;
  FTrace_OSD_update_chunk_flags(FT_temp);
                                                                                                    
  FTrace_unlock();
                                                                                                    
  return 0;

}

void FTrace_chunk_dump(int number) {

  FTrace_Chunk_Ptr FT_temp = ChunkTable[number];

  FTrace_printf("ID       : %x\n",FT_temp->id);
  FTrace_printf("NUM      : %d\n",FT_temp->number);
  FTrace_printf("FLAGS    : %x\n",FT_temp->flags);
  FTrace_printf("SIZE     : %d\n",FT_temp->size);
  FTrace_printf("EMERG    : %d\n",FT_temp->emergency_size);

  FTrace_OSD_chunk_dump(FT_temp);

}

