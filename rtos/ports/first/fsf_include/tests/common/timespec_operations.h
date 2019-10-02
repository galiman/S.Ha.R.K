/*----------------------------------------------------------------------------
 *-------------------------      M a R T E   O S      ------------------------
 *----------------------------------------------------------------------------
 *                                                           V1.0  Dec 2001
 *
 *                    't i m e s p e c _ o p e r a t i o n s'
 *
 *                                      H
 *
 * File 'timespec_operations.h'                                        by MAR.
 *                                (july 2002)  transformed into macros by Jul.
 * Some basic operations with the type 'timespec'.
 *
 * ----------------------------------------------------------------------
 *  Copyright (C) 2001   Universidad de Cantabria, SPAIN
 *
 *  Authors: Mario Aldea Rivas          aldeam@ctr.unican.es
 *           Michael Gonzalez Harbour      mgh@ctr.unican.es
 *
 * MaRTE OS  is free software; you can  redistribute it and/or  modify it
 * under the terms of the GNU General Public License  as published by the
 * Free Software Foundation;  either  version 2, or (at  your option) any
 * later version.
 *
 * MaRTE OS  is distributed  in the  hope  that  it will be   useful, but
 * WITHOUT  ANY  WARRANTY;     without  even the   implied   warranty  of
 * MERCHANTABILITY  or  FITNESS FOR A  PARTICULAR PURPOSE.    See the GNU
 * General Public License for more details.
 *
 * You should have received  a  copy of  the  GNU General Public  License
 * distributed with MaRTE  OS;  see file COPYING.   If not,  write to the
 * Free Software  Foundation,  59 Temple Place  -  Suite 330,  Boston, MA
 * 02111-1307, USA.
 *
 * As a  special exception, if you  link this  unit  with other  files to
 * produce an   executable,   this unit  does  not  by  itself cause  the
 * resulting executable to be covered by the  GNU General Public License.
 * This exception does  not however invalidate  any other reasons why the
 * executable file might be covered by the GNU Public License.
 *
 *---------------------------------------------------------------------------*/

#ifndef	_MARTE_MISC_TIMESPEC_OPERATIONS_H_
#define _MARTE_MISC_TIMESPEC_OPERATIONS_H_

#include <time.h>

#define smaller_timespec(t1, t2) \
 ( \
  (t1)->tv_sec < (t2)->tv_sec || ((t1)->tv_sec == (t2)->tv_sec &&   \
				     (t1)->tv_nsec < (t2)->tv_nsec) \
 )

#define smaller_or_equal_timespec(t1, t2) \
 ( \
  (t1)->tv_sec < (t2)->tv_sec || ((t1)->tv_sec == (t2)->tv_sec &&    \
				     (t1)->tv_nsec <= (t2)->tv_nsec) \
 )

#define incr_timespec(t1, t2) \
{ \
  (t1)->tv_sec += (t2)->tv_sec; \
  (t1)->tv_nsec += (t2)->tv_nsec; \
  if ((t1)->tv_nsec >= 1000000000) { \
    (t1)->tv_sec ++; \
    (t1)->tv_nsec -= 1000000000; \
  } \
}

#define decr_timespec(t1, t2) \
{ \
  if ((t1)->tv_nsec < (t2)->tv_nsec) { \
    (t1)->tv_sec -= (t2)->tv_sec + 1; \
    (t1)->tv_nsec = (t1)->tv_nsec + 1000000000 - (t2)->tv_nsec; \
  } else { \
    (t1)->tv_sec -= (t2)->tv_sec; \
    (t1)->tv_nsec -= (t2)->tv_nsec; \
  } \
}


#define  add_timespec( sum , t1 , t2 ) \
{ \
  (sum)->tv_sec  = (t1)->tv_sec  + (t2)->tv_sec; \
  (sum)->tv_nsec = (t1)->tv_nsec + (t2)->tv_nsec; \
  if ((sum)->tv_nsec >= 1000000000) { \
    (sum)->tv_sec ++; \
    (sum)->tv_nsec -= 1000000000; \
  } \
}

#define float_to_timespec( f1 , t1 ) \
( \
  (t1)->tv_sec = (int)(f1), \
  (t1)->tv_nsec = (int)(((f1)-(float)((t1)->tv_sec))*1000000000.0), \
  (t1) \
)

#define float_to_timespec_value(f1, t1) \
( \
  (t1).tv_sec = (int)(f1), \
  (t1).tv_nsec = (int)(((f1)-(float)((t1).tv_sec))*1000000000.0), \
  (t1) \
)

#endif /* _MARTE_MISC_TIMESPEC_OPERATIONS_H_ */

