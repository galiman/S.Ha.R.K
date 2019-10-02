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
 * CVS :        $Id: ctype.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/* MODIFIED from GLibc 2.x.x */

/* Copyright (C) 1991,92,93,95,96,97,98,99 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 *	ISO C Standard 4.3: CHARACTER HANDLING	<ctype.h>
 */

#ifndef	_CTYPE_H
#define	_CTYPE_H	1

#include <features.h>
#include <bits/types.h>

#include <ll/ctype.h>

__BEGIN_DECLS

#ifndef _ISbit
/* These are all the characteristics of characters.
   If there get to be more than 16 distinct characteristics,
   many things must be changed that use `unsigned short int's.

   The characteristics are stored always in network byte order (big
   endian).  We define the bit value interpretations here dependent on the
   machine's byte order.  */

# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define _ISbit(bit)	(1 << (bit))
# else /* __BYTE_ORDER == __LITTLE_ENDIAN */
#  define _ISbit(bit)	((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))
# endif

enum
{
  _ISupper  = _ISbit (0),	/* UPPERCASE.  */
  _ISlower  = _ISbit (1),	/* lowercase.  */
  _ISalpha  = _ISbit (2),	/* Alphabetic.  */
  _ISdigit  = _ISbit (3),	/* Numeric.  */
  _ISxdigit = _ISbit (4),	/* Hexadecimal numeric.  */
  _ISspace  = _ISbit (5),	/* Whitespace.  */
  _ISprint  = _ISbit (6),	/* Printing.  */
  _ISgraph  = _ISbit (7),	/* Graphical.  */
  _ISblank  = _ISbit (8),	/* Blank (usually SPC and TAB).  */
  _IScntrl  = _ISbit (9),	/* Control character.  */
  _ISpunct  = _ISbit (10),	/* Punctuation.  */
  _ISalnum  = _ISbit (11)	/* Alphanumeric.  */
};
#endif /* ! _ISbit  */

/* These are defined in ctype-info.c.
   The declarations here must match those in localeinfo.h.

   These point into arrays of 384, so they can be indexed by any `unsigned
   char' value [0,255]; by EOF (-1); or by any `signed char' value
   [-128,-1).  ISO C requires that the ctype functions work for `unsigned
   char' values and for EOF; we also support negative `signed char' values
   for broken old programs.  The case conversion arrays are of `int's
   rather than `unsigned char's because tolower (EOF) must be EOF, which
   doesn't fit into an `unsigned char'.  But today more important is that
   the arrays are also used for multi-byte character sets.  */
extern __const unsigned short int *__ctype_b;	/* Characteristics.  */

#define	__isctype(c, type) \
  (__ctype_b[(int) (c)] & (unsigned short int) type)

#define	__isascii(c)	 (((c) & ~0x7f) == 0)	/* If C is a 7 bit value.  */
#define	__toascii(c)	 ((c) & 0x7f)		/* Mask off high bits.  */

#define isalnum(c)	 __isctype((c), _ISalnum)
#define isalpha(c)	 __isctype((c), _ISalpha)
#define iscntrl(c)	 __isctype((c), _IScntrl)
#define isdigit(c)	 __isctype((c), _ISdigit)
#define islower(c)	 __isctype((c), _ISlower)
#define isgraph(c)	 __isctype((c), _ISgraph)
#define isprint(c)	 __isctype((c), _ISprint)
#define ispunct(c)	 __isctype((c), _ISpunct)
#define isxdigit(c)	 __isctype((c), _ISxdigit)
#define isblank(c)	 __isctype((c), _ISblank)

#define isascii(c)	 __isascii (c)
#define toascii(c)	 __toascii (c)

__END_DECLS

#endif /* ctype.h  */
