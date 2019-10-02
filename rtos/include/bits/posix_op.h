/* Define POSIX options for Linux.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

/* MODIFIED (from GNU libc 2.1.2) */

#ifndef	_POSIX_OPT_H
#define	_POSIX_OPT_H	1

/* Job control is supported.  */
#define	_POSIX_JOB_CONTROL	0

/* Processes have a saved set-user-ID and a saved set-group-ID.  */
#define	_POSIX_SAVED_IDS	0

/* Priority scheduling is supported.  */
#define	_POSIX_PRIORITY_SCHEDULING	0

/* Synchronizing file data is supported.  */
#define	_POSIX_SYNCHRONIZED_IO	0

/* The fsync function is present.  */
#define	_POSIX_FSYNC	0

/* Mapping of files to memory is supported.  */
#define	_POSIX_MAPPED_FILES	0

/* Locking of all memory is supported.  */
#define	_POSIX_MEMLOCK	0

/* Locking of ranges of memory is supported.  */
#define	_POSIX_MEMLOCK_RANGE	0

/* Setting of memory protections is supported.  */
#define	_POSIX_MEMORY_PROTECTION	0

/* Implementation supports `poll' function.  */
#define	_POSIX_POLL	0

/* Implementation supports `select' and `pselect' functions.  */
#define	_POSIX_SELECT	0

/* Only root can change owner of file.  */
#define	_POSIX_CHOWN_RESTRICTED	0

/* `c_cc' member of 'struct termios' structure can be disabled by
   using the value _POSIX_VDISABLE.  */
#define	_POSIX_VDISABLE	'\0'

/* Filenames are not silently truncated.  */
#define	_POSIX_NO_TRUNC	1

/* X/Open realtime support is available.  */
#define _XOPEN_REALTIME	0

/* X/Open realtime thread support is available.  */
#define _XOPEN_REALTIME_THREADS	0

/* XPG4.2 shared memory is supported.  */
#define	_XOPEN_SHM	0

/* Tell we have POSIX threads.  */
#define _POSIX_THREADS	0

/* We have the reentrant functions described in POSIX.  */
#define _POSIX_REENTRANT_FUNCTIONS      0
#define _POSIX_THREAD_SAFE_FUNCTIONS	0

/* We provide priority scheduling for threads.  */
#define	_POSIX_THREAD_PRIORITY_SCHEDULING	0

/* We support user-defined stack sizes.  */
#define _POSIX_THREAD_ATTR_STACKSIZE	0

/* We support user-defined stacks.  */
#define _POSIX_THREAD_ATTR_STACKADDR	0

/* We support POSIX.1b semaphores, but only the non-shared form for now.  */
/*#define _POSIX_SEMAPHORES	1	XXX We are not quite there now.  */

/* Real-time signals are supported.  */
#define _POSIX_REALTIME_SIGNALS	0

/* We support asynchronous I/O.  */
#define _POSIX_ASYNCHRONOUS_IO	0
/* Alternative name for Unix98.  */
#define _LFS_ASYNCHRONOUS_IO	0

/* The LFS support in asynchronous I/O is also available.  */
#define _LFS64_ASYNCHRONOUS_IO	0

/* The rest of the LFS is also available.  */
#define _LFS_LARGEFILE		0
#define _LFS64_LARGEFILE	0
#define _LFS64_STDIO		0

#endif /* posix_opt.h */
