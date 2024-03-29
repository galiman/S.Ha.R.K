/* $Id: common_x86.c,v 1.1 2003/03/13 12:11:48 giacomo Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
 *
 * Copyright (C) 1999-2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Check CPU capabilities & initialize optimized funtions for this particular
 * processor.
 *
 * Written by Holger Waechtler <holger@akaflieg.extern.tu-berlin.de>
 * Changed by Andre Werthmann <wertmann@cs.uni-potsdam.de> for using the
 * new Katmai functions.
 */

#include <stdlib.h>
#include <stdio.h>
#if defined(USE_SSE_ASM) && defined(__linux__)
#include <signal.h>
#endif
#if defined(USE_SSE_ASM) && defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "context.h"
#include "common_x86_asm.h"
#include "imports.h"


int _mesa_x86_cpu_features = 1;

void _mesa_init_all_x86_transform_asm( void )
{
   
   _mesa_x86_cpu_features = 1;

   if ( _mesa_x86_cpu_features ) {
      _mesa_init_x86_transform_asm();
   }

   _mesa_x86_cpu_features &= ~(X86_FEATURE_XMM);

}

