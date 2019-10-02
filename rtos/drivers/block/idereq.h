
/*
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


#ifndef __IDEREQ_H__
#define __IDEREQ_H__

#include "bqueue.h"
#include "glue.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define MAXIDEREQUEST 64

/* -------- */

typedef struct TAGidereq_t {

  request_prologue_t info;

  int next;
  
  __uint8_t  resetonerror; /*+ request/reply soft reset on error (see code) +*/
  
  __uint8_t  cmd;
  __b_sem_t  wait;
  int        result;

  __uint8_t  features;
  __uint8_t  cyllow;
  __uint8_t  cylhig;
  __uint8_t  seccou;
  __uint8_t  secnum;
  __uint8_t  devhead;
  
  __uint8_t  *buffer;
} idereq_t;

extern idereq_t idereq[];

/* initialization */
void init_idereq(void);

/* get a request from the free queue */
int get_idereq(void);
/* insert a request into the free queue */
void free_idereq(int req);

/* insert a request into an ide interface struct */
int insert_idereq(int ideif, int drive, int req);
/* release a blocked queue of an ide interface*/
int releasequeue_idereq(int ideif);

/* get the first request to serve */
int first_idereq(int ideif);
/* get the actual request that we are serving */
int actual_idereq(int ideif);
/* remove the request that we have served */
int remove_idereq(int ideif);
/* remove the request that we have served blocking the queue*/
int remove_idereq_blocking(int ideif);

__END_DECLS
#endif
