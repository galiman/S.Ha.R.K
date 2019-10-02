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
 * CVS :        $Id: limits.h,v 1.2 2003/03/13 13:37:58 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:37:58 $
 */

#ifndef __FS_LIMITS_H
#define __FS_LIMITS_H

#include <bits/limits.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* max filename lenght in chars */
#define MAXFILENAMELEN FILENAME_MAX

/* max pathname lenght in chars */
#define MAXPATHLEN MAXPATHNAMELEN

/* every process can open MAXOPENFILES */
/* (is equal to MAXSYSFILE because the file descriptors table */
/* is global to all task and not local by default) */

/*#define MAXOPENFILES   128*/
/* defined into bits/limits.h */

extern __inline__ int getdtablesize(void)
{ return MAXOPENFILES; }

/* PS: note that an hartik task can be considered threads or process
 * see fsconfig.h for more info
 */

/* max open file into the system */
/* globally the system can open MAXSYSFILE files */
#define MAXSYSFILE     MAXOPENFILES

/*
 * these are used by GLIBC
 */

//#define NGROUPS_MAX 0		/* No supplementary groups.  */
//#define ARG_MAX 5120
//#define CHILD_MAX 25
#ifndef OPEN_MAX
#define OPEN_MAX MAXOPENFILES
#endif
//#define LINK_MAX 1000
//#define MAX_CANON 256

/* For SVR3, this is 14.  For SVR4, it is 255, at least on ufs
   file systems, even though the System V limits.h incorrectly
   defines it as 14.  Giving it a value which is too large
   is harmless (it is a maximum).  */
#ifndef NAME_MAX
#define NAME_MAX MAXFILENAMELEN
#endif
#ifndef PATH_MAX
#define PATH_MAX MAXPATHNAMELEN
#endif

__END_DECLS
#endif





