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
 * CVS :        $Id: phdsk.h,v 1.2 2003/03/13 13:43:13 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:43:13 $
 */

#ifndef __PHDSK_H__
#define __PHDSK_H__

#include "glue.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define MAXPHDSK 4

#define MAXPHDSKNAME 16

/*-----------*/

struct dskgeometry {
  __uint16_t	cyls;	  /* logical cylinders */
  __uint16_t	heads;	  /* logical heads */
  __uint16_t	sectors;  /* logical sectors per track */
};
  		     
struct phdskinfo {

#ifdef _PARANOIA
  __uint32_t magic;
#endif
  
  char      pd_name[MAXPHDSKNAME];
  __dev_t   pd_device;

#ifdef IDE_BLOCK_DEVICE
  __uint32_t pd_ide_check_geom:1;
#endif
  
  __blkcnt_t  pd_size;
  int       pd_sectsize;
  
  struct dskgeometry pd_phgeom;
  struct dskgeometry pd_logeom;
};

void phdsk_init(void);
struct phdskinfo *phdsk_register(struct phdskinfo *disk);

__END_DECLS
#endif
