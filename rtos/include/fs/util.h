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

/***************************************



***************************************/

/*
 * CVS :        $Id: util.h,v 1.2 2003/03/13 13:37:58 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:37:58 $
 */

/*
 * Copyright (C) 1999,2000 Massimiliano Giorgi
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

#ifndef _FS_UTIL_H
#define _FS_UTIL_H

#include <fs/types.h>
#include <fs/stdarg.h>

#include <ll/i386/stdio.h>
//#include <ll/string.h> strcasecmp moved!!!
#include <string.h>
#include <ll/stdlib.h>
	
#include <kernel/log.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define panic(x) sys_panic(x)

/* local time, number of seconds since the Epoch (0:0:0 1 Gen 1970 GMT) */
/* must be in sync with <drivers/rtc.h> */
extern __time_t sys_getdate(void);
#define gettimek() sys_getdate()

/* all in usec */
//extern void __delayk(int usec);
//extern long __gettimer(void);

__END_DECLS	
#endif
