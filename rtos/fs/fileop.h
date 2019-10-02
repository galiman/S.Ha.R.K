/*
 * Project: HARTIK (HA-rd R-eal TI-me K-ernel)
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *               Gerardo Lamastra <gerardo@sssup.it>
 *
 * Authors     : Massimiliano Giorgi <massy@hartik.sssup.it>
 * (see authors.txt for full list of hartik's authors)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it 
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
 * CVS :        $Id: fileop.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 "Virtual" operations on files that must be exported by filesystem modules
***/

#ifndef __FILEOP_H__
#define __FILEOP_H__

#include <fs/types.h>

struct file_operations {
  __off_t   (*lseek)   (struct file *, __off_t, int);
  __ssize_t (*read)    (struct file *, char *, __ssize_t);
  __ssize_t (*write)   (struct file *, char *, __ssize_t);
  int       (*readdir) (struct file *, void *);
  int       (*open)    (struct inode *, struct file *);
  int       (*close)   (struct inode *, struct file *);
};

__off_t dummy_lseek(struct file *f, __off_t off, int whence);
__ssize_t dummy_read(struct file *f, char *p, __ssize_t d);
__ssize_t dummy_write(struct file *f, char *p, __ssize_t d);
int dummy_readdir(struct file *f, void *e);
int dummy_open(struct inode *i, struct file *f);
int dummy_close(struct inode *i, struct file *f);

#endif
