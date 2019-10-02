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

/**
 ------------
 CVS :        $Id: unistd.h,v 1.3 2003/05/22 09:35:08 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.3 $
 Last update: $Date: 2003/05/22 09:35:08 $
 ------------

 unistd.h, inspired on BSD one...
 and heavy modified by Massy (inspired on GLibc one...)
 
**/

/*
 * Copyright (C) 2000 Paolo Gai
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
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)unistd.h	8.2 (Berkeley) 1/7/94
 */

#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <features.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

void _exit(int status);

/* compile-time symbolic constants */

/* NYI = Not Yet Implemented */

// NYI #define _POSIX_ASYNCHRONOUS_IO
// NYI #define _POSIX_ASYNC_IO
// NRQ for PSE52 #define	_POSIX_CHOWN_RESTRICTED	1
// NYI #define _POSIX_FSYNC
// NRQ for PSE52 #define	_POSIX_JOB_CONTROL
// NYI #define _POSIX_MAPPED_FILES
// NYI #define _POSIX_MEMLOCK
// NYI #define _POSIX_MEMLOCK_RANGE
// NRQ for PSE52 #define _POSIX_MEMORY_PROTECTION
#define _POSIX_MESSAGE_PASSING
// NYI #define	_POSIX_NO_TRUNC		1
// NRQ for PSE52 #define _POSIX_PRIORITIZED_IO
// NRQ for PSE52 #define _POSIX_PRIORITY_SCHEDULING
// NYI #define _POSIX_PRIO_IO
#define _POSIX_REALTIME_SIGNALS
// NRQ for PSE52 #define	_POSIX_SAVED_IDS
#define _POSIX_SEMAPHORES
// NYI #define _POSIX_SHARED_MEMORY_OBJECTS
// NYI #define _POSIX_SYNCHRONIZED_IO
// NYI #define _POSIX_SYNC_IO
#define _POSIX_THREADS
#define _POSIX_THREAD_ATTR_STACKADDR
#define _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_PRIO_INHERIT
#define _POSIX_THREAD_PRIO_PROTECT
#define _POSIX_THREAD_PRIORITY_SCHEDULING
// NRQ for PSE52 #define _POSIX_THREAD_PROCESS_SHARED
// NYI #define _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_TIMERS
// NRQ for PSE52 #define	_POSIX_VDISABLE		0xff
#define	_POSIX_VERSION		199506L







/* access function */
#define	F_OK		0	/* test for existence of file */
#define	X_OK		0x01	/* test for execute or search permission */
#define	W_OK		0x02	/* test for write permission */
#define	R_OK		0x04	/* test for read permission */

/* whence values for lseek(2) */
#define	SEEK_SET	0	/* set file offset to offset */
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#define	SEEK_END	2	/* set file offset to EOF plus offset */

#ifndef _POSIX_SOURCE
/* whence values for lseek(2); renamed by POSIX 1003.1 */
#define	L_SET		SEEK_SET
#define	L_INCR		SEEK_CUR
#define	L_XTND		SEEK_END
#endif

/* configurable pathname variables */
#define	_PC_ASYNC_IO		 1
#define	_PC_CHOWN_RESTRICTED	 2
#define	_PC_LINK_MAX		 3
#define	_PC_MAX_CANON		 4
#define	_PC_MAX_INPUT		 5
#define	_PC_NAME_MAX		 6
#define	_PC_NO_TRUNC		 7
#define	_PC_PATH_MAX		 8
#define	_PC_PIPE_BUF		 9
#define	_PC_PRIO_IO		10
#define	_PC_SYNC_IO		11
#define	_PC_VDISABLE		12

/* configurable system variables */
#define _SC_AIO_LISTIO_MAX                1
#define _SC_AIO_MAX                       2
#define _SC_AIO_PRIO_DELTA_MAX            3
#define _SC_ARG_MAX                       4
#define _SC_CHILD_MAX                     5
#define _SC_CLK_TICK                      6
#define _SC_DELAYTIMER_MAX                7
#define _SC_GETGR_R_SIZE_MAX              8
#define _SC_GETPW_R_SIZE_MAX              9
#define _SC_LOGIN_NAME_MAX               10
#define _SC_MQ_OPEN_MAX                  11
#define _SC_MQ_PRIO_MAX                  12
#define _SC_NGROUPS_MAX                  13
#define _SC_OPEN_MAX                     14
#define _SC_PAGESIZE                     15
#define _SC_RTSIG_MAX                    16
#define _SC_SEM_NSEMS_MAX                17
#define _SC_SEM_VALUE_MAX                18
#define _SC_SIGQUEUE_MAX                 19
#define _SC_STREAM_MAX                   20
#define _SC_THREAD_DESTRUCTOR_ITERATIONS 21
#define _SC_THREAD_KEYS_MAX              22
#define _SC_THREAD_STACK_MIN             23
#define _SC_THREAD_THREADS_MAX           24
#define _SC_TIMER_MAX                    25
#define _SC_TTY_NAME_MAX                 26
#define _SC_TZNAME_MAX                   27

#define _SC_ASYNCHRONOUS_IO              28
#define _SC_FSYNC                        29
#define _SC_JOB_CONTROL                  30
#define _SC_MAPPED_FILES                 31
#define _SC_MEMLOCK                      32
#define _SC_MEMLOCK_RANGE                33
#define _SC_MEMORY_PROTECTION            34
#define _SC_MESSAGE_PASSING              35
#define _SC_PRIORITIZED_IO               36
#define _SC_PRIORITY_SCHEDULING          37
#define _SC_REALTIME_SIGNALS             38
#define _SC_SAVED_IDS                    39
#define _SC_SEMAPHORES                   40
#define _SC_SHARED_MEMORY_OBJECTS        41
#define _SC_SYNCHRONIZED_IO              42
#define _SC_TIMERS                       43
#define _SC_THREADS                      44
#define _SC_THREAD_ATTR_STACKADDR        45
#define _SC_THREAD_ATTR_STACKSIZE        46
#define _SC_THREAD_PRIORITY_SCHEDULING   47
#define _SC_THREAD_PRIO_INHERIT          48
#define _SC_THREAD_PRIO_PROTECT          49
#define _SC_THREAD_PROCESS_SHARED        50
#define _SC_THREAD_SAFE_FUNCTIONS        51
#define _SC_VERSION                      52

/* configurable system strings */
#define	_CS_PATH		 1

/*+ send an alarm signal +*/
unsigned int alarm(unsigned int seconds);

/*+ wait a signal delivery +*/
int pause(void);

/*+ sleep for n seconds or until a signal is delivered to the thread +*/
unsigned int sleep(unsigned int seconds);

/*+ sleep for n useconds */
unsigned int usleep(unsigned int usec);

#define udelay(x) usleep(x)
/*
 *
 *
 *
 *
 *
 *
 */

/* X/Open version number to which the library conforms.  It is selectable.  */
//#ifdef __USE_UNIX98
//# define _XOPEN_VERSION	500
//#else
//# define _XOPEN_VERSION	4
//#endif

/* Commands and utilities from XPG4 are available.  */
//#define _XOPEN_XCU_VERSION	4

/* We are compatible with the old published standards as well.  */
//#define _XOPEN_XPG2	1
//#define _XOPEN_XPG3	1
//#define _XOPEN_XPG4	1

/* The X/Open Unix extensions are available.  */
//#define _XOPEN_UNIX	1

/* Encryption is present.  */
//#define	_XOPEN_CRYPT	1

/* The enhanced internationalization capabilities according to XPG4.2
   are present.  */
//#define	_XOPEN_ENH_I18N	1

/* The legacy interfaces are also available.  */
//#define _XOPEN_LEGACY	1

/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */
#define	STDOUT_FILENO	1	/* Standard output.  */
#define	STDERR_FILENO	2	/* Standard error output.  */

#include <ll/sys/types.h>
#include <sys/types.h>

__DJ_dev_t
#undef __DJ_dev_t
#define __DJ_dev_t

__DJ_ino_t
#undef __DJ_ino_t
#define __DJ_ino_t

__DJ_mode_t
#undef __DJ_mode_t
#define __DJ_mode_t

__DJ_nlink_t
#undef __DJ_nlink_t
#define __DJ_nlink_t

__DJ_gid_t
#undef __DJ_gid_t
#define __DJ_gid_t

__DJ_off_t
#undef __DJ_off_t
#define __DJ_off_t

__DJ_pid_t
#undef __DJ_pid_t
#define __DJ_pid_t

__DJ_uid_t
#undef __DJ_uid_t
#define __DJ_uid_t

/* Test for access to NAME using the real UID and real GID.  */
extern int access __P ((__const char *__name, int __type));
#if defined __USE_BSD && !defined L_SET

/* Old BSD names for the same constants; just for compatibility.  */
# define L_SET		SEEK_SET
# define L_INCR		SEEK_CUR
# define L_XTND		SEEK_END
#endif

/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the new file position.  */
//extern __off_t __lseek __P ((int __fd, __off_t __offset, int __whence));
#ifndef __USE_FILE_OFFSET64
extern __off_t lseek __P ((int __fd, __off_t __offset, int __whence));
#else
# ifdef __REDIRECT
extern __off64_t __REDIRECT (lseek,
			     __P ((int __fd, __off64_t __offset,
				   int __whence)),
			     lseek64);
# else
#  define lseek lseek64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern __off64_t lseek64 __P ((int __fd, __off64_t __offset, int __whence));
#endif

/* Close the file descriptor FD.  */
//extern int __close __P ((int __fd));
extern int close __P ((int __fd));

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.  */
//extern ssize_t __read __P ((int __fd, __ptr_t __buf, size_t __nbytes));
extern ssize_t read __P ((int __fd, __ptr_t __buf, size_t __nbytes));

/* Write N bytes of BUF to FD.  Return the number written, or -1.  */
//extern ssize_t __write __P ((int __fd, __const __ptr_t __buf, size_t __n));
extern ssize_t write __P ((int __fd, __const __ptr_t __buf, size_t __n));

/* Change the process's working directory to PATH.  */
extern int chdir __P ((__const char *__path));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Change the process's working directory to the one FD is open on.  */
//extern int fchdir __P ((int __fd));
#endif

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE == 0, in which case it is as
   big as necessary.  */
extern char *getcwd __P ((char *__buf, size_t __size));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Put the absolute pathname of the current working directory in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
extern char *getwd __P ((char *__buf));
#endif


/* Duplicate FD, returning a new file descriptor on the same file.  */
extern int dup __P ((int __fd));

/* Duplicate FD to FD2, closing FD2 and making it open on the same file.  */
extern int dup2 __P ((int __fd, int __fd2));

/* Return 1 if FD is a valid descriptor associated
   with a terminal, zero if not.  */
extern int isatty __P ((int __fd));

/* Make all changes done to FD actually appear on disk.  */
extern int fsync __P ((int __fd));

/* Truncate FILE to LENGTH bytes.  */
#ifndef __USE_FILE_OFFSET64
extern int truncate __P ((__const char *__file, __off_t __length));
#else
# ifdef __REDIRECT
extern int __REDIRECT (truncate,
		       __P ((__const char *__file, __off64_t __length)),
		       truncate64);
# else
#  define truncate truncate64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int truncate64 __P ((__const char *__file, __off64_t __length));
#endif

/* Truncate the file FD is open on to LENGTH bytes.  */
extern int __ftruncate __P ((int __fd, __off_t __length));
#ifndef __USE_FILE_OFFSET64
extern int ftruncate __P ((int __fd, __off_t __length));
#else
# ifdef __REDIRECT
extern int __REDIRECT (ftruncate, __P ((int __fd, __off64_t __length)),
		       ftruncate64);
# else
#  define ftruncate ftruncate64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int ftruncate64 __P ((int __fd, __off64_t __length));
#endif

/* Return the maximum number of file descriptors
   the current process could possibly have.  */
extern int getdtablesize __P ((void));

/* Make a link to FROM named TO.  */
extern int link __P ((__const char *__from, __const char *__to));

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
/* Make a symbolic link to FROM named TO.  */
extern int symlink __P ((__const char *__from, __const char *__to));

/* Read the contents of the symbolic link PATH into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
extern int readlink __P ((__const char *__path, char *__buf, size_t __len));
#endif /* Use BSD.  */

/* Remove the link NAME.  */
extern int unlink __P ((__const char *__name));

/* Remove the directory PATH.  */
extern int rmdir __P ((__const char *__path));

/* ??? */
char	*mktemp __P((char *));

extern __inline__ pid_t getpid(void)
{
  return 0;
}

extern long pathconf(const char *path, int name);
extern long fpathconf(int filedes, int name);

__END_DECLS
#endif /* !__UNISTD_H_ */
