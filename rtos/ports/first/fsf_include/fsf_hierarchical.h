
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

//fsf_hierarchical.h
//====================================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// FSF(FIRST Scheduling Framework) 
// hierarchical scheduling management
//===================================================================

#include <time.h>
#include "fsf_basic_types.h"
#include "fsf_core.h"

#ifndef _FSF_HIERARCHICAL_H_
#define _FSF_HIERARCHICAL_H_

#define FSF_HIERARCHICAL_MODULE_SUPPORTED       1

//// The definition of this types is in fsf_basic_types.h
//
//// Scheduling policies
//typedef enum {FSF_FP, FSF_EDF, FSF_TABLE_DRIVEN, FSF_NONE} 
//    fsf_sched_policy_t;
//
//// Scheduling policy and parameters
//typedef struct {
//  fsf_sched_policy_t    policy;
//  void *                params;
//} fsf_sched_params_t;
//// The params member is a pointer to one of the 
//// following:
////    FP:  int (priority)
////    EDF: struct timespec (deadline)
////    TABLE_DRIVEN : struct fsf_table_driven_params_t
//
//
////Scheduling parameters for the table-driven policy (t.b.d)
//typedef struct {
//  // list of target windows (t.b.d.) 
//  // deadline (for the API): end of september
//} fsf_table_driven_params_t;
//
//
////Initialization information for a scheduling policy
//typedef void * fsf_sched_init_info_t;
//// It shall be one of the following:
////    FP:  none
////    EDF: none
////    TABLE_DRIVEN : struct timespec (schedule duration)
//


/**
   \ingroup hiermodule   

   This call has the following effects:

    - FP: none

    - EDF: none

    - TABLE_DRIVEN : Records the schedule duration, and starts the
       schedule at the time of the call. After the schedule duration
       has elapsed, the schedule in the table is repeated.

    - RR : TBD

     @param [in] server server id

     @param [in] info TBD

     @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument 
            is not in range or info is NULL
     @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
            scheduled under the FSF
     @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
             running
     @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling 
             thread has been cancelled or it is not valid
*/
int fsf_init_local_scheduler(
   fsf_server_id_t       server,
   fsf_sched_init_info_t info);


/////////////////////////////////////////////////
//                       CONTRACT PARAMETERS
////////////////////////////////////////////////

/**
   \ingroup hiermodule

   The operation updates the specified contract parameters object by
   setting its scheduling policy to the specified input parameter.
   The default policy is FSF_NONE, which means that only one thread
   may be bound to the server

   @param [in] contract pointer to the contract
   @param [in] sched_policy local scheduling policy for this server. 
     Can be FSF_FP, FSF_EDF, FSF_TABLE_DRIVEN, FSF_NONE.

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if sched_policy is not one of the supported 
       ones, or contract is NULL
   
*/
int
fsf_set_contract_scheduling_policy
  (fsf_contract_parameters_t *contract,
   fsf_sched_policy_t         sched_policy);


/**
   \ingroup hiermodule

   This operation obtains from the specified contract parameters
   object its scheduling policy, and copies it to the place pointed to
   by the corresponding input parameter.

   @param [in] contract pointer to the contract
   @param [out] sched_policy pointer to a variable that will contain 
                the scheduling policy

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if sched_policy or contract are NULL
   
*/
int
fsf_get_contract_scheduling_policy
  (const fsf_contract_parameters_t *contract,
   fsf_sched_policy_t              *sched_policy);


/**
   \ingroup hiermodule

   This operation creates a thread and binds it to the specified
   server that has a local scheduler, i.e. policy different than FSF_NONE. The new
   thread is created with the arguments thread, attr, thread_code and
   arg as they are defined for the pthread_create() POSIX function
   call, and its local scheduling parameters are set to the value
   stored in the variable pointed to by sched_params, which must be
   compatible with the server's scheduling policy. Then, the function
   binds the created thread to the new server. The attr parameter is
   overwritten as necessary to introduce the adequate scheduling
   policy and priority, according to the preemption level given in the
   contract and the fsf_priority_map() function defined by the user.

   @param [in] server server id
   @param [in] sched_params scheduling parameters for the thread
   @param [out] thread the thread id after creation
   @param [in] attr attributes for the task (see pthread_create())
   @param [in] thread_code pointer to a function that implements the
       thread code
   @param [in] arg arguments for the thread
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument 
           is not in range, or sched_params is NULL
   @retval FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE if the scheduling policy 
       in sched_params is not compatible to the server's one.
   @retval FSF_ERR_INTERNAL_ERROR erroneous binding or malfunction of the FSF
       main scheduler
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the referenced server is not valid
     
   @retval others It may also return any of  the errors that may be 
       returned by the pthread_create()POSIX function call
*/

int
fsf_create_local_thread
  (fsf_server_id_t        server,
   fsf_sched_params_t    *sched_params,
   pthread_t             *thread,
   pthread_attr_t        *attr,
   fsf_thread_code_t      thread_code,
   void                  *arg);

/**
   \ingroup hiermodule

   This operation associates a thread with a server that has a local
   scheduler, i.e. with a policy different than FSF_NONE. The thread's
   local scheduling parameters are set to the value stored in the
   variable pointed to by sched_params, which must be compatible with
   the server's scheduling policy. After the call the thread starts
   consuming the server's budget and is executed according to the
   contract established for that server and to its scheduling
   policy. If the thread was already bound to another server, it is
   effectively unbound from it and bound to the specified one.
     
   <i>Implementation dependent issue for MARTE OS: In order to allow the
   usage of application defined schedulers, the given thread must not
   have the scheduling policy SCHED_APP and at the same time be
   attached to an application scheduler different than the fsf
   scheduler.</i>

   @param [in] server server id
   @param [in] thread thread id
   @param [in] sched_params scheduling parameters for the thread

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if the server argument does not comply with
       the expected format or valid range, the given thread does not exist,
       or sched_params is NULL
   @retval FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE if the scheduling policy 
       in sched_params is not compatible to the server's one.
   @retval FSF_ERR_INTERNAL_ERROR erroneous binding or malfunction of the FSF
       main scheduler
   @retval FSF_ERR_UNKNOWN_APPSCHEDULED_THREAD if the thread is attached to
       an application defined scheduler different than the fsf scheduler
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the referenced server is not valid
   @retval FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE: if the kind of workload
       of the server is FSF_OVERHEAD
*/
int  
fsf_bind_local_thread_to_server
  (fsf_server_id_t      server,
   pthread_t            thread,
   fsf_sched_params_t  *sched_params);
     

/**
   \ingroup hiermodule
   
   This function changes the local scheduling parameters of the thread
   to the value pointed to by sched_params. This value must be
   compatible with the scheduling policy of the server to which the
   thread is bound.

   @param [in] thread thread id

   @param [in] sched_params scheduling parameters

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if the given thread does not exist,
       or sched_params is NULL
   @retval FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE if the thread is already bound
       and the scheduling policy in sched_params is not compatible to the
       one of the thread's server.
   @retval FSF_ERR_NOT_SCHEDULED_THREAD if the given thread is not scheduled 
       under the FSF
   @retval FSF_ERR_INTERNAL_ERROR erroneous binding or malfunction of the FSF
       main scheduler
   @retval FSF_ERR_UNKNOWN_APPSCHEDULED_THREAD if the thread is attached to
       an application defined scheduler different than the fsf scheduler
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the thread is bound and its server
       is not valid
*/
int  
fsf_set_local_thread_sched_parameters
  (pthread_t                     thread,
   const fsf_sched_params_t  *sched_params);


/**
   \ingroup hiermodule

   This function stores the local scheduling parameters of the
   specified thread in the variable pointed to by sched_params

   @param [in] thread thread id
   @param [out] sched_params scheduling parameters

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if sched_params is NULL or the thread does
           not exist
   @retval FSF_ERR_NOT_SCHEDULED_THREAD if the given thread is not scheduled 
       under the FSF
*/
int  
fsf_get_local_thread_sched_parameters
  (pthread_t            thread,
   fsf_sched_params_t  *sched_params);


#endif // _FSF_HIERARCHICAL_H_
