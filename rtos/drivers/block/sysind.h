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
 * CVS :        $Id: sysind.h,v 1.1.1.1 2002/03/29 14:12:49 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:49 $
 */

#ifndef __SYSIND_H__
#define __SYSIND_H__

/* These three have identical behaviour; use the second one if DOS fdisk gets
   confused about extended/logical partitions starting past cylinder 1023. */
#define DOS_EXTENDED_PARTITION 5
#define LINUX_EXTENDED_PARTITION 0x85
#define WIN98_EXTENDED_PARTITION 0x0f

#define FS_DEFAULT      0xff    /* usa il default */

#define FS_DOS12 	0x01	/* partizione dos sotto i 16M */
#define FS_DOS16	0X04	/* partizione dos 3.0+ ( < 65535 settori ) */
#define FS_DOSBIG	0X06	/* partizione dos 4.0+ ( > 32M ) */
#define FS_FAT32        0x0b	/* partizione WIN95 con fat32 */
#define FS_LNXMNX	0x81	/* partizione Linux/Minix */
#define FS_LNXSWP	0x82	/* partizione Linux Swap */
#define FS_LNXNTV	0x83	/* partizione Linux native */
#define FS_FREBSD	0xa5	/* partizione FreeBSD */

#endif
