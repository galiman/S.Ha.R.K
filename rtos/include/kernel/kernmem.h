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
 CVS :        $Id: kernmem.h,v 1.2 2003/03/13 13:36:28 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2003/03/13 13:36:28 $
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
#ifndef __KERNEL_KERNMEM_H_
#define __KERNEL_KERNMEM_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS
/* These function are used to manage memory at VM layer */

void * kern_alloc(DWORD size);
WORD kern_free(void *ptr,DWORD size);
void kern_mem_init(void *base,DWORD size);
void kern_mem_dump(void);

__END_DECLS
#endif /* __KERNEL_KERNMEM_H__ */
