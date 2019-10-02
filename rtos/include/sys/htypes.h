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
 CVS :        $Id: htypes.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1.1.1 $
 Last update: $Date: 2002/03/29 14:12:51 $
 ------------

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
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */

#ifndef __SYS_HTYPES_H_
#define __SYS_HTYPES_H_

#define __DJ_clock_t	typedef int clock_t;
#define __DJ_clockid_t	typedef int clockid_t;
#define __DJ_dev_t	typedef unsigned int dev_t;
#define __DJ_gid_t	typedef unsigned int gid_t;
#define __DJ_ino_t	typedef unsigned int ino_t;
#define __DJ_mode_t	typedef int mode_t;
#define __DJ_nlink_t	typedef short int nlink_t;
#define __DJ_off_t	typedef long int off_t;
#define __DJ_pid_t	typedef int pid_t;
#define __DJ_time_t	typedef unsigned int time_t;
#define __DJ_timer_t	typedef int timer_t;
#define __DJ_uid_t	typedef unsigned int uid_t;

#if defined(__cplusplus) && ( (__GNUC_MINOR__ >= 8 && __GNUC__ == 2 ) || __GNUC__ >= 3 )
/* wchar_t is now a keyword in C++ */
#define __DJ_wchar_t
#else
/* but remains a typedef in C */
#define __DJ_wchar_t    typedef int wchar_t;
#endif

#define __DJ_wint_t     typedef int wint_t;

#endif
