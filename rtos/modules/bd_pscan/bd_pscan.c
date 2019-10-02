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
 * CVS :        $Id: bd_pscan.c,v 1.1 2005/02/25 10:55:38 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1 $
 * Last update: $Date: 2005/02/25 10:55:38 $
 */

#include <bd_pscan/bd_pscan/bd_pscan.h>

#include <ll/ll.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/var.h>
#include <kernel/func.h>
#include <kernel/assert.h>

#define LOWESTPRIORITY 255
#define HIGHERPRIORITY   0

static int mylevel=-1;

typedef struct TAGbd_pscan_resource_des
{
  resource_des rd;
  int priority[MAX_PROC];
} bd_pscan_resource_des;

static int res_register(RLEVEL l, PID p, RES_MODEL *r)
{
  bd_pscan_resource_des *m=(bd_pscan_resource_des*)(resource_table[l]);
  BDPSCAN_RES_MODEL *rm;

  if (r->rclass!=BDEDF_RCLASS)
    return -1;
  if (r->level && r->level !=l)
    return -1;
  
  rm=(BDPSCAN_RES_MODEL*)r;
  assertk(mylevel==l);
  m->priority[p]=rm->priority;

  return 0;
}

static void res_detach(RLEVEL l, PID p)
{
  bd_pscan_resource_des *m=(bd_pscan_resource_des*)(resource_table[l]);
  assertk(mylevel==l);
  m->priority[p]=LOWESTPRIORITY;
}

RLEVEL BD_PSCAN_register_module(void)
{
  RLEVEL l;
  bd_pscan_resource_des *m;
  int i;
  
  /* request an entry in the level_table */
  l=resource_alloc_descriptor();

  /* alloc the space needed for the EDF_level_des */
  m=(bd_pscan_resource_des*)kern_alloc(sizeof(bd_pscan_resource_des));

  /* update the level_table with the new entry */
  resource_table[l]=(resource_des*)m;

  /* fill the resource_des descriptor */
  m->rd.rtype=DEFAULT_RTYPE;
  m->rd.res_register=res_register;
  m->rd.res_detach=res_detach;

  for (i=0;i<MAX_PROC;i++) m->priority[i]=LOWESTPRIORITY;
  
  assertk(mylevel==-1);
  mylevel=l;

  return l;
}

int bd_pscan_getpriority(void)
{
  bd_pscan_resource_des *m;
  if (mylevel==-1) return LOWESTPRIORITY;
  m=(bd_pscan_resource_des*)(resource_table[mylevel]);
  return m->priority[exec_shadow];
}
