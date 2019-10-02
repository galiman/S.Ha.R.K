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
 * CVS :        $Id: printk.c,v 1.7 2004/01/12 18:26:38 giacomo Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.7 $
 * Last update: $Date: 2004/01/12 18:26:38 $
 */

#include <ll/i386/cons.h>
#include <ll/stdarg.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/log.h>
#include <kernel/func.h>

static int printklevel=0;

#define NO_LEVEL 100

static char *levelname[]={
  "emerg ",
  "alert ",
  "crit  ",
  "err   ",
  "warn  ",
  "notice",
  "info  ",
  "debug "
};

int vprintk(int flag, char *fmt, va_list ap)
{
  static char buf[1024]; /* DANGER !!!!! */
  int level;
  
  level = NO_LEVEL;
  if (*fmt == '<' && *(fmt+2) == '>')
    level = *(fmt+1) - '0';
  if (level != NO_LEVEL) {
    if (level<LOG_EMERG||level>LOG_DEBUG) level=LOG_INFO;
    fmt+=3;
  }
 
  if (level<=printklevel) return 0;
  
  vksprintf(buf,(char*)fmt,ap);

  if (level != NO_LEVEL) 
    message("[%s] %s",levelname[level],buf);
  else
   message("%s",buf);
  
  return 0;
}

int printk(const char *fmt, ...)
{
  va_list ap;
  int res = 0;
  
  va_start(ap, fmt);
  res=vprintk(0,(char *)fmt, ap);
  va_end(ap);
  return res;
}

int printkboot(const char *fmt, ...)
{
  va_list ap;
  int res;
  
  va_start(ap, fmt);
  res=vprintk(1,(char *)fmt, ap);
  va_end(ap);
  return res;
}
