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
 * Copyright (C) 2000 Massimiliano Giorgi
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
 * CVS :        $Id: log.h,v 1.2 2003/03/13 13:36:28 pj Exp $
 *
 * File:        log.h
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:36:28 $
 */

#ifndef __KERNEL_LOG_H
#define __KERNEL_LOG_H

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+
  The following values are used for the first parameter
  of the printk() and printkboot() functions.

    system is unusable			+*/
#define	KERN_EMERG	"<0>"	
/*+ action must be taken immediately	+*/
#define	KERN_ALERT	"<1>"	
/*+ critical conditions			+*/
#define	KERN_CRIT	"<2>"	
/*+ error conditions			+*/
#define	KERN_ERR	"<3>"	
/*+ warning conditions			+*/
#define	KERN_WARNING	"<4>"
/*+ normal but significant condition	+*/	
#define	KERN_NOTICE	"<5>"
/*+ informational			+*/
#define	KERN_INFO	"<6>"	
/*+ debug-level messages		+*/
#define	KERN_DEBUG	"<7>"	

#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_ERR     3
#define LOG_WARNING 4
#define LOG_NOTICE  5
#define LOG_INFO    6
#define LOG_DEBUG   7

extern void setprintklevel(int level);

extern int printk(const char * fmt, ...)
     __attribute__ ((format (printf, 1, 2)));
extern int printkboot(const char * fmt, ...)
     __attribute__ ((format (printf, 1, 2)));

__END_DECLS
#endif     
     
     
