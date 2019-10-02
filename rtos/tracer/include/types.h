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
 * CVS :        $Id: types.h,v 1.1 2003/12/10 16:48:47 giacomo Exp $
 *
 * File:        $File$
 * Revision:    $Revision: 1.1 $
 * Last update: $Date: 2003/12/10 16:48:47 $
 */

#ifndef __TRACE_TYPES_H
#define __TRACE_TYPES_H

/*
 * EVENTS
 */

/* tracer class */
#define TRC_RESERVED      0x00
#define TRC_HOOPS         0x01

/* system class */
#define TRC_CREATE        0x02
#define TRC_ACTIVATE      0x03
#define TRC_SCHEDULE      0x04
#define TRC_DELAY         0x05
#define TRC_SLEEP         0x06
#define TRC_ENDCYCLE      0x07
#define TRC_DESTROY       0x08
#define TRC_INTACTIVATION 0x09
#define TRC_DISABLE	  0x0a

/* user class */
#define TRC_USER0         0x0b
#define TRC_USER1         0x0c 
#define TRC_USER2         0x0d
#define TRC_USER3         0x0e
#define TRC_USER4         0x0f
#define TRC_USER5         0x10
#define TRC_USER6         0x11
#define TRC_USER7         0x12

/* low level class */
#define TRC_INTR          0x13

/* semaphore class */
#define TRC_SEM_WAIT      0x14
#define TRC_SEM_WAITNB    0x15
#define TRC_SEM_SIGNAL    0x16

/* used internal by the tracer (the first event of a class) */
#define TRC_F_TRACER  TRC_RESERVED
#define TRC_F_SYSTEM  TRC_CREATE
#define TRC_F_USER    TRC_USER0
#define TRC_F_LL      TRC_INTR
#define TRC_F_SEM     TRC_SEM_WAIT
#define TRC_F_LAST    (TRC_SEM_SIGNAL+1)

/* classes */
#define TRC_CLASS_TRACER  0
#define TRC_CLASS_SYSTEM  1
#define TRC_CLASS_USER    2
#define TRC_CLASS_LL      3
#define TRC_CLASS_SEM     4

/* max values */
#define TRC_NUMCLASSES    5
#define TRC_NUMEVENTS     TRC_F_LAST

/*
 * STRUCTURES
 */

/* tracer class */
typedef struct TAGtrc_tracer_event_t {
  u_int32_t n;
} trc_tracer_event_t __attribute__ ((packed));

/* system class */
typedef struct TAGtrc_system_event_t {
  u_int16_t task;
} trc_system_event_t __attribute__ ((packed));

/* user class */
typedef struct TAGtrc_user_event_t {
#define TRC_MAXUSERINFO sizeof(long)
  long n;
} trc_user_event_t __attribute__ ((packed));

/* low level class */
typedef struct TAGtrc_ll_event_t {
  int n;
} trc_ll_event_t __attribute__ ((packed));

/* semaphore class */
typedef struct TAGtrc_sem_event_t {
  u_int16_t task;
  u_int16_t id;
} trc_sem_event_t __attribute__ ((packed));

/* generics event */
typedef union TAGtrc_allevents {
  trc_tracer_event_t trc;
  trc_system_event_t sys;
  trc_user_event_t   usr;
  trc_ll_event_t     ll;
  trc_sem_event_t    sem;
} trc_allevents_t __attribute__ ((packed));;

/* event struct */
typedef struct TAGtrc_event_t {
  u_int32_t       time;
  u_int16_t       event;
  trc_allevents_t x;
} trc_event_t __attribute__ ((packed));


#endif
