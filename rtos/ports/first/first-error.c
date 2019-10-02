
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

//=====================================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST      
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS 
//       
// Basic FSF(FIRST Scheduling Framework) contract management
// S.Ha.R.K. Implementation
//=====================================================================

#include "fsf.h"
#include <string.h>
  
static char *fsf_msg[]={
 "TOO_MANY_TASKS                 ",
 "BAD_ARGUMENT                   ",
 "INVALID_SYNCH_OBJ_HANDLE       ",
 "NO_RENEGOTIATION_REQUESTED     ",
 "CONTRACT_REJECTED              ",
 "NOT_SCHEDULED_CALLING_THREAD   ",
 "NOT_BOUND_THREAD               ",
 "UNKNOWN_SCHEDULED_THREAD       ",
 "NOT_CONTRACTED_SERVER          ",
 "NOT_SCHEDULED_THREAD           ",
 "TOO_MANY_SERVICE_JOBS          ",
 "TOO_MANY_SYNCH_OBJS            ",
 "TOO_MANY_SERVERS_IN_SYNCH_OBJ  ",
 "TOO_MANY_EVENTS_IN_SYNCH_OBJ   ",
 "INTERNAL_ERROR                 ",
 "TOO_MANY_SERVERS               ",
 "INVALID_SCHEDULER_REPLY        ",
 "TOO_MANY_PENDING_REPLENISHMENTS",
 "SYSTEM_ALREADY_INITIALIZED     ",
 "SHARED_OBJ_ALREADY_INITIALIZED ",
 "SHARED_OBJ_NOT_INITIALIZED     ",
 "SCHED_POLICY_NOT_COMPATIBLE    ",
 "SERVER_WORKLOAD_NOT_COMPATIBLE ",
 "ALREADY_BOUND                  ",
 "WRONG_NETWORK                  ",
 "TOO_LARGE                      ",
 "BUFFER_FULL                    ",
 "NO_SPACE                       ",
 "NO_MESSAGES                    ",
 "MODULE_NOT_SUPPORTED           ",
 "SYSTEM_NOT_INITIALIZED         ",
 "TOO_MANY_SHARED_OBJS           ",
};

int fsf_strerror (int error, char *message, size_t size) 
{
  if (message && size>0 && error>FSF_ERR_BASE_VALUE && error<=FSF_ERR_LAST_VALUE) {
    strncpy(message,  fsf_msg[error-FSF_ERR_BASE_VALUE-1], size);
  } else return FSF_ERR_BAD_ARGUMENT;
  return 0;
}
