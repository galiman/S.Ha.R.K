
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

//====================================================================================
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

#include <time.h>
#include <sys/boolean.h>
#include <sys/types.h>

#include "fsf_configuration_parameters.h"
#include "fsf_opaque_types.h"

#include "edfstar.h"
#include "rmstar.h"
#include "posixstar.h"
#include "nonestar.h"

#ifndef _FSF_CONTRACT_H_
#define _FSF_CONTRACT_H_



/* S.Ha.R.K. Init */
int FSF_register_module(int server_level, bandwidth_t max_bw);

//////////////////////////////////////////////////////////////////
//                     BASIC TYPES AND CONSTANTS
//////////////////////////////////////////////////////////////////

typedef enum {FSF_BOUNDED, FSF_INDETERMINATE} fsf_workload_t;

typedef enum {FSF_CONTINUOUS, FSF_DISCRETE} fsf_granularity_t;
  
typedef struct {
  struct timespec    budget;    // Execution time
  struct timespec    period;    // Period
} fsf_utilization_value_t;

typedef struct {
  int                         size; // = 0
  fsf_utilization_value_t     unit[FSF_MAX_N_UTILIZATION_VALUES];
} fsf_utilization_set_t;

typedef unsigned long         fsf_preemption_level_t; // range 1..2**32-1

typedef struct {
  struct timespec          wcet;      // Execution time
  fsf_preemption_level_t   plevel;    // Preemption_Level, range 1..2**32-1
} fsf_critical_section_data_t;


// Definition of basic types

typedef unsigned int  fsf_shared_op_id_t;
typedef unsigned int  fsf_shared_obj_id_t;

// Operations
// The first field contains the id of the related object
// the second field contains an identifier of the operation
//   this identifier is unique for the object but needs not to be unique
//   for the system. In other words, there can be two operations with the 
//   same identifier belonging to different shared objects: they are 
//   effectively two distinct operations; two different operations on the 
//   same object must have different identifiers

typedef struct {
  fsf_shared_obj_id_t      obj_id;    // Object identification
  fsf_shared_op_id_t       op_id;     // Operation Identification
  struct timespec          wcet;      // Execution time
} fsf_shared_operation_t;

// Shared object
// the first field is the number of distinct operations on the 
//    shared object. It is initialized to 0.
// the second field is the id of the object, must be unique in the system.
// the third field is a list of shared operations.




typedef struct {
  int                          size; // = 0
  fsf_critical_section_data_t  section[FSF_MAX_N_CRITICAL_SECTIONS];
} fsf_critical_sections_t;

typedef struct {
  int                      size;      // = 0
  fsf_shared_obj_id_t      obj_id;    // object id
  fsf_shared_operation_t   shared_op[FSF_MAX_SHARED_OPERATION];
} fsf_shared_object_t;

typedef struct {
  int size; // =0
  fsf_shared_operation_t       operation[FSF_MAX_SHARED_OPERATION];
} fsf_shared_operations_t;

typedef int fsf_scheduler_id_t;

#define FSF_SCHEDULER_POSIX             0
#define FSF_SCHEDULER_EDF               1
#define FSF_SCHEDULER_RM                2
#define FSF_SCHEDULER_NONE              3

// Constants for assigning default values
#define FSF_DEFAULT_WORKLOAD            FSF_INDETERMINATE
#define FSF_DEFAULT_GRANULARITY         FSF_CONTINUOUS
#define FSF_DEFAULT_QUALITY             0
#define FSF_DEFAULT_IMPORTANCE          1
#define FSF_DEFAULT_D_EQUALS_T          TRUE
#define FSF_DEFAULT_DEADLINE            {0,0}
#define FSF_DEFAULT_SCHEDULER           FSF_SCHEDULER_NONE

// Constants for omitting the assignment of values to specific arguments
// in calls to initialization functions
#define FSF_NULL_CRITICAL_SECTIONS      (fsf_critical_sections_t *)NULL
#define FSF_NULL_UTILIZATION_SET        (fsf_utilization_set_t *)NULL
#define FSF_NULL_DEADLINE               (struct timespec *)NULL
#define FSF_NULL_SIGNAL                 0


// Error codes
#define FSF_ERR_NOT_INITIALIZED                 2003001
#define FSF_ERR_TOO_MANY_TASKS                  2003002
#define FSF_ERR_ALREADY_INITIALIZED             2003003
#define FSF_ERR_BAD_ARGUMENT                    2003004
#define FSF_ERR_INVALID_SYNCH_OBJECT_HANDLE     2003005
#define FSF_ERR_NO_RENEGOTIATION_REQUESTED      2003006
#define FSF_ERR_CONTRACT_REJECTED               2003007
#define FSF_ERR_TOO_MANY_SERVERS	        2003008
#define FSF_ERR_CREATE_THREAD                   2003009
#define FSF_ERR_SERVER_USED                     2003010
#define FSF_ERR_INVALID_SERVER                  2003011
#define FSF_ERR_CREATE_SERVER                   2003012

//////////////////////////////////////////////////////////////
//                       CONTRACT PARAMETERS
//////////////////////////////////////////////////////////////

// Contract parameters type; it is an opaque type
typedef FSF_CONTRACT_PARAMETERS_T_OPAQUE fsf_contract_parameters_t;

int
fsf_initialize_contract(fsf_contract_parameters_t *contract);

//Description: The operation receives a pointer to a contract parameters
//object and initializes it, setting it to the default values.
//  budget_min                  => {0,0};                              
//  period_max                  => {0,0};                              
//  budget_max                  => {0,0};                              
//  period_min                  => {0,0};                              
//  workload                    => DEFAULT_WORKLOAD;                   
                                                         
//  d_equals_t                  => DEFAULT_D_EQUALS_T; (false or true)
//  deadline                    => DEFAULT_DEADLINE;                     
//  budget_overrun_sig_notify   => 0;                  (signal number)
//  budget_overrun_sig_value    => {0, NULL};
//  deadline_miss_sig_notify    => 0;                  (signal number)
//  deadline_miss_sig_value     => {0, NULL};
//                                                         
//  granularity                 => DEFAULT_GRANULARITY;               
//  utilization_set;            => size = 0                         
//  quality                     => DEFAULT_QUALITY;     (range 0..100)
//  importance                  => DEFAULT_IMPORTANCE;    (range 1..5)
//                                                         
//  preemption_level            => 0;               (range 1..2**32-1)
//  critical_sections;          => size = 0                         
 

int
fsf_set_contract_basic_parameters 
  (fsf_contract_parameters_t *contract,
   const struct timespec  *budget_min,
   const struct timespec  *period_max,
   const struct timespec  *budget_max,
   const struct timespec  *period_min,
   fsf_workload_t          workload);

//Description: The operation updates the specified contract parameters
//object by setting its budget, period, and workload to the specified
//input parameters. (Note: the workload is a basic parameter because
//bounded tasks are triggered by the scheduler (see the Timed Schedule
//Next Job operation, later), while indeterminate tasks are not;
//therefore, their programming model is quite different).

int
fsf_get_contract_basic_parameters 
  (const fsf_contract_parameters_t *contract,
   struct timespec  *budget_min,
   struct timespec  *period_max,
   struct timespec  *budget_max,
   struct timespec  *period_min,
   fsf_workload_t   *workload);

//Description: This operation obtains from the specified contract parameters
//object its budget, period, and workload, and copies them to the places
//pointed to by the corresponding input parameters.

int
fsf_set_contract_timing_requirements 
  (fsf_contract_parameters_t *contract,
   bool                   d_equals_t,
   const struct timespec *deadline,
   int                    budget_overrun_sig_notify,
   union sigval           budget_overrun_sig_value,
   int                    deadline_miss_sig_notify,
   union sigval           deadline_miss_sig_value);

//Description: The operation updates the specified contract parameters
//object. d_equals_t is used as a boolean, deadline must be
//NULL_DEADLINE if d_equals_t is true, budget_overrun_sig_notify or
//deadline_miss_sig_notify may be NULL_SIGNAL (no notification) or any
//posix signal. budget_overrun_sig_value and deadline_miss_sig_value
//are the values to be delivered with the signal.

int
fsf_get_contract_timing_requirements 
  (const fsf_contract_parameters_t *contract,
   bool                            *d_equals_t,
   struct timespec                 *deadline,
   int                             *budget_overrun_sig_notify,
   union sigval                    *budget_overrun_sig_value,
   int                             *deadline_miss_sig_notify,
   union sigval                    *deadline_miss_sig_value);

//Description: The operation obtains the corresponding input
//parameters from the specified contract parameters object. If
//d_equals_t is true, the deadline will not be updated.

int  
fsf_set_contract_reclamation_parameters 
  (fsf_contract_parameters_t *contract,
   fsf_granularity_t            granularity,
   const fsf_utilization_set_t *utilization_set,
   int                          quality,
   int                          importance);

//Description: The operation updates the specified contract parameters
//object by setting its granularity, utilization set, quality, and
//importance to the specified input parameters.

int  
fsf_get_contract_reclamation_parameters 
  (const fsf_contract_parameters_t *contract,
   fsf_granularity_t               *granularity,
   fsf_utilization_set_t           *utilization_set,
   int                             *quality,
   int                             *importance);

//Description: The operation obtains from the specified contract parameters
//object its granularity, utilization set, quality, and importance. Then
//copies them to the places pointed to by the specified input parameters.
//Only the utilization_values of the utilization_set that are in use, are
//copied (according to its size field). 


/* OLD VERSION 
int
fsf_set_contract_synchronization_parameters 
  (fsf_contract_parameters_t     *contract,
   fsf_preemption_level_t         preemption_level,
   const fsf_critical_sections_t *critical_sections);

//Description: The operation updates the specified contract parameters
//object by setting its preemption level and critical sections to the
//specified input parameters.

*/

int
fsf_get_contract_synchronization_parameters 
  (const fsf_contract_parameters_t *contract,
   fsf_preemption_level_t          *preemption_level,
   fsf_critical_sections_t         *critical_sections);

//Description: The operation obtains from the specified contract
//parameters object its preemption level and critical sections, and
//copies them to the places pointed to by the specified input
//parameters.  Only those critical_section_data records that are in use
//in the critical_sections structure are copied (according to its size
//field).

int
fsf_set_local_scheduler_parameter
  (fsf_contract_parameters_t *contract,
   fsf_scheduler_id_t local_scheduler_id);

//Description: Set the local scheduler

int
fsf_get_local_scheduler_parameter
  (const fsf_contract_parameters_t *contract,
   fsf_scheduler_id_t *local_scheduler_id);

//Description: Get the local scheduler

//////////////////////////////////////////////////////////////
//                 SYNCHRONIZATION OBJECTS
//////////////////////////////////////////////////////////////

//An abstract synchronization object is defined by the application.
//This object can be used by an application to wait for an event to
//arrive by invoking the Event Triggered Schedule Next Job operation.
//It can also be used to signal the event either causing a waiting
//server to wake up, or the event to be queued if no server is waiting
//for it.  It is defined by the following opaque type and has the
//following operations:

typedef FSF_SYNCH_OBJECT_HANDLE_T_OPAQUE fsf_synch_object_handle_t;
  
int
fsf_create_synchobject(fsf_synch_object_handle_t *synch_handle);

//Description: This operation creates and initializes a
//synchronization object variable managed by the scheduler, and
//returns a handle to it in the variable pointed to by synch_handle.
  
int
fsf_signal_synchobject(fsf_synch_object_handle_t *synch_handle);

//Description: If one or more servers are waiting upon the specified
//synchronization object one of them is awakened; if not, the event is
//queued at the synchronization object.

int
fsf_destroy_synchobject(fsf_synch_object_handle_t *synch_handle);

//This operation destroys the synchronization object (created by a
//previous call to fsf_create_synchobject) that is referenced by the
//synch_handle variable. After calling this operation, the
//synch_handle variable can not be used until it is initialized again
//by a call to fsf_create_synchobject.


///////////////////////////////////////////////////////////////
//                 CONTRACT NEGOCIATION OPERATIONS
///////////////////////////////////////////////////////////////

// Server Id type, that identifies a server created to manage a 
// given contract

typedef int      fsf_server_id_t;

                                                                                
typedef struct {
        fsf_server_id_t  server;
        TIME             actual_period;
        TIME             actual_budget;
	int              Qs;    // quality of service
        int              Is;    // importance of service
	bandwidth_t      U;     // actual bandwidth
        bandwidth_t      Umin;  // min bandwidth
	bandwidth_t      Umax;  // max bandwidth
	TIME             Cmin;  
	TIME             Tmin;
	TIME	         Tmax;
	TIME		 deadline;
	bool		 d_equals_t;
} server_elem;

int recalculate_contract(bandwidth_t U); 


// The following type references a function that may become 
// a thread's code

typedef void * (*fsf_thread_code_t) (void *); 

// Negotiate contract functions: The following functions are used to
// create servers for a contract parameters specification and also to
// assign one or more threads to a server (Note: the current
// implementation only supports one thread per server; this limitation
// will be removed in the next phase of the project)

// The first time that any of these operations is called, it creates
// all the internal management structures that are necessary for the
// FIRST Scheduling Framework to operate properly.

int
fsf_negotiate_contract 
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t      *server);

//Description: The operation negotiates a contract for a new
//server. If the on-line admission test is enabled it determines
//whether the contract can be admitted or not based on the current
//contracts established in the system. Then it creates the server and
//recalculates all necessary parameters for the contracts already
//present in the system. This is a potentially blocking operation; it
//returns when the system has either rejected the contract, or
//admitted it and made it effective. It returns zero and places the
//server identification number in the location pointed to by the
//server input parameter if accepted, or an error if rejected.  No
//thread is bound to the newly created server, which will be idle
//until a thread is bound to it. This operation can only be executed
//by threads that are already bound to an active server and therefore
//are being scheduled by the fsf scheduler.

int
fsf_create_thread
  (fsf_server_id_t    server,
   pthread_t         *thread,
   pthread_attr_t    *attr,
   fsf_thread_code_t  thread_code,
   void              *arg,
   void              *local_scheduler_arg);

//Description: This operation creates a new thread inside a specific
//server. The local_scheduler_arg parameter is used to pass specific
//parameters to local scheduler. These parameters are application
//depented.

int
fsf_get_server 
  (fsf_server_id_t *server,
   pthread_t       thread);

//Description: This operation returns the server associated with a
//thread. It returns an error if the thread does not exist, it is not 
//under the control of the scheduling framework, or is not bound.

int
fsf_cancel_contract (fsf_server_id_t *server);

//Description: The operation eliminates the specified server and
//recalculates all necessary parameters for the contracts remaining in 
//the system. This is a potentially blocking operation; it returns when 
//the system has made the changes effective.

int
fsf_renegotiate_contract 
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t server);

//Description: The operation renegotiates a contract for an existing
//server. If the on-line admission test is enabled it determines
//whether the contract can be admitted or not based on the current
//contracts established in the system. If it cannot be admitted, the
//old contract remains in effect and an error is returned. If it can
//be admitted, it recalculates all necessary parameters for the
//contracts already present in the system anr returns zero. This is a
//potentially blocking operation; it returns when the system has
//either rejected the new contract, or admitted it and made it
//effective.

int
fsf_request_contract_renegotiation 
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t                  server,
   int                              sig_notify,
   union sigval                     sig_value);

//Description: The operation enqueues a renegotiate operation for an
//existing server, and returns immediately. The renegotiate operation
//is performed asynchronously, as soon as it is practical; meanwhile
//the system operation will continue normally. When the renegotiation
//is made, if the on-line admission test is enabled it determines
//whether the contract can be admitted or not based on the current
//contracts established in the system. If it cannot be admitted, the
//old contract remains in effect. If it can be admitted, it
//recalculates all necessary parameters for the contracts already
//present in the system. When the operation is completed, notification
//is made to the caller, if requested, via a signal. The status of the
//operation (in progress, admitted, rejected) can be checked with the
//get_renegotiation_status operation.  The argument sig_notify can be
//NULL_SIGNAL (no notification), or any posix signal; and in this case
//sig_value is to be sent with the signal.

typedef enum {FSF_IN_PROGRESS,
              FSF_REJECTED,
              FSF_ADMITTED} fsf_renegotiation_status_t;

int
fsf_get_renegotiation_status 
  (fsf_server_id_t server,
   fsf_renegotiation_status_t *renegotiation_status);

//Description: The operation reports on the status of the last
//renegotiation operation enqueued for the specified server. It is 
//callable even after notification of the completion of such operation,
//if requested.

int
fsf_request_change_quality_and_importance 
  (fsf_server_id_t server,
   int new_importance,
   int new_quality);

//Description: The operation enqueues a request to change the quality and 
//importance parameters of the specified server, and returns immediately.
//The change operation is performed as soon as it is practical;
//meanwhile the system operation will continue normally.


////////////////////////////////////////////////////////////
//                  SCHEDULING BOUNDED WORKLOADS
////////////////////////////////////////////////////////////

int
fsf_schedule_next_timed_job 
  (const struct timespec *at_absolute_time,
   struct timespec       *next_budget,
   struct timespec       *next_period,
   bool                  *was_deadline_missed,
   bool                  *was_budget_overran);

//Description: This operation is invoked by threads associated with
//bounded workload servers to indicate that a job has been completed
//(and that the scheduler may reassign the unused capacity of the
//current job to other servers), and also when the first job require
//to be scheduled. The system will activate the job at the specified 
//absolute time, and will then use the scheduling rules to determine 
//when the job can run, at which time the call returns. Upon return, 
//the system reports the current period and budget for the current
//job, whether the deadline of the previous job was missed or not, 
//and whether the budget of the previous job was overrun or not.


int
fsf_schedule_next_event_triggered_job 
  (fsf_synch_object_handle_t *synch_handle,
   struct timespec           *next_budget,
   struct timespec           *next_period,
   bool                      *was_deadline_missed,
   bool                      *was_budget_overran);

//Description: This operation is invoked by threads associated with
//bounded workload servers to indicate that a job has been completed
//(and that the scheduler may reassign the unused capacity of the
//current job to other servers), and also when the first job require
//to be scheduled. If the specified synchronization object has events 
//queued, one of them is dequeued; otherwise the server will wait upon 
//the specified synchronization object until it is signalled. Then, the 
//system will use the scheduling rules to determine when the job can run 
//and the call will return at that time. Upon return, the system reports
//the current period and budget for the current job, whether the deadline
//of the previous job was missed or not, and whether the budget of the
//previous job was overrun or not.


//////////////////////////////////////////////////////////////
//           OBTAINING INFORMATION FROM THE SCHEDULER
//////////////////////////////////////////////////////////////

int
fsf_get_available_capacity (fsf_server_id_t server, float *capacity);

//Description: This operation returns the current spare capacity (in 
//percentage of processor or network utilization), currently assigned 
//to the importance level of the specified server.

int
fsf_get_total_quality (fsf_server_id_t server, int *total_quality);
 
//Description: This operation returns the sum of the quality parameters 
//for all servers in the system of importance level equal to that of 
//the specified server.

int
fsf_is_admission_test_enabled();
  
//Description: Returns true if the system is configured with the 
//on-line admission test enabled, or false otherwise.


// Mutex function

// Initialization
// here we initialize the second and third field of the structure
void fsf_init_shared_op(fsf_shared_operation_t     *op,
			fsf_shared_op_id_t  op_id,
			struct timespec     wcet);

// initialization
// The shared object id is set equal to the second parameter.
int fsf_init_shared_object(fsf_shared_object_t *obj, 
			    fsf_shared_obj_id_t id);

// Declare an operation
// This function is used to declare that a shared object has 
//    a synchronized operation on it. 
// It checks if another operation with the same id has already been 
//    declared; if so, return false (-1).
// The obj_id field of the operation is set equal to the shared object id.
// the structure op is copied in the first free element in the array 
//    shared_op pof the structure obj. If there are no more free element, 
//    returns -1.
// Returns 0 if the operation has been completed, -1 otherwise.

int fsf_declare_shared_object_operation(fsf_shared_object_t *obj,
					fsf_shared_operation_t *op);


int fsf_lock_object(fsf_shared_operation_t *op);

int fsf_unlock_object(fsf_shared_operation_t *op);

// set contract parameters
// specify a list of operations for each contract
// the list is specified as a pointer to an array.
// the size of the array is specified as third parameter
// returns 0 if all operations are correctly initialized
// returns -1 otherwise.

int fsf_set_contract_synchronization_parameters(
    fsf_contract_parameters_t *contract,
    const fsf_shared_operation_t *shared_ops,
    size_t op_num);



#endif // _FSF_CONTRACT_H_
