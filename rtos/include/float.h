/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)float.h	7.1 (Berkeley) 5/8/90
 *	$Id: float.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 */

/* modified for S.Ha.R.K.! */

#ifndef _MACHINE_FLOAT_H_
#define _MACHINE_FLOAT_H_ 1

#include <ll/limits.h>
#include <ll/float.h>

#define LDBL_MANT_DIG	DBL_MANT_DIG
#define LDBL_EPSILON	DBL_EPSILON
#define LDBL_DIG	DBL_DIG
#define LDBL_MIN_EXP	DBL_MIN_EXP
#define LDBL_MIN	DBL_MIN
#define LDBL_MIN_10_EXP	DBL_MIN_10_EXP
#define LDBL_MAX_EXP	DBL_MAX_EXP
#define LDBL_MAX	DBL_MAX
#define LDBL_MAX_10_EXP	DBL_MAX_10_EXP

#endif /* _MACHINE_FLOAT_H_ */
