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


/***************************************

  CVS :        $Id: glue.h,v 1.1.1.1 2002/03/29 14:12:51 pj Exp $
 
  Revision:    $Revision: 1.1.1.1 $

  Last update: $Date: 2002/03/29 14:12:51 $

***************************************/

/*
 * Copyright (C) 1999,2000 Massimiliano Giorgi
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

#ifndef __FS_GLUE_H
#define __FS_GLUE_H

#include <ll/sys/types.h>

/*
 *
 * Block Device Glue
 *
 */

#include <fs/bdevconf.h>

/*
 * IDE Glue
 */

#ifdef IDE_BLOCK_DEVICE

struct ide_server_model {
  TIME dl;
  TIME wcet;
  TIME mit;
};

#endif

#endif
