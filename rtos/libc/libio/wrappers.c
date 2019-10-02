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
 * CVS :        $Id: wrappers.c,v 1.2 2006/03/09 16:30:05 tullio Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.2 $
 * Last update: $Date: 2006/03/09 16:30:05 $
 */

#include <kernel/model.h>

#include <fs/types.h>
#include <fs/const.h>
#include <fs/stdarg.h>
#include <fs/syscall.h>
#include <fs/errno.h>
#include <fs/limits.h>
#include <fs/fcntl.h>
#include <fs/stat.h>
#include <fs/dirent.h>
#include <fs/mount.h>

#include "errno.h"
#include "mutex.h"

static NOP_mutexattr_t libio_mutexattr=NOP_MUTEXATTR_INITIALIZER;
mutexattr_t *libio_mutexattrptr=&libio_mutexattr;

int libio_initialize(void)
{
  extern int dir_initialize(void);  
  return dir_initialize();
}

/*
 *
 * SYSTEM CALLS
 *
 */

     


extern int k_open(const char *pathname, int oflag, int mode);

int open(const char *pathname, int oflag, ...)     
{
  int mode=0;
  int res;
  
  if (oflag & O_CREAT){
    va_list arg;
    va_start(arg, oflag);
    mode=va_arg(arg,int);
    va_end(arg);
  }

  res=SYS_CALL3(k_open,pathname,oflag,mode);
  
  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  return res;
}

/* --- */


/* --- */

/*
extern __ssize_t k_read(int fd, __ptr_t buf, __ssize_t nbytes);

__ssize_t read(int fd, __ptr_t buf, __ssize_t nbytes)
{
  int res;

  res=SYS_CALL3(k_read,fd,buf,nbytes);
  
  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  return res;
}

extern __ssize_t k_write(int fd, __ptr_t buf, __ssize_t nbytes);

__ssize_t write(int fd, __ptr_t buf, __ssize_t nbytes)
{
  int res;

  res=SYS_CALL3(k_write,fd,buf,nbytes);
  
  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  return res;
}
*/

/* --- */


/* --- */

extern int k_fstat(int fd, struct stat *buf);

int __fxstat(int version, int fd, struct stat *buf)
{
  int res;

  res=SYS_CALL2(k_fstat,fd,buf);
  
  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  return 0;
}

extern int k_stat(char *path, struct stat *buf);

int __xstat(int version, char *path, struct stat *buf)
{
  int res;

  res=SYS_CALL2(k_stat,path,buf);

  
  if (ISERROR(res)) {
    __set_errno(-res);    
    return -1;
  }

  return 0;
}

/* for inlines */
      
int stat (char *__path,struct stat *__statbuf)
{
  return __xstat (_STAT_VER, __path, __statbuf);
}

int fstat (int __fd, struct stat *__statbuf)
{
  return __fxstat (_STAT_VER, __fd, __statbuf);
}

/* --- */
/* --- */
/* --- */
/* --- */

int k_ioctl(int fs, unsigned long int request, void *arg)
{
  return -ENOSYS;
}

int ioctl(int fd, unsigned long int request, ...)
{
  int res;
  void *arg=NULL;
  
  res=SYS_CALL3(k_ioctl,fd,request,arg);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return 0;    
}

/* --- */

extern int k_fdevice(char *device_name);

__dev_t fdevice(char *device_name)
{
  int res;
  
  res=SYS_CALL1(k_fdevice,device_name);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return res;      
}

/* --- */

extern int k_mount(__dev_t device, __uint8_t fsind,
		   char *where, struct mount_opts *);

int mount(__dev_t device, __uint8_t fsind,
	  char *where, struct mount_opts *options)
{
  int res;
  
  res=SYS_CALL4(k_mount,device,fsind,where,options);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return 0;    
}

extern int k_umount(__dev_t device);

int umount(__dev_t device)
{
  int res;
  
  res=SYS_CALL1(k_umount,device);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return 0;    
}

/* --- */

int isatty(int fd)
{
  return 0;
}

/* --- */

extern int k_chdir(char *path);

int chdir(char *path)
{
  int res;
  
  res=SYS_CALL1(k_chdir,path);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return 0;    
}

extern int k_access(char *path,int amode);

int access(char *path,int amode)
{
  int res;
  
  res=SYS_CALL2(k_access,path,amode);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return 0;    
}

extern int k_utime(char *path,void *);

int utime(char *path,void *ptr)
{
  int res;
  
  res=SYS_CALL2(k_utime,path,ptr);

  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  
  return 0;    
}

/* --- */

extern int k_fcntl(int fildes, int cmd, void *ptr);

int fcntl(int fildes, int cmd, ...)     
{
  void *ptr;
  int mode;
  int res;

  ptr=NULL;
  if (cmd==F_SETFL||cmd==F_SETFD||cmd==F_DUPFD){
    va_list arg;
    mode=0;
    va_start(arg, cmd);
    mode=va_arg(arg,int);
    va_end(arg);
    ptr=(void *)mode;
  } else if (cmd==F_GETLK||cmd==F_SETLK||cmd==F_SETLKW) {
    va_list arg;
    va_start(arg, cmd);
    ptr=va_arg(arg,void *);
    va_end(arg);
  }

  res=SYS_CALL3(k_fcntl,fildes,cmd,ptr);
  
  if (ISERROR(res)) {
    __set_errno(-res);
    return -1;
  }
  return res;
}



/* --- */

