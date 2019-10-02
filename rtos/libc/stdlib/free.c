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
 CVS :        $Id: free.c,v 1.2 2004/05/07 10:09:10 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2004/05/07 10:09:10 $
 ------------

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
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

void free(void *ptr)
{
  if (ptr) {
    char *p = ptr;
    SYS_FLAGS f;

    f = kern_fsave();
    //kern_cli();
    p -= sizeof(size_t);
    if (*(size_t *)p == 0) {
	kern_frestore(f);
	return;
    }
    kern_free(p,*((size_t *)p));
    //kern_sti();
    kern_frestore(f);
  }
}
