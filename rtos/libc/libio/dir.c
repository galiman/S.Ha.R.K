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
 * CVS :        $Id: dir.c,v 1.1.1.1 2002/03/29 14:12:53 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:53 $
 */

#include <features.h>

#include <fs/syscall.h>

//#define __need_FOPEN_MAX
//#include <bits/stdio_li.h>

#include <sys/types.h>
#include <limits.h>
#include <dirent.h>

#include "errno.h"
#include "mutex.h"

#define MAXOPENDIR 256

struct __dirstream {
  int           dir_next;
  
  int           dir_h;
  struct dirent dir_e;

  int32_t       dir_used:1;
  int32_t       dir_inprogress:1;
};

#define NIL -1

static DIR     direntry[MAXOPENDIR];
static int     freedirentry;
static _priv_mutex_t direntrymutex;

extern int k_opendir(const char *pathname);
extern int k_closedir(int fd);
extern int k_readdir(int fd, struct dirent *den);

int dir_initialize(void)
{
  int i;
  _priv_mutex_init(&direntrymutex);

  //cprintf("<mutex on %p [DIR_INITIALIZE]>",&direntrymutex);
    
  freedirentry=0;
  for (i=0;i<MAXOPENDIR;i++) {
    direntry[i].dir_next=i+1;
    direntry[i].dir_h=-1;
    direntry[i].dir_used=0;
    direntry[i].dir_inprogress=0;
  }
  direntry[MAXOPENDIR-1].dir_next=NIL;
  return 0;
}

DIR *opendir(const char *pathname)     
{
  int ind,res;
  
  _priv_mutex_lock(&direntrymutex);
  ind=freedirentry;
  if (ind!=NIL) freedirentry=direntry[ind].dir_next;
  _priv_mutex_unlock(&direntrymutex);

  if (ind==NIL) {
    __set_errno(EMFILE);
    return NULL;
  }
  
  res=SYS_CALL1(k_opendir,pathname);
  
  if (ISERROR(res)) {

    _priv_mutex_lock(&direntrymutex);
    direntry[ind].dir_next=freedirentry;
    freedirentry=ind;
    _priv_mutex_unlock(&direntrymutex);
    
    __set_errno(-res);
    return NULL;
  }

  direntry[ind].dir_h=res;
  direntry[ind].dir_inprogress=0;
  direntry[ind].dir_used=1;
  
  return direntry+ind;
}

int closedir(DIR *d)
{
  int res;

  if (d<direntry||d>direntry+MAXOPENDIR) {
    __set_errno(EBADF);
    return -1;
  }

  _priv_mutex_lock(&direntrymutex);
  if (d->dir_used==0) {
    _priv_mutex_unlock(&direntrymutex);
    __set_errno(EBADF);
    return -1;
  }
  if (d->dir_inprogress) {
    _priv_mutex_unlock(&direntrymutex);
    __set_errno(EBUSY);
    return -1;
  }    
  d->dir_used=0;
  _priv_mutex_unlock(&direntrymutex);
  
  res=SYS_CALL1(k_closedir,d->dir_h);
  
  if (ISERROR(res)) {
    d->dir_used=1;
    __set_errno(-res);
    return -1;
  }

  _priv_mutex_lock(&direntrymutex);
  d->dir_h=-1;
  d->dir_used=0;
  d->dir_inprogress=0;
  d->dir_next=freedirentry;
  freedirentry=d-direntry; 
  _priv_mutex_unlock(&direntrymutex);
  
  return 0;  
}

struct dirent *readdir(DIR *d)
{
  int res;

  if (d<direntry||d>direntry+MAXOPENDIR) {
    __set_errno(EBADF);
    return NULL;
  }
  
  _priv_mutex_lock(&direntrymutex);
  if (!d->dir_used) {
    _priv_mutex_unlock(&direntrymutex);
    __set_errno(EBADF);
    return NULL;
  }
  d->dir_inprogress=1;
  _priv_mutex_unlock(&direntrymutex);
  
  res=SYS_CALL2(k_readdir,d->dir_h,&d->dir_e);
  d->dir_inprogress=0;
  if (res==1) return NULL;

  if (ISERROR(res)) {
    __set_errno(-res);
    return NULL;
  }

  return &d->dir_e;
}

int readdir_t(DIR *d,struct dirent *den, struct dirent **result)
{
  int res;

  if (d<direntry||d>direntry+MAXOPENDIR) {
    __set_errno(EBADF);
    return -1;
  }

  _priv_mutex_lock(&direntrymutex);
  if (!d->dir_used) {
    _priv_mutex_unlock(&direntrymutex);
    __set_errno(EBADF);
    return -1;
  }
  d->dir_inprogress=1;
  _priv_mutex_unlock(&direntrymutex);
  
  res=SYS_CALL2(k_readdir,d->dir_h,den);
  d->dir_inprogress=0;
  
  if (res==1) {
    *result=NULL;
    return 0;
  }

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }

  return 0;
}
