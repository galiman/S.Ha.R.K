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
 CVS :        $Id: fsconf.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1.1.1 $
 Last update: $Date: 2002/03/29 14:12:51 $
 ------------
**/

/*
 * Copyright (C) 2000 Massimilano Giorgio
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

/* one of these two must be defined!
 *
 * GLOB_DESC_TABLE the kernel files table is shared by all the task
 *                 (so a task is similar to a thread)
 * LOC_DESC_TABLE   every task has a file table
 *                 (so a task is similar to a process)
 */
//#define GLOB_DESC_TABLE
//#define LOC_DESC_TABLE

#define MULTITHREAD
//#undef MULTITHREAD

/*
 * file systems
 */

/* define this if you want MSDOS filesystem support*/

#define MSDOS_FILESYSTEM
//#undef MSDOS_FILESYSTEM