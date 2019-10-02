
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

// Header file that allows a general testing code to be used for calling
// the FSF primitives in both platforms: MaRTE_OS and SHARK

#ifndef _FSF_OS_COMPATIBILITY_H_
#define _FSF_OS_COMPATIBILITY_H_
#define SHARK_FSF
/////////////////////////////SHARK SECTION /////////////////////////////
#ifdef SHARK_FSF

#define  INITIALIZATION_CODE   \
{                              \
  fsf_init();                  \
  calibrate_cycle();           \
  init_network();              \
}

#define fsf_printf udp_print
#define printf cprintf
/*
double
t2d(struct timespec time)
{
	  return time.tv_nsec*0.000000001 + (double)time.tv_sec;
}
*/
#define t2d( ts ) ((ts).tv_nsec*0.000000001 + (double)((ts).tv_sec))

#define ERROR(nn, ss) { if(errno==0) errno=nn ; kern_printf(ss); exit(nn);}

#define STANDARD_CONSOLE_INIT { }
#define SERIAL_CONSOLE_INIT { }
#endif /* SHARK_FSF */
//////////////////////////END OF SHARK SECTION /////////////////////////


/////////////////////////////MARTE SECTION /////////////////////////////
#ifdef MARTE_FSF

#include <misc/serial_console.h>
#include <misc/load.h>

//#include <debug_marte.h>
#include "fsf_contract.h"
#include "fsf_types.h"


#define  INITIALIZATION_CODE \
{                                                                         \
	SERIAL_CONSOLE_INIT; /*marte1.26i+                              */\
        /*init_serial_communication_with_gdb(SERIAL_PORT_1);            */\
        /*printf("max_prio = %d\n", sched_get_priority_max(SCHED_FIFO));*/\
        /*printf("min_prio = %d\n", sched_get_priority_min(SCHED_FIFO));*/\
                                                                          \
        adjust();                                                         \
        /*set_break_point_here;                                         */\
}

#define ERROR(nn, ss) {if(errno==0) errno=(nn); perror(ss); exit (nn);}

int
fsf_priority_map (unsigned long plevel)
{
	return plevel;
}

#endif /* MARTE_FSF */
//////////////////////////END OF MARTE SECTION /////////////////////////

#endif /* _FSF_OS_COMPATIBILITY_H_ */
