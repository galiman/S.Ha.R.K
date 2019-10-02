/*
 * Doubly indexed dynamic memory allocator (DIDMA)
 * Version 0.61
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) Dec, 2002 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 * shark porting Trimarchi Michael
 */

#include <kernel/func.h>
#include "didma.h"
#include "queuem.h"
#include "i386/bitops.h"
#include "i386/bits.h"

#ifndef NULL
#define NULL ((void *) 0)
#endif // #ifndef NULL


#define INIT_THREAD_MUTEX()
#define THREAD_LOCK()	kern_cli()
#define THREAD_UNLOCK()	kern_sti()

// u8, u16, u32 are only defined in kernel headers

#define u8 char 
#define u16 unsigned short int 
#define u32 unsigned int 

#define MAX_SL_LOG2_INDEX 5 // Real value is 2**MAX_SL_INDEX

#define MAX_SL_INDEX (1 << MAX_SL_LOG2_INDEX)

#define F_OK 0
#define F_ERROR -1

#define MIN_LOG2_SIZE 5 // 32 bytes
#define MAX_LOG2_SIZE MAX_FL_INDEX
#define MIN_SIZE (1 << MIN_LOG2_SIZE)
#define MAX_SIZE (1 << MAX_LOG2_SIZE)

#define DIDMA__set_bit(num, mem) mem |= (1 << num)
#define DIDMA__clear_bit(num, mem) mem &= ~(1 << num)

/*
 * First, all necessary TADs are defined 
 */


/*
 * the follow TAD will be a double level indexed array, the most important
 * thing is that the time is limited, it is because this dynamic memory manger
 * is designed to run with real time programs.
 *
 *     First level       Second level
 *     it is indexed     it is indexed by 2**n+(2**n/m*index_number)
 *     by power of 2        
 *                            0             1     m-1        m
 *                            ----> NULL   --> NULL          ----> NUL
 *                            |            |                 |
 *       -------         ---------------------...---------------------
 *  2**n |  n  |  -----> |2**n+(2**n/m*0)|...|   |...|2**n+(2**n/m*m)|
 *       |-----|         ---------------------...---------------------
 * 2**n-1| n-1 |  -----> ....                      |             
 *       |-----|                                   --->NULL
 *        .....
 */

/* DON'T TOUCH THESE MACROS */
#define MAGIC_NUMBER 0xA5A5
#define MAGIC_NUMBER_NONE 0x0000
#define BLOCK_FREE 0
#define BLOCK_USED 1


struct free_ptr_struct {
  struct block_header_struct *prev;
  struct block_header_struct *next;
  /*
   * first_l_index and second_l_index are used to store
   * mapping_function results, that's how we get some extra 
   * nanoseconds
   */
  u8 first_l_index;
  u8 second_l_index;
};

typedef struct block_header_struct {
  /* magic_number is the id of the block */
  u16 magic_number;
  /* size of the block */
  size_t size;
  /* state of the block can be free or used */
  u8 state;

  struct block_header_struct *prev;
  struct block_header_struct *next;
  
  union {
    struct free_ptr_struct free_ptr;
    u8 buffer[sizeof(struct free_ptr_struct)];
  } ptr;
} block_header_t;

/* first level index array */
typedef struct fl_array_struct {
  /* ptr is pointer to next level */
  block_header_t **sl_array;
  /* n_blocks is the number of the blocks which they are pointed by sl_array */
  u32 bitmapSL;
} fl_array_t;

/*
 * fl_array will be our first level array.
 */

static fl_array_t *fl_array;// [MAX_FL_INDEX - MIN_LOG2_SIZE];
u32 bitmapFL;
/*
 * first_bh will be our first memory block allocated by MALLOC function
 */
static block_header_t *first_bh, *free_block;

static int all_ok = 0;

/* 
 * header_overhead has size of blocks_header - 2 * ptr to next free block
 */
static int header_overhead = 0, total_size = 0;

/*
 * log2size () returns cell of log2 (len)
 * it does a search between MIN_SIZE and MAX_SIZE values
 */
static int log2size(size_t size, size_t *new_size)
{
  int i;
  
  if (size <= 0) {
    PRINT_MSG ("ERROR: Chunk length must be > 0\n");
    return F_ERROR;
  }
  
  
  // MIN_SIZE and MAX_SIZE must be defined  
  
  if (size <= MIN_SIZE){
    *new_size = MIN_SIZE;
    return MIN_LOG2_SIZE;
  }
  
  for (i = MIN_LOG2_SIZE; 
       (i <= MAX_LOG2_SIZE) && (size > (*new_size = (1 << i))) ; i++);

  if (i > MAX_LOG2_SIZE) {
    PRINT_MSG ("ERROR: Maximum chunk size exceeded\n");
    return F_ERROR;
  }
 
  return i;
}

/*
 * caculate_reserve () returns the size that has to be reservate for 
 * avoid fragmentation
 */

#define MAX(a, b) (a>b)?a:b

/*
  May be calculate_reserve can be improved
*/

static inline int calculate_reserve (int size){
  //  int internal_frag, dummy;

  //  header_overhead = (int) first_bh -> ptr.buffer - (int) first_bh;  

  //internal_frag = (1 << log2size(size, &dummy)) / MAX_SL_INDEX;

  //dummy = MAX((header_overhead * size / MAX_SL_INDEX) + internal_frag, size);

  return size;//(size + dummy);

}

/*
 * mapping_function () returns first and second level index
 * 
 * first level index function is:
 * fl = log2 (size)
 *
 * and second level index function is:
 * sl = (size - 2**(log2size (size))) / (log2 (size) - log2 (MAX_SL_INDEX))
 *
 */
static int mapping_function (size_t size, int *fl, int *sl){
  int aux;
  int new_len;
 
  // first level = log2 (size)
  *fl = log2size (size, (size_t *)&new_len);
   
  if (new_len == size) {
    //  if 2**fl == size, second level must be 0
    *sl = 0;
  } else { 
    -- *fl;
    aux = *fl - MAX_SL_LOG2_INDEX;
    *sl = (int)((size - (new_len >> 1)) >> aux);
  }
  
  *fl -= MIN_LOG2_SIZE;
  return F_OK;
}

/* 
 * init_TAD () initialize the structure fl_array to NULL
 * and free_block with the initial blocks pointed by ptr
 *
 * its cost is O (n x m)
 */
static int init_TAD (size_t size, u8 *ptr){
  int n, aux, dummy;
  if (!(size > 0)) {
    PRINT_MSG ("ERROR: size must be > 0\n");
    return F_ERROR;
  }
  if (MAX_SL_INDEX > MIN_SIZE) {
    PRINT_MSG ("ERROR: MAX_SL_INDEX must be < MIN_SIZE\n");
    return F_ERROR;
  }

  total_size = size;

  MAX_FL_INDEX = log2size(size, (size_t *)&dummy);

  if (MAX_FL_INDEX < 0) return -1;
  fl_array = (fl_array_t *) SYSTEM_MALLOC ((MAX_FL_INDEX - MIN_LOG2_SIZE)
					   * sizeof(fl_array_t));
  if (fl_array == NULL) return -1;
  _init_malloc_ (20);
  for (n = 0; n < MAX_FL_INDEX - MIN_LOG2_SIZE; n++) {
    fl_array [n].bitmapSL = 0;
    aux = (1 << (n + MIN_LOG2_SIZE));
    fl_array[n].sl_array = NULL;
  }
  bitmapFL = 0;
  
  header_overhead = (int) first_bh -> ptr.buffer - (int) first_bh;
  first_bh = (block_header_t *) ptr;

  first_bh -> magic_number = MAGIC_NUMBER; 
  first_bh -> size = size - header_overhead;
  first_bh -> state = BLOCK_FREE;
  first_bh -> prev = NULL;
  first_bh -> next = NULL;
  first_bh -> ptr.free_ptr.prev = NULL;
  first_bh -> ptr.free_ptr.next = NULL;

  free_block = first_bh;
  //mapping_function (first_bh->size,&n, &m);
  //fl_array[n].sl_array[m].ptr= first_bh;
  //DIDMA__set_bit (m, fl_array[n].bitmapSL);
  //DIDMA__set_bit (n, bitmapFL);
  all_ok = 1;
  return F_OK;
}

/* 
 * destroy_TAD return a pointer to the initial block 
 */
static void *destroy_TAD (void){
  all_ok = 0;
  _destroy_malloc_();
  return (void *) first_bh;
}

/*
 * insert_TAD merge a free block pointed by ptr 
 * with its buddies and then 
 * insert it into fl_array
 *
 * The cost of this operation is
 * always O (K) = (1)
 */
static int insert_TAD (u8 *ptr){
  int fl, sl;
  block_header_t *bh = NULL, *bh2;
  int for_free_block = 0;

  if (!all_ok) return F_ERROR;
 
  bh = (block_header_t *) (ptr - header_overhead);
  if (bh -> magic_number != MAGIC_NUMBER)
    return F_ERROR;
 
  THREAD_LOCK();
  /*
   * now bh is a free block
   */
  bh -> state = BLOCK_FREE;
  bh -> ptr.free_ptr.prev = NULL;
  bh -> ptr.free_ptr.next = NULL;
 
  /*
   * first bh will be merge with its free buddies and 
   * then it will be inserted with the free blocks
   */
  /* 
   * it is used to know if bh have to be inserted into free_block or 
   * into fl_array
   */
  if (bh -> next == NULL) for_free_block = 1;
 
  /* is it free the follow block? */
  if (bh -> next != NULL) {
    bh2 = bh -> next;
   
    if (bh2 -> magic_number == MAGIC_NUMBER && 
	bh2 -> state == BLOCK_FREE) {
      /* we are lucky, we can merge bh with the next block */
      
      if (bh2 == free_block) for_free_block = 1;
      bh2 -> magic_number = MAGIC_NUMBER_NONE;
      if (bh2 -> next != NULL)
	bh2 -> next -> prev = bh;
      if (!for_free_block) {
	if (bh2 -> ptr.free_ptr.next != NULL)
	  bh2 -> ptr.free_ptr.next -> ptr.free_ptr.prev = 
	    bh2  -> ptr.free_ptr.prev;
	
	if (bh2 -> ptr.free_ptr.prev != NULL)
	  bh2 -> ptr.free_ptr.prev -> ptr.free_ptr.next = 
	    bh2 -> ptr.free_ptr.next;
	//mapping_function (bh2 -> size, &fl, &sl);
	fl = bh2 -> ptr.free_ptr.first_l_index;
	sl = bh2 -> ptr.free_ptr.second_l_index;
	/* bh2 must be erased from fl_array */
	if (fl_array [fl].sl_array [sl] == bh2)
	  fl_array [fl].sl_array [sl] = bh2 -> ptr.free_ptr.next;

	  //fl_array [fl].n_blocks --;
	  if (fl_array [fl].sl_array [sl] == NULL){
	    //fl_array[fl].bitmapSL &= ~(1 << sl);
	    DIDMA__clear_bit (sl, fl_array[fl].bitmapSL);
	    if (!fl_array[fl].bitmapSL) {
	      _free_ ((void *) fl_array [fl].sl_array);
	      DIDMA__clear_bit (fl, bitmapFL);
	    }
	  }
      }
      bh -> size += bh2 -> size + header_overhead;
      bh -> next = bh2 -> next;
    }
  }

  /* is it free the previous block? */
  if (bh -> prev != NULL) {
    bh2 = bh -> prev;
    if (bh2 -> magic_number == MAGIC_NUMBER && 
      bh2 -> state == BLOCK_FREE) {
      bh -> magic_number = MAGIC_NUMBER_NONE;
      
      if (bh -> next != NULL)
	bh -> next -> prev = bh2;

      if (bh2 -> ptr.free_ptr.next != NULL)
	bh2 -> ptr.free_ptr.next -> ptr.free_ptr.prev = 
	  bh2  -> ptr.free_ptr.prev;
      
      if (bh2 -> ptr.free_ptr.prev != NULL)
	bh2 -> ptr.free_ptr.prev -> ptr.free_ptr.next = 
	  bh2 -> ptr.free_ptr.next;
 
      //mapping_function (bh2 -> size, &fl, &sl);
      
      fl = bh2 -> ptr.free_ptr.first_l_index;
      sl = bh2 -> ptr.free_ptr.second_l_index;
      if (fl_array [fl].sl_array [sl] == bh2)
	fl_array [fl].sl_array [sl] = bh2 -> ptr.free_ptr.next;

      if (fl_array[fl].sl_array[sl] == NULL){
	//fl_array[fl].bitmapSL &= ~(1 << sl);
	DIDMA__clear_bit (sl, fl_array[fl].bitmapSL);
	if (!fl_array[fl].bitmapSL) {
	  _free_ ((void *) fl_array [fl].sl_array);
	  DIDMA__clear_bit (fl, bitmapFL);
	}
      }
    
      bh2 -> size += bh -> size + header_overhead;
      bh2 -> next = bh -> next;
      bh = bh2;
    
    }
  }
 
  /*
   * and then we merge the free block with the initial memory
   */
    if (for_free_block) {
      free_block = bh;
      bh -> ptr.free_ptr.next = NULL;
      bh -> ptr.free_ptr.prev = NULL;
    } else {
      /*
       * or we insert the free block in the TAD
       */
      mapping_function (bh -> size, &fl, &sl);
      if (fl_array[fl].bitmapSL == 0)
	fl_array[fl].sl_array = (block_header_t **) _malloc_ ();
      if (fl_array[fl].sl_array == NULL) {
	PRINT_MSG("CRITICAL ERROR: there isn't enought memory for free\n");
	THREAD_UNLOCK();
        return F_ERROR;
      }
      bh -> ptr.free_ptr.first_l_index = fl;
      bh -> ptr.free_ptr.second_l_index = sl;
      bh -> ptr.free_ptr.next = fl_array [fl].sl_array [sl];
      bh -> ptr.free_ptr.prev = NULL;
      if (fl_array [fl].sl_array [sl] != NULL)
	fl_array [fl].sl_array [sl] -> ptr.free_ptr.prev = bh;
      fl_array [fl].sl_array [sl] = bh;
    
      DIDMA__set_bit (sl, fl_array[fl].bitmapSL);
      DIDMA__set_bit (fl, bitmapFL);
      
    }

  THREAD_UNLOCK();
  return F_OK;
}

/*
 * search_TAD searchs a free block of size 'size'
 * then this block will be splitted in two new blocks,
 * one of these new blocks will be given to the user and the
 * other will be inserted into a free blocks structure
 *
 * The cost of this operation is
 *      best case: (K) = (1)  
 *      worst case: (MAX_FL_LOG2_INDEX - MIN_FL_LOG2_INDEX + MAX_SL_INDEX + K) 
 *                   = (1)
 * where K is an integer constant
 */

static u8 *search_TAD (size_t size) {
  int for_free_block = 0;
  int fl, sl, n;
  int found;
  block_header_t *bh = NULL, *bh2;

  if (!(size > 0) || size >= MAX_SIZE || !all_ok) return NULL;
  THREAD_LOCK();

  if (size <= MIN_SIZE) {
    size = MIN_SIZE;
    fl = 0;
    sl = 0;
  } else {
    mapping_function (size, &fl, &sl);
  
    if (++sl == MAX_SL_INDEX) {
      fl ++;
      sl = 0;
    }
    /*
     * This is the reason of the internal fragmentation
     * The block given is greater that the size demanded
     */
    size = (1 << (fl + MIN_LOG2_SIZE));
    size = size + ((size >> MAX_SL_LOG2_INDEX) * sl);
  }
   
  /* the size given can be bigger than the size demanded */
  found = 0;
  
  // we take the first free block from fl_array or a buddy of him

  sl = fl_array[fl].bitmapSL & ((~0) << sl);
  if (sl != 0) { 
    sl = DIDMA_fls(sl);
    bh = fl_array [fl].sl_array [sl];
    fl_array [fl].sl_array [sl] = bh -> ptr.free_ptr.next;
    if (fl_array [fl].sl_array [sl] != NULL)
      fl_array [fl].sl_array [sl] -> ptr.free_ptr.prev = NULL;
    else {
      DIDMA__clear_bit (sl, fl_array[fl].bitmapSL);
      if (!fl_array[fl].bitmapSL) {
	DIDMA__clear_bit (fl, bitmapFL);
	_free_ ((void *) fl_array [fl].sl_array);
      }
    }
    found = 1;
    goto out;
  }
  /* 
   * if fl_array is empty we will take a free block 
   * from free_block pointer
   */
  
  if (free_block != NULL && free_block -> size >= size) {
    bh = free_block;
    free_block = NULL;
    for_free_block = 1;
    found = 1;
    goto out;
  }
  
  /* 
   * A free block is searched
   *
   * Using a bitmap
   */

  fl = DIDMA_fls(bitmapFL & ((~0) << (fl + 1)));
  
  if (fl > 0) {
    sl = DIDMA_fls(fl_array[fl].bitmapSL);
    bh = fl_array [fl].sl_array [sl];
    fl_array [fl].sl_array [sl] = bh -> ptr.free_ptr.next;
    if (fl_array [fl].sl_array [sl] != NULL){
      fl_array [fl].sl_array [sl] -> ptr.free_ptr.prev = NULL;
    } else {
      DIDMA__clear_bit (sl, fl_array[fl].bitmapSL);
      if (!fl_array[fl].bitmapSL) {
	_free_ ((void *) fl_array[fl].sl_array);
	DIDMA__clear_bit (fl, bitmapFL);
      }
    }
    found = 1;
    goto out;
  }

 out:
  
  /* 
   * HUGGGG, NOT ENOUGHT MEMORY 
   * I think that we have done all that we have been able
   */
  if (!found) {
    PRINT_MSG ("ERROR: Memory pool exhausted!!!\n");
    THREAD_UNLOCK();
    return NULL;
  }
  
  /*
   * we can say: YESSSSSSSSSSS, we have enought memory!!!!
   */
  bh -> state = BLOCK_USED;

  /* can bh be splitted? */ 
  n = (int)(bh -> size - size - header_overhead);
  if (n >= (int) MIN_SIZE) {

    /* 
     * Yes, bh will be splitted
     */

    bh2 = (block_header_t *) ((u8 *)(bh -> ptr.buffer) + size);    
    bh2 -> magic_number = MAGIC_NUMBER;
    bh2 -> state = BLOCK_FREE;
    bh2 -> size = n;
    if (bh -> next != NULL)
      bh -> next -> prev = bh2;
    bh2 -> next = bh -> next;
    bh -> next = bh2;
    bh2 -> prev = bh;
    bh -> size = size;

    if (!for_free_block) {
      mapping_function (bh2 -> size, &fl, &sl);
      if (fl_array[fl].bitmapSL == 0)
	fl_array[fl].sl_array = (block_header_t **) _malloc_ ();
      if (fl_array[fl].sl_array == NULL) {
	PRINT_MSG("CRITICAL ERROR: there isn't enought memory for free\n");
	return NULL;
      }
      bh2 -> ptr.free_ptr.first_l_index = fl;
      bh2 -> ptr.free_ptr.second_l_index = sl;
      bh2 -> ptr.free_ptr.prev = NULL;
      bh2 -> ptr.free_ptr.next = fl_array [fl].sl_array [sl];
      
      if (fl_array [fl].sl_array [sl] != NULL)
	fl_array [fl].sl_array [sl] -> ptr.free_ptr.prev = bh2;
      fl_array [fl].sl_array [sl] = bh2;
      DIDMA__set_bit (sl, fl_array[fl].bitmapSL);
      DIDMA__set_bit (fl, bitmapFL);
      
       	//fl_array [fl].n_blocks ++;
    } else 
      free_block = bh2;
  }
  THREAD_UNLOCK();
  return (u8 *) bh -> ptr.buffer;
}


int init_memory_pool (int max_size){
  
  u8 *buffer;
  int real_max_size = 0;

  if (max_size < 0) return -1;


  //buffer = (u8 *) SYSTEM_MALLOC (real_max_size * sizeof (u8));
  real_max_size = calculate_reserve(max_size);
  buffer = (u8 *) SYSTEM_MALLOC (real_max_size * sizeof (u8));
  if (buffer == NULL) return F_ERROR;
  return init_TAD (real_max_size, buffer);
}

void destroy_memory_pool(void){
  
	SYSTEM_FREE (destroy_TAD ());
}

/* see 'man malloc' */
void *rt_malloc (size_t size){
  return (void *) search_TAD (size);
}

/* see 'man free' */
void rt_free (void *ptr){
  if (ptr != NULL) insert_TAD ((u8 *) ptr);
}

/* see 'man calloc' */
void *rt_calloc (size_t nelem, size_t elem_size)
{
  void *p;

  if (!all_ok || nelem <= 0 || elem_size <= 0) return NULL;

  if ((p = rt_malloc (nelem * elem_size)) == NULL)
    return NULL;

  memset (p, 0, nelem * elem_size);

  return p;
}

/* 
 * see man realloc  
 * be careful, realloc () is an expensive operation because
 * it uses malloc () and free ()
 */
void *rt_realloc (void *p, size_t new_len)
{
  u8 *aux;
  void *addr;
  int old_len, min;
  block_header_t *b;

  if (!all_ok) return NULL;

  if (p == NULL)
    return rt_malloc (new_len);
  else if (new_len == 0) {
    rt_free(p);
    return NULL;
  } else {
    /* 
     * Now we try to achieve old size because
     * then we need it to copy all data in the new allocated
     * memory
     */
    aux = (u8 *) p;
    aux -= header_overhead;
    b = (block_header_t *) aux;
    if (b -> magic_number == MAGIC_NUMBER) {
      old_len = b -> size;
    } else {
      PRINT_MSG ("CRITICAL ERROR: block corrupted\n");
      return NULL;
    }
    
    addr = rt_malloc (new_len);

    if (addr == NULL) return NULL;
    
    min = (old_len > new_len)? new_len : old_len;
    memcpy (addr, p, min);
    rt_free (p);

    return addr;
  }
}

#ifdef __DEBUG__

/*
 * dump_mem () is a simple operation, 
 * it only does a dumped of memory
 */

void dump_mem (void){
  u8 *aux;
  int n;

  if (!all_ok) return;

  aux = (u8 *) first_bh;

  for (n = 0; n < total_size; n++) {
    PRINT_DBG_H (aux [n]);
    PRINT_DBG_C (" ");
  }
  PRINT_DBG_C ("\n");
}

/*
 * I haven't anything to say about print_block ()
 */
static void print_block (block_header_t *b){
  if (b == NULL) return;
  PRINT_DBG_C (">>>> MNum 0x");
  PRINT_DBG_H (b -> magic_number);
  PRINT_DBG_C (" Address 0x");
  //PRINT_DBG_H (b);
  if (b -> state == BLOCK_FREE) 
    PRINT_DBG_C (" State FREE");
  else 
    PRINT_DBG_C (" State USED");
  PRINT_DBG_C (" Size ");
  PRINT_DBG_D (b -> size);
  PRINT_DBG_C ("\n---- Prev 0x");
  //PRINT_DBG_H (b -> prev);
  PRINT_DBG_C (" Next 0x");
  //PRINT_DBG_H (b -> next);
  if (b -> state == BLOCK_FREE){
    PRINT_DBG_C ("\n---- Prev Free 0x");
    //PRINT_DBG_H (b -> ptr.free_ptr.prev);
    PRINT_DBG_C (" Next Free 0x");
    //PRINT_DBG_H (b -> ptr.free_ptr.next);
  }
  PRINT_DBG_C ("\n");
}

/*
 * structure_context () show the content of all blocks
 */
void structure_context (void) {
  block_header_t *b;

  if (!all_ok) return;

  b = first_bh;
  PRINT_DBG_C ("\nALL BLOCKS\n");
  while (b != NULL) {
    print_block (b);
    b = b -> next;
  }

}

/* 
 * global_state () returns overhead, free and used space
 */
void global_state (int *free, int *used, int *overhead){
  block_header_t *b;

  *free = 0;
  *used = 0;
  *overhead = 0;
  if (!all_ok) return; 
  b = first_bh;
  while (b != NULL) {
    if (b -> state == BLOCK_USED) {
      *used += b -> size;
    } else {
      *free += b -> size;
    }
    b = b -> next;
    *overhead += header_overhead;
  }

}

/* 
 * free_blocks_context () show the content
 * of free blocks
 */
void free_blocks_context (void){
  int i, j;
  block_header_t *b;
  
  if (!all_ok) return;
  
  PRINT_DBG_C ("\nFREE BLOCKS\n");
  PRINT_DBG_C ("MAIN BLOCK\n");
  print_block (free_block);
  PRINT_DBG_C ("FRAGMENTED BLOCKS\n");
  for (i = MAX_FL_INDEX - 1 - MIN_LOG2_SIZE; i >= 0; i--) {
    if (fl_array [i].bitmapSL > 0) 
      for (j = MAX_SL_INDEX - 1; j >= 0; j--) {
	if (fl_array [i].sl_array[j] != NULL) {
	  b = fl_array [i].sl_array [j];
	  PRINT_DBG_C ("[");
	  PRINT_DBG_D (i);
	  PRINT_DBG_C ("] ");
	  PRINT_DBG_D (1 << (i + MIN_LOG2_SIZE));
	  PRINT_DBG_C (" bytes -> Free blocks: 0x");
	  PRINT_DBG_H (fl_array [i].bitmapSL);
	  PRINT_DBG_C ("\n");
	
	  while (b != NULL) {
	    PRINT_DBG_C (">>>> ");
	    PRINT_DBG_D ((1 << (i + MIN_LOG2_SIZE)) +
			 (1<< (i + MIN_LOG2_SIZE)) / MAX_SL_INDEX * j);
	    PRINT_DBG_C (" bytes\n");
	    print_block (b);
	    b = b -> ptr.free_ptr.next;
	  }
	}
      }
  }
}

/* 
 * check_mem () checks memory searching 
 * errors and incoherences 
 * return :
 * 0 if there isn't any error
 * or 
 * -1 in other case
 */
int check_mem (void){
  block_header_t *b, *b2;
  int i, j, num_blocks;

  if (!all_ok) return F_ERROR;

  b = first_bh;
  num_blocks = 0;
  for (i = MAX_FL_INDEX - 1 - MIN_LOG2_SIZE; i >= 0; i--) {
    if (fl_array [i].bitmapSL < 0) return F_ERROR;
    for (j = MAX_SL_INDEX - 1; j >= 0; j--) {
      b2 = fl_array [i].sl_array[j];
      while (b2 != NULL) {
	b2 = b2 -> ptr.free_ptr.next;
	num_blocks ++;
      }
    }
    if (num_blocks != fl_array [i].bitmapSL) return F_ERROR;
    num_blocks = 0;
  }
   
  while (b != NULL) {
    if (b -> magic_number != MAGIC_NUMBER || 
	b -> size > MAX_SIZE ||	b -> size < MIN_SIZE) 
      return F_ERROR;

    if (b -> next != NULL)
      if (b -> next < first_bh || b -> next > 
	  (block_header_t *) ((u8 *) (first_bh) + MAX_SIZE)) 
	return F_ERROR;

    if (b -> prev != NULL)
      if (b -> prev < first_bh || b -> prev > 
	  (block_header_t *) ((u8 *) (first_bh) + MAX_SIZE) )
	return F_ERROR;
    
    if (b -> state != BLOCK_FREE && b -> state != BLOCK_USED) return F_ERROR;
    if (b -> state == BLOCK_FREE) {
      if (b -> ptr.free_ptr.next != NULL)
	if (b -> ptr.free_ptr.next < first_bh ||
	    b -> ptr.free_ptr.next > 
	    (block_header_t *) ((u8 *) (first_bh) + MAX_SIZE))
	  return F_ERROR;
	
      if (b -> ptr.free_ptr.prev != NULL)
	if (b -> ptr.free_ptr.prev < first_bh ||
	    b -> ptr.free_ptr.prev > 
	    (block_header_t *) ((u8 *) (first_bh) + MAX_SIZE) )
	    return F_ERROR;
    }

    b = b -> next;
   
  }
  return F_OK;
}
#endif // #ifdef __DEBUG__
