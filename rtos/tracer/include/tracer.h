/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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

#ifndef __TRACER_H__
#define __TRACER_H__

#ifdef __OLD_TRACER__

#include <ll/sys/types.h>
#include "types.h"
#include "trace.h"
#include "FTrace.h"

#define TRACER_LOGEVENT oldtrace

extern int (*trc_register_eventclass)
     (int class,int num,int(*func)(trc_event_t *, int event, void *ptr));

extern void (*trc_logevent)(int event, void *ptr);

extern int (*trc_resume)(void);
extern int (*trc_suspend)(void);

#define NOT_DEFINED 0xFF

extern __inline__ void oldtrace(WORD type, WORD par1, DWORD par2)
{

  static int oldtype;
  static int oldpar;
  extern BYTE conv[256]; // Conversion table: ctable.c
  BYTE ttype = 0;
 
  ttype = type & 0x000F;
  type = (type >> 4) & 0x000F;
  type |= (ttype << 4);

  oldtype = conv[type];

  oldpar = par1;

  if (oldtype != NOT_DEFINED) trc_logevent(oldtype,&oldpar);

}


#else
#ifdef __NEW_TRACER__

#include <ll/sys/types.h>
#include <ll/i386/hw-instr.h>
#include "FTrace.h"
#include "FTrace_chunk.h"
#include "FTrace_OSD.h"
#include "FTrace_udp.h"

#define TRACER_NO_OUTPUT  0x00
#define TRACER_UDP_OUTPUT 0x01

#define TRACER_LOGEVENT FTrace_safe_ipoint
#define FAST_TRACER_LOGEVENT FTrace_unsafe_ipoint

void FTrace_unsafe_ipoint(WORD type, WORD par1, DWORD par2);
void FTrace_safe_ipoint(WORD type, WORD par1, DWORD par2); 

#else

#include "FTrace.h"

#define TRACER_LOGEVENT(a,b,c)

#endif

#endif

#endif
