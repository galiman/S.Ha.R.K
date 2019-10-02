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
 * CVS :        $Id: file.h,v 1.1.1.1 2002/03/29 14:12:50 pj Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1.1.1 $
 * Last update: $Date: 2002/03/29 14:12:50 $
 */

/***
 This files contains the "file" structure that rapresents??? a file in the system and functions
 to manage the local descriptors table of every process (not task).
***/

#ifndef __FILE_H__
#define __FILE_H__

#include <fs/types.h>
#include <fs/limits.h>
#include <fs/fsconf.h>
#include <fs/task.h>
#include <fs/assert.h>

#ifdef MSDOS_FILESYSTEM
#include "msdos/msdos_f.h"
#endif

#include "mutex.h"
#include "inode.h"

struct file {
  struct file            *f_next;

  struct dentry		 *f_dentry;
  struct file_operations *f_op;

  
  /*__uint32_t		 f_flags;  */
  __loff_t		 f_pos;

  __uint32_t             f_flag_isdir:1;

  unsigned int 		 f_count; /* number of fd that point to this */
  
  union {
#ifdef MSDOS_FILESYSTEM
    struct msdos_file_info msdos_f;
#endif
    unsigned               dummy; 
  } u;
  
  unsigned long 	 f_reada, f_ramax, f_raend, f_ralen, f_rawin;
  unsigned int		 f_uid, f_gid;
  int			 f_error;
  unsigned long		 f_version;
};

struct file_descriptors {  
  struct file   *fd_file[MAXOPENFILES];

  __uint16_t    fd_flags[MAXOPENFILES];  /* FD_CLOEXEC*/
  __uint32_t    fd_fflags[MAXOPENFILES]; /* O_CREAT, O_SYNC, O_RDONLY, ...*/
  
  __mode_t      fd_umask;
  char          fd_cwd[MAXPATHNAMELEN+1];
  struct dentry *fd_cwden;
#ifdef MULTITHREAD
  __mutex_t     fd_mutex;
#endif
};

/* if fd_file[fd] is
 * NULL       -> fd is free
 * RESERVEDFD -> fd is reserved for console i/o
 * other      -> fd is usuable
 */
#define RESERVEDFD ((void*)-1)

int file_init(void);

struct file *get_file(struct inode *ptr);
void free_file(struct file *f);

int get_fd(__pid_t pid, struct file *f);
struct file *free_fd(__pid_t pid, int fd);

void enums_file(void (*func)(struct file *));

/* does not aquire or release a lock! */
int __isvalid_fd(__pid_t pid, int fd);

int __duplicate_fd(__pid_t pid, int fd, int low);

/* ------ */

extern struct file_descriptors desctable[];

#ifdef MULTITHREAD

extern __inline__ void lock_desctable(__pid_t pid)
{
  __fs_mutex_lock(&desctable[pid].fd_mutex);
}

extern __inline__ void unlock_desctable(__pid_t pid)
{
  __fs_mutex_unlock(&desctable[pid].fd_mutex);
}

extern __inline__ void init_mutex(__pid_t pid)
{
  __fs_mutex_init(&desctable[pid].fd_mutex);
}

#else

#define lock_desctable(pid)

#define unlock_desctable(pid)

#define init_mutex(pid)

#endif

extern __inline__ __mode_t *umask_ptr(__pid_t pid)
{
  return &desctable[pid].fd_umask;
}

extern __inline__ char *cwd_ptr(__pid_t pid)
{
  return desctable[pid].fd_cwd;
}

extern __inline__ struct dentry *cwden_ptr(__pid_t pid)
{
  return desctable[pid].fd_cwden;
}

extern __inline__ struct file *file_ptr(__pid_t pid, int i)
{
  return desctable[pid].fd_file[i];
}

extern __inline__ int check_fd(__pid_t pid, int fd)
{
  if (fd<0||fd>=MAXOPENFILES) return -1;
  _assert(pid>=0&&pid<MAXIMUMPROCESS);
  if (desctable[pid].fd_file[fd]==NULL) return -1;
  if (desctable[pid].fd_file[fd]==RESERVEDFD) return -1;
  if (desctable[pid].fd_file[fd]->f_count==0) return -1;
  return 0;
}

extern __inline__ __uint16_t *fildesflags_ptr(__pid_t pid, int i)
{
  return &desctable[pid].fd_flags[i];
}

extern __inline__ __uint32_t *fildesfflags_ptr(__pid_t pid, int i)
{
  return &desctable[pid].fd_fflags[i];
}

#endif
