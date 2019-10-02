/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
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
 * CVS :        $Id: assert.c,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:51 $
 */

#include <ll/i386/cons.h>
#include <ll/stdarg.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/func.h>
#include <kernel/var.h>
#include <kernel/assert.h>

void _failed_assertk(char *test,char *filename, int line)
{
  sys_panic("assertion %s failed in %s at line %i (task:%i_%i)\n",
	test,filename,line,0,(int)exec_shadow);
}

void _failed_magic_assert(char *filename, int line, char *fmt, ...)
{
  static char buffer[1024]; /* DANGER!!! */
  va_list ap;
  
  va_start(ap,fmt);
  vsprintf(buffer,(char*)fmt,ap);
  va_end(ap);

  sys_panic("MAGIC assertion failed in %s at line %i (task:%i_%i): %s\n",
	filename,line,0,exec,buffer);
}
