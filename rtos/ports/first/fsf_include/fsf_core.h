
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

/**
   @file This file contains the functions and definitions for the core
   module.

   The file contains all fuinctions and data structures for the core
   module of the FSF.
 */

//fsf_core.h
//=================================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// Basic FSF(FIRST Scheduling Framework) contract management
//================================================================


#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>  

#include "fsf_configuration_parameters.h"
#include "fsf_opaque_types.h"
#include "fsf_basic_types.h"

#ifndef _FSF_CORE_H_
#define _FSF_CORE_H_


//////////////////////////////////////////////////////////////////////
//           INITIALIZATION SERVICES 
//////////////////////////////////////////////////////////////////////

/**
   \ingroup coremodule

   We cannot call any fsf functions before fsf_init. After calling
   fsf_init, the main will be executing in the background. Then, it
   can do the negotiations, create the threads and, if needed,
   activate them via some user-specified synchronization mechanism. It
   may also create a contract for itself. The second time this
   function is called it fails.

   @retval 0 if the system is initialized
   @retval FSF_ERR_SYSTEM_ALREADY_INITIALIZED if the function has already
           been called before
       
   @retval others It may also return any of the errors that may be
         returned by the underlying operating system primitives
         required to perform the FSF system start up
*/
int fsf_init();

/** 
   \ingroup coremodule

   This function converts an error code to an error message that is
   stored in the buffer starting at the location pointed to by
   message. The size of this buffer is specified by the size
   argument. If the error message is longer than size-1, it is
   truncated to that length. Regardless of whether the message is
   truncated or not, a final zero character that marks the end of the
   string is stored in the buffer.  The function fails if the error
   code passed does not correspond to any of the fsf error codes.

   @retval FSF_ERR_BAD_ARGUMENT  error is not a valid value
*/
int fsf_strerror (int error, char *message, size_t size);


/////////////////////////////////////////////////////////////
//                       CONTRACT PARAMETERS
/////////////////////////////////////////////////////////////

/** 
   \ingroup coremodule

    The operation receives a pointer to a contract parameters object
    and initializes it, setting it to the default values.
    The default values are:
    - budget min and max are set to 0;
    - period min and max are set to 0;
    - the workload is unbounded (FSF_INDETERMINATE);
    - the server deadline is equal to the period;
    - the budget and deadline overrun are not notified;
    - the granularity is set to "continuous" (FSF_CONTINUOUS);
    - the quality and importance are set to the default values;
    - the scheduling policy is FSF_NONE.

    @param     contract the pointer to the contract variable.
    @retval   FSF_ERR_BAD_ARGUMENT   contract is NULL
*/
int fsf_initialize_contract (fsf_contract_parameters_t *contract);

//  budget_min                => {0,0};
//  period_max                => {0,0};
//  budget_max                => {0,0};
//  period_min                => {0,0};
//  workload                  => DEFAULT_WORKLOAD;

//  d_equals_t                => DEFAULT_D_EQUALS_T; (false or true)
//  deadline                  => DEFAULT_DEADLINE;
//  budget_overrun_sig_notify => 0;  (signal number)
//  budget_overrun_sig_value  => {0, NULL};
//  deadline_miss_sig_notify  => 0;  (signal number)
//  deadline_miss_sig_value   => {0, NULL};
//
//  granularity               => DEFAULT_GRANULARITY;
//  utilization_set;          => size = 0
//  quality                   => DEFAULT_QUALITY; (0, range 0..2**32-1)
//  importance                => DEFAULT_IMPORTANCE; (1, range 1..5)
//
//  preemption_level          => 0; (range 1..2**32-1)
//  critical_sections;        => size = 0

//  sched_policy              => DEFAULT_SCHED_POLICY
//                              (FSF_NONE)
                                                    
//  network_id                => FSF_NULL_NETWORK_ID;
//                               (0)
//  granted_capacity_flag     => false;

/** 
   \ingroup coremodule

   The operation updates the specified contract parameters object by
   setting its budget, period, and workload to the specified input
   parameters. (Note: the workload is a basic parameter because
   bounded tasks are triggered by the scheduler (see the
   fsf_schedule_timed_job() operation), while indeterminate tasks are
   not; therefore, their programming model is quite different).

   @param contract          the pointer to the contract object
   @param [in] budget_min   the minimum budget for the contract
   @param [in] period_max   the maximum period for the contract
   @param [in] workload     the kind of workload (can be FSF_BOUNDED or 
                            FSF_INDETERMINATE)
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if any of the pointers is NULL
       or if only one of the timespec values is 0, and also if the workload
       is not a proper value (FSF_INDETERMINATE, FSF_BOUNDED or FSF_OVERHEAD)
*/
int
fsf_set_contract_basic_parameters
  (fsf_contract_parameters_t *contract,
   const struct timespec     *budget_min,
   const struct timespec     *period_max,
   fsf_workload_t            workload);

/**
   \ingroup coremodule

   This operation obtains from the specified contract parameters
   object its budget, period, and workload, and copies them to the
   places pointed to by the corresponding input parameters.

   @param [in] contract   the pointer to the contract object
   @param[out] budget_min pointer to the variable that will contain 
   the minimum budget
   @param[out] period_max pointer to the variable that will contain the 
   max_period
   @param[out] workload pointer to the variable that will contain the
   workload type

   @retval  FSF_ERR_BAD_ARGUMENT :  if contract is NULL
*/
int
fsf_get_contract_basic_parameters
  (const fsf_contract_parameters_t *contract,
   struct timespec  *budget_min,
   struct timespec  *period_max,
   fsf_workload_t   *workload);


/** 
   \ingroup coremodule

   The operation updates the specified contract parameters
   object, specifying the additional parameters requirements of
   a contract.

   @param  contract The pointer to the contract object
   
   @param [in] d_equals_t It is a boolean value, set to true (1) if the 
                 we want to specify a deadline different from the period 
		 for the contract. 
   @param [in] deadline If the previous parameter is set to true, 
                 this parameter should be set to NULL_DEADLINE. Otherwise, 
		 it contains the desired deadline value. 

   @param [in] budget_overrun_sig_notify contains the number of posix signal 
                 that must be raised if the budget of the server is overrun. 
		 If the value of this parameter is NULL_SIGNAL, no signal will 
		 be raised. 
   @param [in] budget_overrun_sig_value contains the value that will be 
                 passed to the signal "catcher" when the signal is raised. 
		 This parameters is not used if the budget_overrun_sig_notify 
		 parameters is set to NULL_SIGNAL.
   @param [in] deadline_miss_sig_notify contains the number of posix 
                 signal that must be raised if the deadline of the server 
		 is missed. If the value of this parameter is NULL_SIGNAL, 
		 no signal is raised. 
   @param [in] deadline_miss_sig_value contains the value that will be 
                 passed to the signal "catcher" when the signal is raised. 
		 This parameters is not used if the budget_overrun_sig_notify 
		 parameters is set to NULL_SIGNAL
		 
   @retval 0    if the operation is succesful
   @retval FSF_BAD_ARGUMENT if contract is NULL or  
       (d_equals_t is true and  deadline is not FSF_NULL_DEADLINE) or
       (budget_overrun_sig_notify is not a valid signal)  or
       (deadline_miss_sig_notify is not a valid signal)  or
       (d_equals_t is false but (deadline is FSF_NULL_DEADLINE or its value 
                                 is grater than the contracts maximum period))

   @see sigexplanation
*/
int
fsf_set_contract_timing_requirements
  (fsf_contract_parameters_t *contract,
   bool                   d_equals_t,
   const struct timespec *deadline,
   int                    budget_overrun_sig_notify,
   union sigval           budget_overrun_sig_value,
   int                    deadline_miss_sig_notify,
   union sigval           deadline_miss_sig_value);

/**
   \ingroup coremodule

   The operation obtains the corresponding input parameters from the
   specified contract parameters object. If d_equals_t is true, the
   deadline will not be updated.

   @retval FSF_ERR_BAD_ARGUMENT if contract is NULL 

   @see fsf_set_contract_timing_requirements
*/
int
fsf_get_contract_timing_requirements
  (const fsf_contract_parameters_t *contract,
   bool                    *d_equals_t,
   struct timespec         *deadline,
   int                     *budget_overrun_sig_notify,
   union sigval            *budget_overrun_sig_value,
   int                     *deadline_miss_sig_notify,
   union sigval            *deadline_miss_sig_value);

//////////////////////////////////////////////////////////////////
//                 SYNCHRONIZATION OBJECTS
//////////////////////////////////////////////////////////////////


/**
   \ingroup coremodule

   This operation creates and initializes a synchronization object
   variable managed by the scheduler, and returns a handle to it in
   the variable pointed to by synch_handle.
   
   @param[out] synch_handle pointer to the variable that will contain
                the handle to the newly created synchronization object

   @retval  0 if the operation is succesful
   @retval  FSF_ERR_BAD_ARGUMENT   if synch_handle is 0
   @retval  FSF_ERR_TOO_MANY_SYNCH_OBJS  if the number of synchronization 
             objects in the system has already exceeded the maximum
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD  if the calling thread is not
            scheduled under the FSF
   @retval  FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong 
             or not running
*/
int
fsf_create_synch_obj
    (fsf_synch_obj_handle_t *synch_handle);

/**
   \ingroup coremodule

   This function sends a notification to the synchronization object
   specified as parameter. If there is at least one server waiting on
   the synchronization object, the corresponding thread is unblocked, 
   and the server rules for budget recharging apply. 

   If more than one server is waiting, just one of them is woken.
   However, which one is woken is implementation dependent. 

   If no thread is waiting on the synchronization object, the
   notification is queued.

   @param [in] synch_handle the handle of the synchronization object to 
                  notify.

   @retval 0 if the operation is completed succesfully
   @retval FSF_ERR_BAD_ARGUMENT   if synch_handle is 0
   @retval FSF_ERR_INVALID_SYNCH_OBJ_HANDLE if the handle is not valid
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD  if the calling thread 
              is not scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or 
              not running
   @retval FSF_ERR_TOO_MANY_EVENTS_IN_SYNCH_OBJ  if the number of 
              events stored in the synchronization object reaches the 
              maximum defined in the configuration parameter header file

   @see fsf_schedule_triggered_job, fsf_timed_schedule_triggered_job
*/
int
fsf_signal_synch_obj
    (fsf_synch_obj_handle_t synch_handle);

/**
   \ingroup coremodule

   This operation destroys the synchronization object (created by a
   previous call to fsf_create_synch_obj) that is referenced by the
   synch_handle variable. After calling this operation, the
   synch_handle variable can not be used until it is initialized again
   by a call to fsf_create_synch_obj.

   @param synch_handle the handle to the synchronization object 
             to be destroyed

   @retval 0 if the operation is succesful
   @retval FSF_ERR_INVALID_SYNCH_OBJ_HANDLE is the handle is not valid
   @retval FSF_ERR_BAD_ARGUMENT   if synch_handle is 0
   @retval FSF_ERR_INVALID_SYNCH_OBJ_HANDLE if the handle is not valid
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or 
       not running

   @see fsf_create_synch_obj
*/
int
fsf_destroy_synch_obj
    (fsf_synch_obj_handle_t synch_handle);


////////////////////////////////////////////////////
//           SCHEDULING BOUNDED WORKLOADS
////////////////////////////////////////////////////

/**
   \ingroup coremodule

   This operation is invoked by threads associated with bounded
   workload servers to indicate that a job has been completed (and
   that the scheduler may reassign the unused capacity of the current
   job to other servers). It is also invoked when the first job of
   such threads has to be scheduled.

   As an effect, the system will make the current server's budget zero
   for the remainder of the server's period, and will not replenish
   the budget until the specified absolute time.  At that time, all
   pending budget replenishments (if any) are made effective. Once the
   server has a positive budget and the scheduler schedules the
   calling thread again, the call returns and at that time, except for
   those parameters equal to NULL pointers, the system reports the
   current period and budget for the current job, whether the deadline
   of the previous job was missed or not, and whether the budget of
   the previous job was overrun or not.

   In a system with hierarchical scheduling, since this call makes the
   budget zero, the other threads in the same server are not run. As
   mentioned above, only when the call finishes the budget may be
   replenished.

   @param [in] abs_time     absolute time at which the budget will be 
                            replenished

   @param [out] next_budget upon return of this function, the variable 
                            pointed by this function will be equal to 
			    the current server budget. If this parameter is 
			    set to NULL, no action is taken. 
   
   @param [out] next_period upon return of this function, the variable 
                            pointed by this function will be equal to 
			    the current server period. If this parameter is 
			    set to NULL, no action is taken.

   @param [out] was_deadline_missed upon return of this function, the
                            variable pointed by this function will be
                            equal to true if the previous server deadline 
			    was missed, to false otherwise. If this
                            parameter is set to NULL, no action is
                            taken.

   @param [out] was_budget_overran upon return of this function, the
                            variable pointed by this function will be
                            equal to true if the previous server budget was
                            overrun, to false otherwise. If this
                            parameter is set to NULL, no action is
                            taken.

   @retval 0 if the operation is succesful
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or 
              not running
   @retval FSF_ERR_INTERNAL_ERROR  erroneous binding or malfunction of the FSF
       main scheduler
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is 
       not scheduled under the FSF
   @retval FSF_ERR_NOT_BOUND if the calling thread does not have a valid 
       server bound to it
   @retval FSF_ERR_BAD_ARGUMENT  if abs_time is NULL
   @retval FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE  if the kind of workload of
       the server is not FSF_BOUNDED


   @sa fsf_schedule_triggered_job, fsf_timed_schedule_triggered_job
*/
int
fsf_schedule_timed_job
  (const struct timespec *abs_time,
   struct timespec       *next_budget,
   struct timespec       *next_period,
   bool                  *was_deadline_missed,
   bool                  *was_budget_overran);

/**
   \ingroup coremodule

   This operation is invoked by threads associated with bounded
   workload servers to indicate that a job has been completed (and
   that the scheduler may reassign the unused capacity of the current
   job to other servers). It is also invoked when the first job of
   such threads has to be scheduled. If the specified synchronization
   object has events queued, one of them is dequeued; otherwise the
   server will wait upon the specified synchronization object, the
   server's budget will be made zero for the remainder of the server's
   period, and the implementation will not replenish the budget until
   the specified synchronization object is signalled. 

   At that time, all pending budget replenishments (if any) are made
   effective. Once the server has a positive budget and the scheduler
   schedules the calling thread again, the call returns and at that
   time, except for those parameters equal to NULL pointers, the
   system reports the current period and budget for the current job,
   whether the deadline of the previous job was missed or not, and
   whether the budget of the previous job was overrun or not.

   In a system with hierarchical scheduling, since this call makes the
   budget zero, the other threads in the same server are not run. As
   mentioned above, only when the call finishes the budget may be
   replenished.

   @param [in]  synch_handle   handle of the synchronization object

   @param [out] next_budget    upon return of this function, the variable
                               pointed by this function will be equal
                               to the current server budget. If this
                               parameter is set to NULL, no action is
                               taken.
   @param [out] next_period    upon return of this function, the variable 
                               pointed by this function will be equal to 
			       the current server period. If this parameter is 
			       set to NULL, no action is taken.

   @param [out] was_deadline_missed upon return of this function, the
                            variable pointed by this function will be
                            equal to true if the previous server deadline 
			    was missed, to false otherwise. If this
                            parameter is set to NULL, no action is
                            taken.

   @param [out] was_budget_overran upon return of this function, the
                            variable pointed by this function will be
                            equal to true if the previous server budget was
                            overrun, to false otherwise. If this
                            parameter is set to NULL, no action is
                            taken.

   @retval 0 if the operation is succesful
   @retval  FSF_ERR_INVALID_SYNCH_OBJ_HANDLE if the synch_handle is not valid
   @retval  FSF_ERR_BAD_ARGUMENT   if synch_handle is 0
   @retval  FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or not 
            running
   @retval  FSF_ERR_INTERNAL_ERROR  erroneous binding or malfunction of 
            the FSF main scheduler
   @retval  FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread 
            is not scheduled under the FSF
   @retval  FSF_ERR_NOT_BOUND if the calling thread does not have 
            a valid server bound to it
   @retval  FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE: if the kind of workload
            of the server is not FSF_BOUNDED

   @sa fsf_schedule_triggered_job, fsf_schedule_timed_job

*/
int
fsf_schedule_triggered_job
  (fsf_synch_obj_handle_t  synch_handle,
   struct timespec         *next_budget,
   struct timespec         *next_period,
   bool                    *was_deadline_missed,
   bool                    *was_budget_overran);


/**
   \ingroup coremodule

   This call is the same as fsf_schedule_triggered_job, but with an
   absolute timeout. The timed_out argument, indicates whether the
   function returned because of a timeout or not

   @retval FSF_ERR_INVALID_SYNCH_OBJ_HANDLE if the synch_handle is not valid
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
       not running
   @retval FSF_ERR_INTERNAL_ERROR  erroneous binding or malfunction 
       of the FSF main scheduler
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is 
       not scheduled under the FSF
   @retval FSF_ERR_NOT_BOUND  if the calling thread does not have a valid 
       server bound to it
   @retval FSF_ERR_BAD_ARGUMENT   if synch_handle is 0, or the abs_timeout 
       argument is NULL, or its value is in the past
   @retval FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE: if the kind of workload
            of the server is not FSF_BOUNDED

   @see fsf_schedule_triggered_job
*/
int
fsf_timed_schedule_triggered_job
  (fsf_synch_obj_handle_t  synch_handle,
   const struct timespec   *abs_timeout,
   bool                    *timed_out,
   struct timespec         *next_budget,
   struct timespec         *next_period,
   bool                    *was_deadline_missed,
   bool                    *was_budget_overran);


///////////////////////////////////////////////////////////////////
//                 CONTRACT NEGOTIATION OPERATIONS
///////////////////////////////////////////////////////////////////

/**
   \ingroup coremodule

   The operation negotiates a contract for a new server. If the
   on-line admission test is enabled it determines whether the
   contract can be admitted or not based on the current contracts
   established in the system. Then it creates the server and
   recalculates all necessary parameters for the contracts already
   present in the system. 

   This is a potentially blocking operation; it returns when the
   system has either rejected the contract, or admitted it and made it
   effective. It returns zero and places the server identification
   number in the location pointed to by the server input parameter if
   accepted, or an error if rejected.  No thread is bound to the newly
   created server, which will be idle until a thread is bound to
   it. This operation can only be executed by threads that are already
   bound to an active server and therefore are being scheduled by the
   fsf scheduler.

   @param [in] contract pointer to the contract
   @param [out] server server id

   @retval 0 if the call is succesful
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or not 
                                            running
   @retval FSF_ERR_INTERNAL_ERROR  erroneous binding or malfunction of the FSF
                                   main scheduler
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD  
                                   if the calling thread is not scheduled 
                                   under the FSF
   @retval FSF_ERR_BAD_ARGUMENT    if the contract or server arguments 
                                   are NULL
   @retval FSF_ERR_TOO_MANY_SERVERS if there is no space for more servers 
                                    (the maximum number of them is already
                                    reached)
   @retval FSF_ERR_CONTRACT_REJECTED  if the contract is rejected
*/
int
fsf_negotiate_contract
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t      *server);

/**
   \ingroup coremodule

   This operation negotiates a contract for a new server, creates a
   thread and binds it to the server. If the contract is accepted, the
   operation creates a thread with the arguments thread, attr,
   thread_code and arg as they are defined for the pthread_create()
   POSIX function call, and attaches it to the fsf scheduler. Then, it
   binds the created thread to the new server. It returns zero and
   puts the server identification number in the location pointed to by
   the server input parameter. 

   The attr parameter is overwritten as necessary to introduce the
   adequate scheduling attributes, according to the information given
   in the contract. 

   The server is created with the FSF_NONE scheduling policy, which
   means no hierarchical scheduling, and only one thread per server,
   except for the case of background tasks.

   If the contract is rejected, the thread is not created and the
   corresponding error is returned.

   @param [in] contract pointer to the contract
   @param [out] server server id
   @param [out] thread thread id 
   @param [in] attr threads attributes 
   @param [in] thread_code  pointer to the function that implements 
               the thread
   @param [in] arg  arguments for the thread

   @retval  FSF_ERR_BAD_ARGUMENT  if the contract or server arguments are NULL
   @retval  FSF_ERR_CONTRACT_REJECTED  if the contract is rejected
   @retval  FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE if the kind of workload
            in the contract is FSF_OVERHEAD
            
   @retval  others it may also return all the errors that may be returned 
            by the pthread_create()POSIX function call

*/
int
fsf_negotiate_contract_for_new_thread
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t      *server,
   pthread_t            *thread,
   pthread_attr_t       *attr,
   fsf_thread_code_t     thread_code,
   void                 *arg);

/**
   \ingroup coremodule

   This operation negotiates a contract for a new server, and binds
   the calling thread to it. If the contract is accepted it returns
   zero and copies the server identification number in the location
   pointed to by the server input parameter.  If it is rejected, an
   error is returned.

   The server is created with the FSF_NONE scheduling policy, which
   means no hierarchical scheduling, and only one thread per server,
   except for the case of background tasks.

   <i> Implementation dependent issue for Marte OS: to allow the usage of
   application defined schedulers, the calling thread must not have
   the SCHED_APP scheduling policy and at the same time be attached to
   an application scheduler different than the fsf scheduler; in such
   case, an error is returned. After a successful call the calling
   thread will have the SCHED_APP scheduling policy and will be
   attached to the fsf scheduler.</i>

   @param [in] contract pointer to the contract
   @param [out] server id
   
   @retval 0 if the contract negotation is succesful
   @retval  FSF_ERR_UNKNOWN_APPSCHEDULED_THREAD  if the thread is attached to
       an application defined scheduler different than the fsf scheduler
   @retval FSF_ERR_BAD_ARGUMENT  if the contract or server arguments 
       are NULL
   @retval FSF_ERR_CONTRACT_REJECTED  if the contract is rejected.
   @retval FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE if the kind of workload
            in the contract is FSF_OVERHEAD
*/
int
fsf_negotiate_contract_for_myself
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t      *server);


/**
   \ingroup coremodule

   This operation associates a thread with a server, which means that
   it starts consuming the server's budget and is executed according
   to the contract established for that server. If the thread is
   already bound to another server, and error is returned.

   It fails if the server's policy is different than FSF_NONE, or if
   there is already a thread bound to this server

   <i> Implementation dependent issue for MARTE OS: In order to allow the usage of
   application defined schedulers, the given thread must not have the
   scheduling policy SCHED_APP and at the same time be attached to an
   application scheduler different than the fsf scheduler. </i>

   @param [in] server id
   @param [in] thread id

   @retval FSF_ERR_INTERNAL_ERROR  erroneous binding or malfunction 
       of the FSF main scheduler
   @retval FSF_ERR_UNKNOWN_APPSCHEDULED_THREAD if the thread is attached to
       an application defined scheduler different than the fsf scheduler
   @retval FSF_ERR_BAD_ARGUMENT if the server value does not comply with the
       expected format or valid range or the given thread does not exist
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the referenced server 
       is not valid
   @retval FSF_ERR_ALREADY_BOUND if the given server has a thread 
       already bound
   @retval FSF_ERR_SERVER_WORKLOAD_NOT_COMPATIBLE if the kind of workload
       in the contract is FSF_OVERHEAD

*/
int
fsf_bind_thread_to_server
  (fsf_server_id_t server,
   pthread_t       thread);


/**
   \ingroup coremodule

   This operation unbinds a thread from a server.  Since threads with
   no server associated are not allowed to execute, they remain in a
   dormant state until they are either eliminated or bound again.

   If the thread is inside a critical section the effects of this call
   are deferred until the critical section is ended.

   Implementation dependent issue: in the implementation with an
   application scheduler, the thread is still attached to the fsf
   scheduler, but suspended.

   @param [in] thread thread id

   @retval 0 if the operation is succesful
   @retval FSF_ERR_INTERNAL_ERROR  erroneous binding or malfunction of the FSF
       main scheduler
   @retval FSF_ERR_BAD_ARGUMENT  if the given thread does not exist
   @retval FSF_ERR_NOT_SCHEDULED_THREAD  if the given thread is not scheduled 
       under the FSF
   @retval FSF_ERR_UNKNOWN_APPSCHEDULED_THREAD if the thread is attached to
       an application defined scheduler different than the fsf scheduler
   @retval FSF_ERR_NOT_BOUND if the given thread does not have a valid 
       server bound to it
*/
int
fsf_unbind_thread_from_server (pthread_t thread);

/**
   \ingroup coremodule

   This operation stores the Id of the server associated with the
   specified thread in the variable pointed to by server. It returns
   an error if the thread does not exist, it is not under the control
   of the scheduling framework, or is not bound.

   @param [in] thread thread id
   @param [out] server server 

   @retval 0 if the operation is succesful
   @return FSF_ERR_NOT_SCHEDULED_THREAD if the given thread is not scheduled 
       under the FSF
   @return FSF_ERR_NOT_BOUND if the given thread does not have a valid 
       server bound to it
   @return FSF_ERR_BAD_ARGUMENT if the given thread does not exist or the
       server argument is NULL
*/
int
fsf_get_server
  (pthread_t       thread,
   fsf_server_id_t *server);

/**
   This operation stores the contract parameters currently associated
   with the specified server in the variable pointed to by
   contract. It returns an error if the server id is incorrect.

   @param [in] server server id
   @param [out] contract pointer to the contract structure

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT  if the contract argument is 
         NULL or the value of the server argument is not in range
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or 
       not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_get_contract
   (fsf_server_id_t server,
    fsf_contract_parameters_t *contract);

/**
   \ingroup coremodule

   The operation eliminates the specified server
   and recalculates all necessary parameters for the contracts
   remaining in the system. This is a potentially blocking operation;
   it returns when the system has made the changes effective.

   @param [in] server server id

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT   if the value of server is not in range
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD  if the calling thread is not
           scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY  the scheduler is wrong or not 
           running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER  if the server of the calling thread 
           has been cancelled or it is not valid
*/
int
fsf_cancel_contract (fsf_server_id_t server);


/**
   \ingroup coremodule

   The operation renegotiates a contract for an existing server. If
   the on-line admission test is enabled it determines whether the
   contract can be admitted or not based on the current contracts
   established in the system. If it cannot be admitted, the old
   contract remains in effect and an error is returned. If it can be
   admitted, it recalculates all necessary parameters for the
   contracts already present in the system anr returns zero. This is a
   potentially blocking operation; it returns when the system has
   either rejected the new contract, or admitted it and made it
   effective.

   @param [in]  new_contract a pointer to the new contract
   @param [in]  server server id

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT  if the new_contract argument is NULL or the  
       value of the server argument is not in range
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
       running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
   @retval FSF_ERR_REJECTED_CONTRACT if the renegotiation fails
*/
int
fsf_renegotiate_contract
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t server);

/**
   \ingroup coremodule

   The operation enqueues a renegotiate operation for an existing
   server, and returns immediately. The renegotiate operation is
   performed asynchronously and the calling thread may continue
   executing normally. Of course, wheter the operation is performed
   immediately or not depends on the relative priority of the service
   thread and the calling thread, on the scheduler used, etc.

   When the renegotiation is completed, if the on-line admission test
   is enabled it determines whether the contract can be admitted or
   not based on the current contracts established in the system. If it
   cannot be admitted, the old contract remains in effect. If it can
   be admitted, it recalculates all necessary parameters for the
   contracts already present in the system. When the operation is
   completed, notification is made to the caller, if requested, via a
   signal. The status of the operation (in progress, admitted,
   rejected) can be checked with the get_renegotiation_status
   operation.  The argument sig_notify can be NULL_SIGNAL (no
   notification), or any POSIX signal; and in this case sig_value is
   to be sent with the signal.

   @param [in] new_contract pointer to the new contract to be negotiated
   @param [in] server  server id
   @param [in] sig_notify NULL (no signal) or any POSIX signal
   @param [in] sig_value a sigval structure that contains values to be 
               passed to the signal handler. Valid only if sig_notify 
               is different from NULL.

   @retval 0 if the call is succesful
   @retval FSF_ERR_BAD_ARGUMENT  if the new_contract argument is NULL, the  
       value of the server argument is not in range or sig_notify is 
       neither NULL nor a valid POSIX signal
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
       running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid

*/
int
fsf_request_contract_renegotiation
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t                  server,
   int                              sig_notify,
   union sigval                     sig_value);

/**
   \ingroup coremodule

   The operation reports on the status of the last renegotiation
   operation enqueued for the specified server. It is callable even
   after notification of the completion of such operation, if
   requested.

   @param [in] server server id
   @param [out] renegotiation_status the status of the renegotiation;

   @retval 0 if succesful completion;
   @retval FSF_ERR_BAD_ARGUMENT  if the renegotiation_status argument is 
       NULL or the value of the server argument is not in range
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
       running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_get_renegotiation_status
  (fsf_server_id_t server,
   fsf_renegotiation_status_t *renegotiation_status);


////////////////////////////////////////////////////////////////////////
//           CHANGE OF MODE: GROUPS OF CONTRACTS 
////////////////////////////////////////////////////////////////////////


/**
   \ingroup coremodule

   This operation analizes the schedulability of the context that
   results from negotiating the contracts specified in the
   contracts_up list and cacelling the contracts referenced by the
   servers_down list. If the overall negotiation is successful, a new
   server will be created for each of the elements of the contracts_up
   group, the servers in servers_down will be cancelled, the list of
   new server ids will be returned in the variable pointed to by
   servers_up, and the variable pointed to by accepted will be made
   true. Otherwise, this variable will be made false, and no other
   effect will take place. The function returns the corresponding
   error code if any of the contracts is not correct or any of the
   server is is not valid.

   @param [in] contracts_up list of contracts to negotiate
   @param [in] servers_down list of contracts to be canceled
   @param [out] servers_up list of server ids that have been created, they are
      given in the same order as the contract_up list of contracts;
   @param [out] accepted if the operation is succesful;

   @retval 0 if succesful completion;
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
           not running
   @retval FSF_ERR_INTERNAL_ERROR erroneous binding or malfunction of the FSF
       main scheduler
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is 
           not scheduled under the FSF
   @retval FSF_ERR_BAD_ARGUMENT if any of the servers_up or accepted arguments
       is NULL, if the contracts_up and servers_down arguments are both NULL,
       or any of them has erroneous size or its elements are NULL or not in the
       valid range respectively
*/
int
fsf_negotiate_group
   (const fsf_contracts_group_t *contracts_up,
    const fsf_servers_group_t   *servers_down,
    fsf_servers_group_t         *servers_up,
    bool                        *accepted);


////////////////////////////////////////////////////
//           OBTAINING INFORMATION FROM THE SCHEDULER
////////////////////////////////////////////////////


/**
   \ingroup coremodule

   Returns true if the system is configured with the on-line admission
   test enabled, or false otherwise.
*/
bool
fsf_is_admission_test_enabled();

/**
   \ingroup coremodule

   This function stores the current execution time spent by the
   threads bound to the specified server in the variable pointed to by
   cpu_time.

   @param [in] server server id
   @param [out] cpu_time pointer to a timespec structure that will contain 
         the cpu time consumed by the threads that are bound to the 
         specific server, since its creation. 

   @retval 0 if succesful completion
   @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument is 
       not in range or cpu_time is NULL
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
       not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_get_cpu_time
   (fsf_server_id_t server,
    struct timespec *cpu_time);

/**
   \ingroup coremodule

   This function stores in the variable pointed to by budget the
   remaining execution-time budget associated with the specified
   server.

   @param [in] server server id
   @param [out] budget pointer to a timespec structure that will 
     contain the remaining budget

   @retval 0 if succesful completion
   @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument is 
       not in range or budget is NULL
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
       not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_get_remaining_budget
   (fsf_server_id_t server,
    struct timespec *budget);


/**
   \ingroup coremodule

   This function stores in the variables pointed to by budget and
   period, the execution-time budget and the period respectively
   associated with the specified server. If any of these pointers is
   NULL, the corresponding information is not stored.

   @param [in] server server id
   @param [out] budget budget available for the current server instance
   @param [out] period period available for the current server instance

   @retval 0 if succesful completion
   @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument is 
       not in range, budget is NULL or period is NULL
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
     running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_get_budget_and_period
   (fsf_server_id_t server,
    struct timespec *budget,
    struct timespec *period);

/////////////////////////////////////////////////////////////////////
//           SERVICE THREAD TUNING
/////////////////////////////////////////////////////////////////////


/**
   \ingroup coremodule

   This function allows the application to change the period and
   budget of the service thread that makes the
   negotiations. Increasing the utilization of this thread makes the
   negotiations faster, but introduces additional load in the system
   that may decrease the bandwidth available for the servers. For this
   call, the system will make a schedulability analysis to determine
   if the new situation is acceptable or not. This is reported back in
   the variable pointed to by accepted. If the new service thread data
   is accepted, the system will reassign budgets and periods to the
   servers according to the new bandwidth available, in the same way
   as it does for a regular contract negotiation.
   
   When its budget is exhausted, the service thread may run in the
   background.

   The service thread starts with a default budget and period that are
   configurable.

   <i> Implementation dependency for MARTE OS: in the fixed priority
   implementation of fsf, the default priority is lower than the
   priority of any server, but higher than the background. According
   to the implementation-dependent module the priority is adjustable
   by means of a function that changes its preemption level.</i>

   
   @param [in] budget budget for the service thread
   @param [in] period for the service thread
   @param [out] accepted true is the change has been accepted


   @retval 0 is the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if any of the pointer arguments is NULL or
       the budget value is greater than the period value
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
       not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_set_service_thread_data
   (const struct timespec *budget,
    const struct timespec *period,
    bool                  *accepted);

/**
   \ingroup coremodule

   This function returns in the variables pointed by budget and
   period, respectively, the current budget and period of the service
   thread.
   
   @param [out] budget   current budget of the service thread
   @param [out] period   current period of the service thread

   @retval FSF_ERR_BAD_ARGUMENT if any of the pointer arguments is NULL
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong 
       or not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int
fsf_get_service_thread_data
   (struct timespec *budget,
    struct timespec *period);
 
#endif // _FSF_CORE_H_
