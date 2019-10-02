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


/*
 * Copyright (C) 1999 Massimiliano Giorgi
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
 * CVS :        $Id: assert.h,v 1.2 2003/03/13 13:36:27 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:36:27 $
 */

#ifndef _KERNEL_ASSERT_H
#define _KERNEL_ASSERT_H

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/**/

#ifdef NDEBUG

#define assertk(test) {}

#else

void _failed_assertk(char *test,char *filename, int line);

#define assertk(test) \
  { if (!(test)) _failed_assertk(#test,__FILE__,__LINE__); }

#endif

/**/
	
#ifdef _PARANOIA

void _failed_magic_assert(char *filename, int line, char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
	
#define magic_assert(value, magic, format, args...)               \
  {                                                               \
    if ((value)!=(magic))                                         \
      _failed_magic_assert(__FILE__,__LINE__,format, ##args); \
  }

#define magic_assert_ext(value, magic, fun, format, args...)      \
  {                                                               \
    if ((value)!=(magic)) {                                       \
      fun();                                                      \
      _failed_magic_assert(__FILE__,__LINE__,format, ##args); \
    }                                                             \
  }

#else

#define magic_assert(value, magic, format, args...) {}
#define magic_assert_ext(value, magic, fun, format, args...) {}

#endif

/**/
__END_DECLS
#endif
