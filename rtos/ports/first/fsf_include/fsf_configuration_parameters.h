
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

//fsf_configuration_parameters.h
//===============================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// Basic FSF(FIRST Scheduling Framework) configuration parameters
//==============================================================

//////////////////////////////////////////////////////
// Definitions required to configure the fsf 
// scheduling algorithms.

#include <unistd.h>
#include "stdbool.h"

#ifndef _FSF_CONFIGURATION_PARAMETERS_H_
#define _FSF_CONFIGURATION_PARAMETERS_H_

// This symbol specifies whether the scheduler will make a
// schedulability test of the requested contract or not

#define FSF_ADMISSION_TEST_IS_ENABLED        true


// Tune these values for optimizing the amount of memory used by the
// implementation

// Maximum number of accepted contracts (servers)

#define FSF_MAX_N_SERVERS                    125 


// Maximum number of threads that may be scheduled by the framework

#define FSF_MAX_N_THREADS                    25


// Maximum number of critical sections that can be stored in a
// contract parameters object

#define FSF_MAX_N_CRITICAL_SECTIONS          25


// Maximum number of utilization values (pairs of budget and period)
// that can be stored in a contract parameters object

#define FSF_MAX_N_UTILIZATION_VALUES         5


// Maximum number of synchronization objects

#define FSF_MAX_N_SYNCH_OBJECTS              5


// Maximum number of shared objects

#define FSF_MAX_N_SHARED_OBJECTS             100


////////////////////////////////////////////
// Other implementation dependent parameters

// The current implementation in MaRTE OS uses the Application-Defined
// Scheduling Interface (proposed to the POSIX standardization
// committee), to create a fixed-priority-based scheduler that
// operates under the rules of the FIRST scheduling framework.

// In this implementation there are two special threads:
//   - The application scheduler thread, that 
//     implements the scheduler

//   - The service thread, that is in charge of 
//     negotiating and renegotiating contracts 
//     concurrently with the application

// The following symbols are necessary to adapt the application to the
// underlying fixed priority scheduler

// Priority assigned to the application scheduler; it should be above
// the priorities of the application threads and of the service
// thread, and it should be at least 1 level below the maximum of the
// system

#define FSF_SCHEDULER_PRIORITY               29 


// Real-time signal number reserved for the application scheduler to
// manage its timers.

#define FSF_SCHEDULER_SIGNAL                 SIGRTMIN


// The highest priority that can be assigned to an application thread,
// it should be defined as one level less than the
// FSF_SCHEDULER_PRIORITY

#define FSF_HIGHEST_THREAD_PRIORITY  FSF_SCHEDULER_PRIORITY-1


// The lowest priority that can be assigned to an application thread,
// it should be at least 1 level above the minimum of the system

#define FSF_LOWEST_THREAD_PRIORITY           3

// Each call to the functions that negotiate or renegotiate a contract
// or that change the quality and importance generates a request for
// the service thread that we call a service job.  This job will be
// pending in a queue until executed by the service thread.  The
// following symbol represents the maximum number of requests that can
// be simultaneously queued.
#define FSF_MAX_N_SERVICE_JOBS  FSF_MAX_N_SERVERS * 2

// In order to bound the background activity of the scheduler (i.e.,
// the admission tests necessary for the negotiation and
// re-negotiation of contracts), a service thread has been defined. It
// runs at a given priority level and has a budget and period
// assigned.


// Initial period of the service thread (timespec)

#define FSF_SERVICE_THREAD_PERIOD  {0,10000000}  //0.01 seg  //3.1 0.01 


// Initial budget of the service thread (timespec)

#define FSF_SERVICE_THREAD_BUDGET  {0,1000000}   //0.001 seg //3.1 0.001 


// Initial priority of the service thread, it has to be lower than the
// FSF_SCHEDULER_PRIORITY, and is set according to its period and the
// expected response times for reconfiguration or tunning of the
// system.

#define FSF_SERVICE_THREAD_PRIORITY          FSF_LOWEST_THREAD_PRIORITY+1 //3 3    //3 1



//Maximum number of servers that can be simultaneusly waiting for
//being signaled in a synchronization object

#define FSF_MAX_N_SERVERS_IN_SYNCH_OBJECT    4


//Maximum number of events that can be pending to be signaled in a
//synchronization object

#define FSF_MAX_N_EVENTS_IN_SYNCH_OBJECT    100  


//Maximum number of pending replenishments in each sporadic server

#define FSF_MAX_N_PENDING_REPLENISHMENTS     250


//Maximum number of target windows in a table driven schedule

#define FSF_MAX_N_TARGET_WINDOWS             100

//The cpu time given by the round robin scheduler 
//to the threads in the background (timespec)

#define FSF_RR_SLICE_CPU_TIME               {0,100000000}  //3  0.1 sec

// This function must be supplied by the user to map the preemption
// level values given in the contracts for the servers, to priority
// values in the range that is allowed by the present implementation
// for application threads. The value returned by the function must
// fit in the interval defined by the constants:
// [FSF_LOWEST_THREAD_PRIORITY, FSF_HIGHEST_THREAD_PRIORITY]

int
fsf_priority_map (unsigned long plevel);

// This symbol specifies the maximum number of chars that are stored
// of a given shared_object_id, so this is the maximum length used
// in comparissons

#define FSF_MAX_SIZE_SHARED_OBJ_ID           65


//Maximum number of networks accesible from a node

#define FSF_MAX_N_NETWORK_IDS                1

//Maximum number of servers that can be given in a servers list
//to the group negotiation/cancellation primitive

#define FSF_MAX_N_SERVER_VALUES              10

//Maximum number of contracts that can be given in a contracts list
//to the group negotiation/cancellation primitive

#define FSF_MAX_N_CONTRACT_VALUES              10

#endif /* _FSF_CONFIGURATION_PARAMETERS_H_ */
