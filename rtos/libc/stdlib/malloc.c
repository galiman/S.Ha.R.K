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
 CVS :        $Id: malloc.c,v 1.2 2003/09/12 10:11:27 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/09/12 10:11:27 $
 ------------

 malloc:
 the size of the block is stored into the first 4 bytes

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

#include <kernel/func.h>

void *malloc (size_t size)
{
  char *p;
  SYS_FLAGS f;

  f = kern_fsave();

  p = kern_alloc(size + sizeof(size_t));
  if (!p) {
    kern_frestore(f);
    return 0;
  }

  *((size_t *) p) = size + sizeof(size_t);

  kern_frestore(f);

  return p + sizeof(size_t);
}

