
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

//fsf_implementation_specific.h
//===================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// FSF(FIRST Scheduling Framework) 
//implementation-defined optional functionality
//================================================

#include "fsf_basic_types.h"
#include "fsf_core.h"


#ifndef _FSF_IMPLEMENTATION_SPECIFIC_H_
#define _FSF_IMPLEMENTATION_SPECIFIC_H_

#define FSF_IMPLEMENTATION_SPECIFIC_MODULE_SUPPORTED     1


// The operations defined in this module are of optional use. They
// only work in the fixed priority implementation, and they may be
// used to enhance the behavior of the applications running under the
// fsf scheduler.

//// The definition of this type is in fsf_basic_types.h
//
//typedef unsigned long      fsf_preemption_level_t;
//                           // range 1..2**32-1


//fsf_set_contract_preemption_level: The operation updates the
//specified contract parameters object by setting its preemption level
//to the specified input parameter.

int
fsf_set_contract_preemption_level
  (fsf_contract_parameters_t     *contract,
   fsf_preemption_level_t         preemption_level);


//fsf_get_contract_preemption_level: The operation obtains from the
//specified contract parameters object its preemption level and copies
//it to the place pointed to by the specified input parameter.

int
fsf_get_contract_preemption_level
  (const fsf_contract_parameters_t *contract,
   fsf_preemption_level_t          *preemption_level);


//fsf_set_service_thread_preemption_level: this function sets the
//preemption level of the service thread to the specified value. The
//initial preemption level is a configurable parameter. This value
//is stored in a temporary variable and it is used the next time the
//service thread data is updated with the fsf_set_service_thread_data
//function

int
fsf_set_service_thread_preemption_level
  (fsf_preemption_level_t         preemption_level);

//fsf_get_service_thread_preemption_level: this function stores the
//current preemption level of the service thread in the variable
//pointed to by preemption_level

int
fsf_get_service_thread_preemption_level
  (fsf_preemption_level_t        *preemption_level);


//fsf_thread_exit: There is a limitation in the current version of the
//MaRTE implementation that causes the information of a terminated
//thread to continue to be stored in the fsf scheduler, and the thread
//to continue to be counted in the number of threads. The
//fsf_thread_exit operation allows the implementation to delete the
//thread's information, and then terminate the thread. Therefore, it
//is recommended to use this function to terminate a thread under fsf.

//This operation shall terminate the calling thread, make the value
//value_ptr available to any successful join with the terminating
//thread, and unbind the thread from its associated server. After
//cleaning up the thread management data, it is unbound and the
//scheduling policy is changed to fixed priority before the posix
//pthread_exit() function is called.

void
fsf_thread_exit (void *value_ptr);


//fsf_set_shared_obj_preemption_level: The operation updates the
//specified shared object by setting its preemption level
//to the specified input parameter. 
//OBSERVATION: if this value is changed being any contract that
//uses the resource already accepted, the system's behavior and
//particularly the acceptance tests correctness are not garantee
//and probably wrong.

int
fsf_set_shared_obj_preemption_level
  (fsf_shared_obj_handle_t  obj_handle,
   fsf_preemption_level_t   preemption_level);


//fsf_get_shared_obj_preemption_level: The operation obtains from the
//specified shared object its preemption level and copies
//it to the place pointed to by the specified input parameter.

int
fsf_get_shared_obj_preemption_level
  (fsf_shared_obj_handle_t  obj_handle,
   fsf_preemption_level_t  *preemption_level);



#endif // _FSF_IMPLEMENTATION_SPECIFIC_H_
