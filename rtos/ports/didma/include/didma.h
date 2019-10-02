/*
 * Doubly indexed dynamic memory allocator (DIDMA)
 * Version 0.61
 *
 * Written by Miguel Masmano Tello <mmasmano@disca.upv.es>
 * Copyright (C) Dec, 2002 OCERA Consortium
 * Release under the terms of the GNU General Public License Version 2
 *
 */

#ifndef _INDEXED_MALLOC_H_
#define _INDEXED_MALLOC_H_

#include <kernel/mem.h>
#include <stdio.h>

/*
 * if __DEBUG__ is defined several functions like mem_dump(), 
 * free_blocks_context() will be availables.
 */
//#define __DEBUG__

/* 
 * MAX_FL_INDEX define the max first level number of indexes 
 */
static int MAX_FL_INDEX = 25;

#define PRINT_MSG printk
#define PRINT_DBG_C(message) printk(message)
#define PRINT_DBG_D(message) printk("%i", message);
//#define PRINT_DBG_F(message) rtl_printf("%f", message);
#define PRINT_DBG_H(message) printk("%i", message);

#define SYSTEM_MALLOC(size) (void *) kern_alloc(size)
#define SYSTEM_FREE(ptr) kern_free((void *) ptr, 0);

int init_memory_pool (int max_size);
void destroy_memory_pool(void);

/* see man malloc */
void *rt_malloc (size_t size);

/* see man realloc */
void *rt_realloc(void *p, size_t new_len);

/* see man calloc */
void *rt_calloc(size_t nelem, size_t elem_size);

/*  
 * see man free 
 *
 * rt_free () is only guaranteed to work if ptr is the address of a block
 * allocated by rt_malloc() (and not yet freed). 
 */
void rt_free (void *ptr);

#ifdef __DEBUG__

/* dump_mem () does a dumped of the memory context */
void dump_mem (void);

/* 
 * free_blocks_context () show the content
 * of free blocks
 */
void free_blocks_context(void);

/* 
 * structure_context () gives information about 
 * algorithm structures
 */
void structure_context (void);

/* 
 * check_mem () checks memory searching 
 * errors and incoherences 
 * return :
 * 0 if there isn't any error
 * or 
 * -1 in other case
 */
int check_mem (void);

/* 
 * global_state () returns overhead, free and used space
 */
void global_state (int *free, int *used, int *overhead);

#endif // DEBUG
#endif // #ifndef _INDEXED_MALLOC_H_
