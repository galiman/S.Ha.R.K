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
 * CVS :        $Id: panic.c,v 1.2 2005/01/08 14:46:42 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2005/01/08 14:46:42 $
 */

#include <ll/i386/cons.h>
#include <ll/stdarg.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <stdlib.h>
#include <kernel/func.h>

// defined in kern.c
extern int runlevel;

static char buf[1024]; /* DANGER !!!!! */

static void sys_panic_stub(void *arg)
{
  cprintf("KERNEL PANIC (sys_panic_stub): %s\n",buf);
}

void sys_panic(const char * fmt, ...)
{
  va_list ap;
  
  va_start(ap, fmt);
  vsprintf(buf,(char*)fmt,ap);
  va_end(ap);
  
  sys_atrunlevel(sys_panic_stub, NULL, RUNLEVEL_AFTER_EXIT);
  printk("KERNEL PANIC (sys_panic): %s\n",buf);

  if (!ll_ActiveInt()) {
    if (runlevel==RUNLEVEL_RUNNING)
      exit(333);
    
    if (runlevel==RUNLEVEL_SHUTDOWN)
      sys_abort_shutdown(333);
  }
  else
    kern_raise(XPANIC_INSIDE_IRQ, exec_shadow);
}

