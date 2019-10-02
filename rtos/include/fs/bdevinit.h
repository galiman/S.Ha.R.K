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
 * CVS :        $Id: bdevinit.h,v 1.2 2003/03/13 13:37:58 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2003/03/13 13:37:58 $
 */

#ifndef _HARTIK_BDEVINIT_H
#define _HARTIK_BDEVINIT_H

#include <kernel/model.h>

#include <fs/types.h>
#include <fs/const.h>
#include <fs/bdevconf.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/* prolog */
#define BASE_BDEV_START 0

/*
 * IDE
 */

#ifdef IDE_BLOCK_DEVICE

typedef struct ide_parms {
  void *parm_initserver;
} IDE_PARMS;

#define IDEPARMS(ptr) ((ptr)->ideparms)

#define BASE_BDEV_IDE BASE_BDEV_START,{NULL}

/*
typedef struct ide_parms {
  __uint8_t  pclass;
  __uint32_t dline;
  __uint32_t wcet;
} IDE_PARMS;

#define BASE_IDE {DEFAULT,DEFAULT,DEFAULT}

#define ide_def_task(s,p,d,w)  {(s).pclass=(p); (s).dline=(d); (s).wcet=(w);}
*/

#else

#define BASE_BDEV_IDE BASE_BDEV_START

#endif

/*
 * LOOP
 */

#ifdef LOOP_BLOCK_DEVICE

typedef struct loop_parms {
  int dummy;
} LOOP_PARMS;

#define BASE_BDEV_LOOP BASE_BDEV_IDE,{0}

#else

#define BASE_BDEV_LOOP BASE_BDEV_IDE

#endif

/* epilog */
#define BASE_BDEV_OPTIONAL BASE_BDEV_LOOP

/*
 * block device params structure
 */

extern NPP_mutexattr_t bdevdef_mutexattr;

typedef struct bdev_parms {
  __uint32_t showinfo:1;
  char       *config;
  void       *bmutexattr;
  
  __uint16_t dummy;
  
#ifdef IDE_BLOCK_DEVICE
  IDE_PARMS ideparms;
#endif

#ifdef LOOP_BLOCK_DEVICE
  LOOP_PARMS loopparms;
#endif

} BDEV_PARMS;

#define bdev_def_showinfo(s,v) ((s).showinfo=(v)&1)
#define bdev_def_configstring(s,v) ((s).config=(v))
#define bdev_def_mutexattrptr(s,v) ((s).bmutexattr=(v))

#define BASE_BDEV {0,NULL,(void*)&bdevdef_mutexattr,BASE_BDEV_OPTIONAL}

/**/

int bdev_init(BDEV_PARMS *);

#ifdef IDE_BLOCK_DEVICE
int ide_init(BDEV_PARMS *);
#ifndef NDEBUG
void ide_dump_status(void);
#endif
#endif

#ifdef LOOP_BLOCK_DEVICE
int loop_init(BDEV_PARMS *);
#ifndef NDEBUG
void loop_dump_status(void);
#endif
#endif

/*+ find a device using its name +*/
__dev_t bdev_find_byname(char *name);

/*+ find a device using its filesystem indicator +*/
__dev_t bdev_find_byfs(__uint8_t fsind);

/*+ scan all devices present +*/
int bdev_scan_devices(int(*callback)(__dev_t,__uint8_t));

__END_DECLS
#endif
