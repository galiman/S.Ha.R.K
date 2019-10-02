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

  CVS :        $Id: phdsk.c,v 1.1.1.1 2002/03/29 14:12:49 pj Exp $
 
  Revision:    $Revision: 1.1.1.1 $

  Last update: $Date: 2002/03/29 14:12:49 $

  This module register every physical disk into the system
  (actually only for informational purpose).

***************************************/

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

#include "glue.h"
#include <fs/magic.h>

#include "phdsk.h"

/*+ an entry for every hard disk into the system +*/
static struct phdskinfo phdsk[MAXPHDSK];

/*++++++++++++++++++++++++++++++++++++++

  Initialize the interal data structures. Must be called prior of every
  routine in this module.

  ++++++++++++++++++++++++++++++++++++++*/

void phdsk_init(void)
{
  int i;
  for (i=0;i<MAXPHDSK;i++) {

#ifdef _PARANOIA
    phdsk[i].magic=PHDSK_MAGIC;
#endif
    
    memcpy(phdsk[i].pd_name,"noname",7);
    
    phdsk[i].pd_device=0;
    phdsk[i].pd_size=0;
    phdsk[i].pd_sectsize=0;

#ifdef IDE_BLOCK_DEVICE    
    phdsk[i].pd_ide_check_geom=0;
#endif
    
    phdsk[i].pd_phgeom.cyls=0;
    phdsk[i].pd_phgeom.heads=0;
    phdsk[i].pd_phgeom.sectors=0;
    
    phdsk[i].pd_logeom.cyls=0;
    phdsk[i].pd_logeom.heads=0;
    phdsk[i].pd_logeom.sectors=0;
  }
}

/*++++++++++++++++++++++++++++++++++++++
  
  Every block device module shall call this function to inform
  the system that a physical hard-disk in available to the system.

  struct phdskinfo *phdsk_register
    return a pointer to a phdskinfo structure that hold hard disk information
    or NULL in case of error

  struct phdskinfo *disk
    information on the physical hard disk
  ++++++++++++++++++++++++++++++++++++++*/

struct phdskinfo *phdsk_register(struct phdskinfo *disk)
{
  int i;
  
  for (i=0;i<MAXPHDSK;i++)
    if (phdsk[i].pd_device==0) break;
  if (i==MAXPHDSK||disk->pd_device==0) return NULL;

  //cprintf("pdisk dev %i\n",disk->pd_device);
  memcpy(phdsk+i,disk,sizeof(struct phdskinfo));
  //cprintf("pdisk dev %i\n",(phdsk+i)->pd_device);

  return phdsk+i;
}
