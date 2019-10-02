/*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 *
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

/*
 * malloc.c -- memory allocation related functions
 */

/* $Id: malloc.c,v 1.3 2005/01/08 15:00:28 pj Exp $ */
#ifdef FFTW_USING_CILK
#include <cilk.h>
#include <cilk-compat.h>
#endif

#include <fftw-int.h>
//#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_MALLOC_H
//#include <ports/malloc.h>
#endif

#include <kernel/kern.h>
#define EXIT_FAILURE 1

fftw_malloc_type_function fftw_malloc_hook = 0;
fftw_free_type_function fftw_free_hook = 0;
fftw_die_type_function fftw_die_hook = 0;

/**********************************************************
 *   DEBUGGING CODE
 **********************************************************
 gettato via!!!
*/

/**********************************************************
 *   NON DEBUGGING CODE
 **********************************************************/
/* production version, no hacks */

void *fftw_malloc(size_t n)
{
     void *p;

     if (fftw_malloc_hook)
	  return fftw_malloc_hook(n);

     if (n == 0)
	  n = 1;

     p = malloc(n);

     if (!p)
	  fftw_die("fftw_malloc: out of memory\n");

     return p;
}

void fftw_free(void *p)
{
     if (p) {
	  if (fftw_free_hook) {
	       fftw_free_hook(p);
	       return;
	  }
	  free(p);
     }
}

/* die when fatal errors occur */
void fftw_die(const char *s)
{
     if (fftw_die_hook)
	  fftw_die_hook(s);

//     fflush(stdout);
//     fprintf(stderr, "fftw: %s", s);

     printk(KERN_ERR "fftw: %s", s);
     exit(EXIT_FAILURE);
}

/* check for memory leaks when debugging */
void fftw_check_memory_leaks(void)
{
     extern int fftw_node_cnt, fftw_plan_cnt, fftw_twiddle_size;

#ifdef FFTW_DEBUG
     if (fftw_malloc_cnt || fftw_malloc_total ||
	 fftw_node_cnt || fftw_plan_cnt || fftw_twiddle_size) {
//	  fflush(stdout);
//	  fprintf(stderr,
          kern_printf(
		  "MEMORY LEAK!!!\n"
		  "fftw_malloc = %d"
		  " node=%d plan=%d twiddle=%d\n"
		  "fftw_malloc_total = %d\n",
		  fftw_malloc_cnt,
		  fftw_node_cnt, fftw_plan_cnt, fftw_twiddle_size,
		  fftw_malloc_total);
	  exit(EXIT_FAILURE);
     }
#else
     if (fftw_node_cnt || fftw_plan_cnt || fftw_twiddle_size) {
//	  fflush(stdout);
//	  fprintf(stderr,
          kern_printf(
		  "MEMORY LEAK!!!\n"
		  " node=%d plan=%d twiddle=%d\n",
		  fftw_node_cnt, fftw_plan_cnt, fftw_twiddle_size);
	  exit(EXIT_FAILURE);
     }
#endif
}

void fftw_print_max_memory_usage(void)
{
#ifdef FFTW_DEBUG
     printf("\nMaximum number of blocks allocated = %d\n"
	    "Maximum number of bytes allocated  = %0.3f kB\n",
	    fftw_malloc_cnt_max, fftw_malloc_max / 1024.0);
#endif
}
