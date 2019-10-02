/*
 * Project: HARTIK (HA-rd R-eal TI-me K-ernel)
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *               Gerardo Lamastra <gerardo@sssup.it>
 *
 * Authors     : Massimiliano Giorgi <massy@hartik.sssup.it>
 * (see authors.txt for full list of hartik's authors)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it 
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
 * CVS :        $Id: debug.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 Debugging flags that change the beaviour of most functions (causing functions to emit
 debugging informations).
 ***/

#ifndef __DEBUG_H
#define __DEBUG_H

#include <fs/util.h>

/* permit to start/stop debugging messages */
#define DEBUGSTARTSTOP 1
#undef DEBUGSTARTSTOP

/* the mutex are checked to discover bug (do not use! is dangerous)*/
#define FSMUTEXCHECK 1
#undef FSMUTEXCHECK

/**/

void debug_dump_buffer(char *buffer,int size);

/*
 * initialization
 */

/* main init function */
#define DEBUG_MAININIT KERN_DEBUG
#undef DEBUG_MAININIT

/* main init function */
#define DEBUG_MAINEND KERN_DEBUG
#undef DEBUG_MAINEND

/*
 * primitives
 */

/* debug open() */
#define DEBUG_OPEN KERN_DEBUG
#undef DEBUG_OPEN

/* debug opendir() */
#define DEBUG_OPENDIR KERN_DEBUG
#undef DEBUG_OPENDIR

/* debug readdir() */
#define DEBUG_READDIR KERN_DEBUG
#undef DEBUG_READDIR
/* if you want "dump" of directory entries */
#define DEBUG_READDIR_DUMP
#undef DEBUG_READDIR_DUMP

/* debug lseek() */
#define DEBUG_LSEEK KERN_DEBUG
#undef DEBUG_LSEEK

/* debug unlink() */
#define DEBUG_UNLINK KERN_DEBUG
#undef DEBUG_UNLINK

/* debug fstat() & stat() */
#define DEBUG_XSTAT KERN_DEBUG
#undef DEBUG_XSTAT

/* debug mount() & umount() */
#define DEBUG_MOUNT KERN_DEBUG
#undef DEBUG_MOUNT
/* extra debug for mount() & umount() (show childs purging) */
#define DEBUGPURGECHILD 1
#undef DEBUGPURGECHILD

#ifndef DEBUG_MOUNT
#undef DEBUGPURGECHILD
#endif

/*
 * directory scanning
 */

/* debug find (search for new directory on directory cache) */
#define DEBUG_FIND KERN_DEBUG
#undef DEBUG_FIND
/* extra debug for find (dump every entry found!) */
#define DEBUG_FIND_DUMP
#undef DEBUG_FIND_DUMP

/* debug lookup (searching for new directory entries on disk) */
/* normal debug */
#define DEBUG_LOOKUP KERN_DEBUG
#undef DEBUG_LOOKUP
/* extra debug (show all filenames found) */
#define DEBUG_LOOKUP_EXTRA KERN_DEBUG
#undef DEBUG_LOOKUP_EXTRA

/*
 * disk cache
 */

/* debug locking functions of dcache.c module */
#define DEBUG_DCACHE KERN_DEBUG
#undef DEBUG_DCACHE

/* debug dcache_purgedevice() (to sync hard-disk with cache)*/
#define DEBUG_DCACHE_PURGE KERN_DEBUG
#undef DEBUG_DCACHE_PURGE

/**/
/**/
/**/

#ifdef DEBUGSTARTSTOP
extern int debug_info;
extern void debug_info_show(int enable);
#else
#define debug_info 1
#define debug_info_show(x)
#endif

/**/

#ifdef DEBUG_OPEN
#define printk0(fmt,args...) \
        if (debug_info) printk(DEBUG_OPEN fmt,##args)
#else
#define printk0(fmt,args...)
#endif

#ifdef DEBUG_LOOKUP
#define printk1(fmt,args...) \
        if (debug_info) printk(DEBUG_LOOKUP fmt,##args)
#else
#define printk1(fmt,args...)
#endif

#ifdef DEBUG_LOOKUP_EXTRA
#define printk1e(fmt,args...) \
        if (debug_info) printk(DEBUG_LOOKUP_EXTRA fmt,##args)
#else
#define printk1e(fmt,args...)
#endif

#ifdef DEBUG_LSEEK
#define printk3(fmt,args...) \
        if (debug_info) printk(DEBUG_LSEEK fmt,##args)
#else
#define printk3(fmt,args...)
#endif

#ifdef DEBUG_OPENDIR
#define printk4(fmt,args...) \
        if (debug_info) printk(DEBUG_OPENDIR fmt,##args)
#else
#define printk4(fmt,args...)
#endif

#ifdef DEBUG_READDIR
#define printk5(fmt,args...) \
        if (debug_info) printk(DEBUG_READDIR fmt,##args)
#else
#define printk5(fmt,args...)
#endif

#ifdef DEBUG_XSTAT
#define printk6(fmt,args...) \
        if (debug_info) printk(DEBUG_XSTAT fmt,##args)
#else
#define printk6(fmt,args...)
#endif

#ifdef DEBUG_FIND
#define printk7(fmt,args...) \
        if (debug_info) printk(DEBUG_FIND fmt,##args)
#else
#define printk7(fmt,args...)
#endif

#ifdef DEBUG_DCACHE
#define printk8(fmt,args...) \
        if (debug_info) printk(DEBUG_DCACHE fmt,##args)
#else
#define printk8(fmt,args...)
#endif
	
#ifdef DEBUG_MOUNT
#define printk9(fmt,args...) \
        if (debug_info) printk(DEBUG_MOUNT fmt,##args)
#else
#define printk9(fmt,args...)
#endif

#ifdef DEBUG_DCACHE_PURGE     
#define printka(fmt,args...) \
        if (debug_info) printk(DEBUG_DCACHE_PURGE fmt,##args)
#else
#define printka(fmt,args...)
#endif

#ifdef DEBUG_MAININIT
#define printkb(fmt,args...) \
        if (debug_info) printk(DEBUG_MAININIT fmt,##args)
#else
#define printkb(fmt,args...)
#endif

#ifdef DEBUG_MAINEND
#define printkc(fmt,args...) \
        if (debug_info) printk(DEBUG_MAINEND fmt,##args)
#else
#define printkc(fmt,args...)
#endif

#ifdef DEBUG_UNLINK
#define printkd(fmt,args...) \
        if (debug_info) printk(DEBUG_UNLINK fmt,##args)
#else
#define printkd(fmt,args...)
#endif

#endif

