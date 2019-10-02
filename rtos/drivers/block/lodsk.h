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
 * CVS :        $Id: lodsk.h,v 1.2 2003/03/13 13:43:13 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:43:13 $
 */

#ifndef __LODSK_H__
#define __LODSK_H__

#include "glue.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define MSDOS_LABEL_MAGIC		0xAA55

struct phpartition {
  __uint8_t  boot_ind;
  __uint8_t  st_h;
  __uint8_t  st_s;
  __uint8_t  st_c;
  __uint8_t  sys_ind;
  __uint8_t  en_h;
  __uint8_t  en_s;
  __uint8_t  en_c;
  __uint32_t rel_sect;
  __uint32_t nr_sects;
} __attribute__((packed));

struct partition {
  __uint8_t  boot_ind:1;
  __uint8_t  sys_ind;
  __uint16_t st_cyl;
  __uint8_t  st_head;
  __uint8_t  st_sect;
  __uint16_t en_cyl;
  __uint8_t  en_head;
  __uint8_t  en_sect;
  __uint32_t rel_sect;
  __uint32_t nr_sects;
};

#define MAXLODSKNAME 8

struct lodskinfo {
  __uint8_t  fs_ind;
  __blkcnt_t start;
  __blkcnt_t size;
};

typedef int (*lodsk_callback_func)(int,struct lodskinfo*,void *);

int lodsk_scan(__dev_t device, lodsk_callback_func func,
	       void *data, int showinfo, char *lname);

__END_DECLS	       
#endif
