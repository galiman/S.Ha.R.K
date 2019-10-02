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
 * CVS :        $Id: types.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:51 $
 *
 * Heavy modified from glibc 2.x.x
 *
 */

/* Copyright (C) 1991, 92, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

#ifndef	_BITS_TYPES_H
#define	_BITS_TYPES_H	1

#include <features.h>
#include <ll/sys/types.h>
#include <sys/htypes.h>

#ifdef __GNUC__
#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
__extension__ typedef unsigned long long int u_int64_t;
#else
typedef unsigned int u_int64_t __attribute__ ((__mode__(__DI__)));
#endif
#endif

#ifdef __GNUC__
#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
__extension__ typedef signed long long int int64_t;
#else
typedef int int64_t __attribute__ ((__mode__(__DI__)));
#endif
#endif

/* Convenience types.  */
typedef u_char  __u_char;
typedef u_short __u_short;
typedef u_int   __u_int;
typedef u_long  __u_long;

#ifdef __GNUC__
__extension__ typedef u_int64_t __u_quad_t;
__extension__ typedef int64_t __quad_t ;
#else
typedef struct
{
  long __val[2];
} __quad_t;
typedef struct
{
  __u_long __val[2];
} __u_quad_t;
#endif

typedef __quad_t *__qaddr_t;

typedef int8_t    __int8_t;
typedef u_int8_t  __uint8_t;
typedef int16_t   __int16_t;
typedef u_int16_t __uint16_t;
typedef int32_t   __int32_t;
typedef u_int32_t __uint32_t;
#ifdef __GNUC__
typedef int64_t   __int64_t;
typedef u_int64_t __uint64_t;
#endif

__DJ_dev_t
#undef __DJ_dev_t
#define __DJ_dev_t
__DJ_uid_t
#undef __DJ_uid_t
#define __DJ_uid_t
__DJ_gid_t
#undef __DJ_gid_t
#define __DJ_gid_t
__DJ_ino_t
#undef __DJ_ino_t
#define __DJ_ino_t
__DJ_mode_t
#undef __DJ_mode_t
#define __DJ_mode_t
__DJ_nlink_t
#undef __DJ_nlink_t
#define __DJ_nlink_t
__DJ_off_t
#undef __DJ_off_t
#define __DJ_off_t
__DJ_pid_t
#undef __DJ_pid_t
#define __DJ_pid_t
__DJ_clock_t
#undef __DJ_clock_t
#define __DJ_clock_t
__DJ_time_t
#undef __DJ_time_t
#define __DJ_time_t

typedef dev_t __dev_t;		/* Type of device numbers.  */
typedef uid_t __uid_t;	        /* Type of user identifications.  */
typedef gid_t __gid_t;	        /* Type of group identifications.  */
typedef ino_t __ino_t;	        /* Type of file serial numbers.  */
typedef __quad_t __ino64_t;	/* Type of file serial numbers (LFS).  */
typedef mode_t __mode_t;	/* Type of file attribute bitmasks.  */
typedef nlink_t __nlink_t;      /* Type of file link counts.  */
typedef off_t __off_t;	        /* Type of file sizes and offsets.  */
typedef __quad_t __loff_t;	/* Type of file sizes and offsets.  */
typedef __loff_t __off64_t;	/* Type of file sizes and offsets (LFS).  */
typedef pid_t __pid_t;		/* Type of process identifications.  */
typedef size_t __size_t;
typedef ssize_t __ssize_t;	/* Type of a byte count, or error.  */
typedef __u_quad_t __fsid_t;	/* Type of file system IDs.  */
typedef clock_t __clock_t;	/* Type of CPU usage counts.  */
typedef long int __rlim_t;	/* Type for resource measurement.  */
typedef __quad_t __rlim64_t;	/* Type for resource measurement (LFS).  */
typedef unsigned int __id_t;	/* General type for IDs.  */

/* Everythin' else.  */
typedef long int __daddr_t;	/* The type of a disk address.  */
typedef char *__caddr_t;        /* Type of ... */
typedef time_t __time_t;        /* Type of time */
typedef long int __swblk_t;	/* Type of a swap block maybe?  */
typedef long int __key_t;	/* Type of an IPC key */

/* One element in the file descriptor mask array.  */
typedef unsigned long int __fd_mask;

/* Number of descriptors that can fit in an `fd_set'.  */
#define	__FD_SETSIZE	256

/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define	__NFDBITS	(sizeof (unsigned long int) * 8)
#define	__FDELT(d)	((d) / __NFDBITS)
#define	__FDMASK(d)	((__fd_mask) 1 << ((d) % __NFDBITS))

/* fd_set for select and pselect.  */
typedef struct
  {
    /* XPG4.2 requires this member name.  Otherwise avoid the name
       from the user namespace.  */
#ifdef __USE_XOPEN
    __fd_mask fds_bits[(__FD_SETSIZE + (__NFDBITS - 1)) / __NFDBITS];
# define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[(__FD_SETSIZE + (__NFDBITS - 1)) / __NFDBITS];
# define __FDS_BITS(set) ((set)->__fds_bits)
#endif
  } __fd_set;

/* XXX Used in `struct shmid_ds'.  */
typedef unsigned short int __ipc_pid_t;

/* Types from the Large File Support interface.  */

/* Type to count number os disk blocks.  */
typedef long int __blkcnt_t;
typedef __quad_t __blkcnt64_t;

/* Type to count file system blocks.  */
typedef unsigned int __fsblkcnt_t;
typedef __u_quad_t __fsblkcnt64_t;

/* Type to count file system inodes.  */
typedef unsigned long int __fsfilcnt_t;
typedef __u_quad_t __fsfilcnt64_t;

/* Used in XTI.  */
typedef int __t_scalar_t;
typedef unsigned int __t_uscalar_t;

/* Duplicates info from stdint.h but this is used in unistd.h.  */
typedef long int __intptr_t;

#endif /* bits/types.h */
