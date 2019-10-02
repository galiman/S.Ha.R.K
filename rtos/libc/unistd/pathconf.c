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
 * CVS :        $Id: pathconf.c,v 1.1.1.1 2002/03/29 14:12:53 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:53 $
 */

#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

/* it is not conforming to POSIX (the filedescis not check/used) */

long pathconf(const char *path, int name)
{
  switch (name) {
    case _PC_ASYNC_IO	      : return _POSIX_ASYNC_IO;
    case _PC_CHOWN_RESTRICTED : return _POSIX_CHOWN_RESTRICTED;
    case _PC_LINK_MAX	      : return _POSIX_LINK_MAX;
    case _PC_MAX_CANON	      : return _POSIX_MAX_CANON;
    case _PC_MAX_INPUT	      : return _POSIX_MAX_INPUT;
    case _PC_NAME_MAX	      : return _POSIX_NAME_MAX;
    case _PC_NO_TRUNC	      : return _POSIX_NO_TRUNC;
    case _PC_PATH_MAX	      : return _POSIX_PATH_MAX;
    case _PC_PIPE_BUF	      : return _POSIX_PIPE_BUF;
    case _PC_PRIO_IO	      : return _POSIX_PRIO_IO;
    case _PC_SYNC_IO	      : return _POSIX_SYNC_IO;
    case _PC_VDISABLE	      : return _POSIX_VDISABLE;        
  }
  errno=EINVAL;
  return -1;
}
