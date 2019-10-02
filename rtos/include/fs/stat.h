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
 * CVS :        $Id: stat.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:51 $
 */

#ifndef __FS_STAT_H
#define __FS_STAT_H

#include <fs/types.h>

#define _SYS_STAT_H
#include <bits/stat.h>

#define __S_BLKSIZE   512	/* Block size for `st_blocks'.  */

/*
 * test file type macros
 */

#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))

#define	__S_ISDIR(mode)	 __S_ISTYPE((mode), __S_IFDIR)
#define	__S_ISCHR(mode)	 __S_ISTYPE((mode), __S_IFCHR)
#define	__S_ISBLK(mode)	 __S_ISTYPE((mode), __S_IFBLK)
#define	__S_ISREG(mode)	 __S_ISTYPE((mode), __S_IFREG)
/* the following can be undefined */
#define __S_ISFIFO(mode) __S_ISTYPE((mode), __S_IFIFO)
#define __S_ISLNK(mode)	 __S_ISTYPE((mode), __S_IFLNK)
#define __S_ISSOCK(mode) __S_ISTYPE((mode), __S_IFSOCK)

/*
 * files permission
 */

#define	__S_IRUSR	__S_IREAD	        /* Read by owner.  */
#define	__S_IWUSR	__S_IWRITE	        /* Write by owner.  */
#define	__S_IXUSR	__S_IEXEC	        /* Execute by owner.  */
#define	__S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)

#define	__S_IRGRP	(__S_IRUSR >> 3)	/* Read by group.  */
#define	__S_IWGRP	(__S_IWUSR >> 3)	/* Write by group.  */
#define	__S_IXGRP	(__S_IXUSR >> 3)	/* Execute by group.  */
#define	__S_IRWXG	(__S_IRWXU >> 3)

#define	__S_IROTH	(__S_IRGRP >> 3)	/* Read by others.  */
#define	__S_IWOTH	(__S_IWGRP >> 3)	/* Write by others.  */
#define	__S_IXOTH	(__S_IXGRP >> 3)	/* Execute by others.  */
#define	__S_IRWXO	(__S_IRWXG >> 3)

/*  0777 */
#define __ACCESSPERMS (__S_IRWXU|__S_IRWXG|__S_IRWXO)
/* 07777 */
#define __ALLPERMS    (__S_ISUID|__S_ISGID|__S_ISVTX|\
		       __S_IRWXU|__S_IRWXG|__S_IRWXO)
/*  0666 */
#define __DEFFILEMODE (__S_IRUSR|__S_IWUSR|__S_IRGRP|\
		       __S_IWGRP|__S_IROTH|__S_IWOTH)

#define __DEFUMASK    (~(__DEFFILEMODE))

#endif
