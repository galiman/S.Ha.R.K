/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: stdlib.h,v 1.4 2005/01/08 14:54:09 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.4 $
 Last update: $Date: 2005/01/08 14:54:09 $
 ------------

 stdlib.h

**/

/*
 * Copyright (C) 2000 Paolo Gai
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

#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <ll/stdlib.h>
#include <sys/types.h>
#include <stddef.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define qsort _quicksort

void abort(void);
int atexit(void (*function)(void));
void *calloc(size_t nelt, size_t eltsize);
void free(void *ptr);
void *malloc (size_t size);
void *realloc(void *ptr, size_t size);

/* This is not the correct behaviour of the function. Maybe someone will fix
   it; see kernel/kern.c */
void exit(int status);
void _exit(int status);

/* to implement!! */
//char *getenv(const char *);

/* not standard but required! */
long strtol (const char *, char **, int);
unsigned long strtoul (const char *, char **, int);

/* StdLib QSort */
typedef int (*__compar_fn_t) (const void *, const void *);
extern void _quicksort (void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp);
extern void * bsearch (const void *key, const void *base, size_t nmemb, size_t size, int (*compar) (const void *, const void *));

__END_DECLS
#endif
