
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
#include "fsf_server.h"
#include <kernel/descr.h>
#include <kernel/func.h>
#include <pistar.h>
#include <ll/i386/string.h>

#define MAX_SHARED_NAME 15

struct hash_entry {
  mutex_t mx;
  char	shared_obj_name[MAX_SHARED_NAME];
  FSF_SHARED_OBJ_HANDLE_T_OPAQUE id;
};


#define MAX_HASH_ENTRY FSF_MAX_N_SHARED_OBJECTS 
struct 	hash_entry htable[MAX_HASH_ENTRY];


/*----------------------------------------------------------------------*/
/* hash_fun() : address hash table				 	*/
/*----------------------------------------------------------------------*/
static int hash_fun(fsf_shared_obj_id_t id)
{
    return (*id % MAX_HASH_ENTRY);
}

void fsf_register_shared_object(void) {
int i=0;
// Init Hash table
//kern_printf("(IT SO)\n");
for (i=0; i<MAX_HASH_ENTRY; i++) {
  htable[i].id=-1;

}

}
int fsf_init() {

  FSF_start_service_task();
  FSF_init_synch_obj_layer();
  return 0;
}


int fsf_initialize_contract
  (fsf_contract_parameters_t *contract)
{
  struct timespec default_deadline = FSF_DEFAULT_DEADLINE; 

  /* Check */
  if (!contract) return  FSF_ERR_BAD_ARGUMENT;

  /* Set to default value */
  NULL_TIMESPEC(&contract->budget_min);
  NULL_TIMESPEC(&contract->budget_max);
  NULL_TIMESPEC(&contract->period_min);
  NULL_TIMESPEC(&contract->period_max);

  contract->workload = FSF_DEFAULT_WORKLOAD;

  contract->policy = FSF_DEFAULT_SCHED_POLICY;

  contract->d_equals_t = FSF_DEFAULT_D_EQUALS_T;

  TIMESPEC_ASSIGN(&contract->deadline,&default_deadline);

  contract->budget_overrun_sig_notify = 0;
  memset(&contract->budget_overrun_sig_value,0,sizeof(union sigval));

  contract->deadline_miss_sig_notify = 0;
  memset(&contract->deadline_miss_sig_value,0,sizeof(union sigval));

  contract->granularity = FSF_DEFAULT_GRANULARITY;
  contract->utilization_set.size = 0;
  contract->quality = FSF_DEFAULT_QUALITY;
  contract->importance = FSF_DEFAULT_IMPORTANCE;

  contract->preemption_level = 0;
  contract->critical_sections.size = 0;
  
  return 0;

}
 
int fsf_set_contract_basic_parameters
  (fsf_contract_parameters_t *contract,
   const struct timespec  *budget_min,
   const struct timespec  *period_max,  
   fsf_workload_t          workload)
{

  if (!contract) return  FSF_ERR_BAD_ARGUMENT;

  if (budget_min->tv_sec < 0 || budget_min->tv_nsec > 1000000000)
     return FSF_ERR_BAD_ARGUMENT;
 
  if (period_max->tv_sec < 0 || period_max->tv_nsec > 1000000000)
     return FSF_ERR_BAD_ARGUMENT;

  if (budget_min && (budget_min->tv_sec!=0 || budget_min->tv_nsec!=0)) {
     TIMESPEC_ASSIGN(&contract->budget_min,budget_min);
     TIMESPEC_ASSIGN(&contract->budget_max,budget_min);
  } else return FSF_ERR_BAD_ARGUMENT;
  
  if (period_max && (period_max->tv_sec!=0 || period_max->tv_nsec!=0)) {
     TIMESPEC_ASSIGN(&contract->period_max,period_max);
     TIMESPEC_ASSIGN(&contract->period_min,period_max);
  } else return FSF_ERR_BAD_ARGUMENT;

  switch(workload) {
     case FSF_INDETERMINATE:
     case FSF_BOUNDED:
     case FSF_OVERHEAD: 
  	contract->workload = workload;
        break;
     default: return FSF_ERR_BAD_ARGUMENT;
  }


  return 0;
 
}

int fsf_get_contract_basic_parameters
  (const fsf_contract_parameters_t *contract,
   struct timespec  *budget_min,
   struct timespec  *period_max,
   fsf_workload_t   *workload)
{

  if (!contract) return FSF_ERR_BAD_ARGUMENT;

  TIMESPEC_ASSIGN(budget_min,&contract->budget_min);
  TIMESPEC_ASSIGN(period_max,&contract->period_max); 

  *workload = contract->workload;

  return 0;

}

int fsf_set_contract_timing_requirements
  (fsf_contract_parameters_t *contract,
   bool                   d_equals_t,
   const struct timespec *deadline,
   int                    budget_overrun_sig_notify,
   union sigval           budget_overrun_sig_value,
   int                    deadline_miss_sig_notify,
   union sigval           deadline_miss_sig_value)
{

  if (!contract) return FSF_ERR_BAD_ARGUMENT;
  if ((d_equals_t==true && deadline != FSF_NULL_DEADLINE) ||
      (d_equals_t==false && deadline == FSF_NULL_DEADLINE))
     return FSF_ERR_BAD_ARGUMENT;
  if (deadline != FSF_NULL_DEADLINE && TIMESPEC_A_GT_B(deadline, &contract->period_max))
     return FSF_ERR_BAD_ARGUMENT;

  contract->d_equals_t = d_equals_t;

  if (deadline) TIMESPEC_ASSIGN(&contract->deadline,deadline);

  contract->budget_overrun_sig_notify = budget_overrun_sig_notify;
  contract->budget_overrun_sig_value = budget_overrun_sig_value;
  contract->deadline_miss_sig_notify = deadline_miss_sig_notify;
  contract->deadline_miss_sig_value = deadline_miss_sig_value;

  return 0;

}

int fsf_get_contract_timing_requirements
  (const fsf_contract_parameters_t *contract,
   bool                            *d_equals_t,
   struct timespec                 *deadline,
   int                             *budget_overrun_sig_notify,
   union sigval                    *budget_overrun_sig_value,
   int                             *deadline_miss_sig_notify,
   union sigval                    *deadline_miss_sig_value)
{

  if (!contract) return  FSF_ERR_BAD_ARGUMENT;
                                                                                                                             
  *d_equals_t = contract->d_equals_t;
                                                                                                                             
  TIMESPEC_ASSIGN(deadline,&contract->deadline);
                                                                                                                             
  *budget_overrun_sig_notify = contract->budget_overrun_sig_notify;
  *budget_overrun_sig_value = contract->budget_overrun_sig_value;
  *deadline_miss_sig_notify = contract->deadline_miss_sig_notify;
  *deadline_miss_sig_value = contract->deadline_miss_sig_value;

  return 0;

}

int
fsf_set_contract_reclamation_parameters
  (fsf_contract_parameters_t   *contract,
   const struct timespec       *budget_max,
   const struct timespec       *period_min,
   fsf_granularity_t            granularity,
   const fsf_utilization_set_t *utilization_set,
   int                          quality,
   int                          importance)
{

  if (!contract) return  FSF_ERR_BAD_ARGUMENT;

  if (budget_max->tv_sec < 0 || budget_max->tv_nsec > 1000000000)
     return FSF_ERR_BAD_ARGUMENT;
 
  if (period_min->tv_sec < 0 || period_min->tv_nsec > 1000000000)
     return FSF_ERR_BAD_ARGUMENT;

  contract->granularity = granularity;

  if (utilization_set) memcpy(&contract->utilization_set,utilization_set,sizeof(fsf_utilization_set_t));

  if (budget_max) TIMESPEC_ASSIGN(&contract->budget_max,budget_max);
  if (period_min) TIMESPEC_ASSIGN(&contract->period_min,period_min);

  contract->quality = quality;
  contract->importance = importance;

  return 0;

}
                                                                                                                             
int fsf_get_contract_reclamation_parameters
  (const fsf_contract_parameters_t *contract,
   struct timespec                 *budget_max,
   struct timespec                 *period_min,
   fsf_granularity_t               *granularity,
   fsf_utilization_set_t           *utilization_set,
   int                             *quality,
   int                             *importance)
{

  if (!contract) return FSF_ERR_BAD_ARGUMENT;
                                                                                                                             
  *granularity = contract->granularity;
  if (utilization_set)
    memcpy(utilization_set,&contract->utilization_set,sizeof(fsf_utilization_set_t));

  TIMESPEC_ASSIGN(budget_max,&contract->budget_max);
  TIMESPEC_ASSIGN(period_min,&contract->period_min); 
                                                                                                                             
  *quality = contract->quality;
  *importance = contract->importance;
                                                                                                                             
  return 0;

}

int fsf_set_contract_synchronization_parameters
  (fsf_contract_parameters_t     *contract,
   const fsf_critical_sections_t *critical_sections)
{

  if (!contract) return FSF_ERR_BAD_ARGUMENT;

  if (critical_sections) memcpy(&contract->critical_sections,critical_sections,sizeof(fsf_critical_sections_t));

  return 0;

}
                                                                                                                             
int
fsf_get_contract_synchronization_parameters
  (const fsf_contract_parameters_t *contract,
   fsf_critical_sections_t         *critical_sections)
{
  
  if (!contract) return FSF_ERR_BAD_ARGUMENT;
  if (critical_sections)
    memcpy(critical_sections,&contract->critical_sections,sizeof(fsf_critical_sections_t));
  
  return 0;

}

int 
fsf_set_contract_scheduling_policy
  (fsf_contract_parameters_t *contract,
   fsf_sched_policy_t         sched_policy)
{

  if (!contract) return  FSF_ERR_BAD_ARGUMENT;

  contract->policy = sched_policy;

  return 0;

}

int
fsf_get_contract_scheduling_policy
  (const fsf_contract_parameters_t *contract,
   fsf_sched_policy_t              *sched_policy)
{

  if (!contract) return FSF_ERR_BAD_ARGUMENT;

  *sched_policy = contract->policy;

  return 0;

}

/* OLD VERSION
// mutex lock function

int fsf_lock_object(fsf_shared_operation_t *op) {
  int index, oldindex;
  
  index=hash_fun(&(op->obj_id));  
  //kern_printf("index %d, htableid %d, obj_op_id %d", index, htable[index].id,op->obj_id); 

  if (htable[index].id!=op->obj_id) {
    oldindex=index;
    index = (index + 1) % MAX_HASH_ENTRY;
    // find 
    while (htable[index].id != op->obj_id && index!=oldindex) index=(index+1) % MAX_HASH_ENTRY;
    if (index==oldindex) return -1;
  }
  //kern_printf("(SO LK)"); 
  // we need a special implementation for mutex_lock ... additional parameter
  return PISTAR_lock(FSF_get_shared_object_level(), &htable[index].mx,TIMESPEC2USEC(&op->wcet));

}

int fsf_unlock_object(fsf_shared_operation_t *op) {

  int index, oldindex;

  index=hash_fun(&op->obj_id);  

  if (htable[index].id!=op->obj_id) {
    oldindex=index;
    index = (index + 1) % MAX_HASH_ENTRY;
    // find 
    while (htable[index].id != op->obj_id && index!=oldindex) index=(index+1) % MAX_HASH_ENTRY;
    if (index==oldindex) return -1;
  }
  //kern_printf("UNLOCK index %d", index);

  return mutex_unlock(&htable[index].mx);
  


}

*/


int fsf_get_shared_object_handle
   (fsf_shared_obj_id_t      obj_id,
    fsf_shared_obj_handle_t *obj_handle) {
  int index;
  int oldindex;
  SYS_FLAGS f;

  f=kern_fsave();

  index=hash_fun(obj_id);

  if (strcmp(htable[index].shared_obj_name,obj_id) == 0) {
	kern_frestore(f);
	return index;
  }

  if (htable[index].id!=0) {
    oldindex=index;
    index = (index + 1) % MAX_HASH_ENTRY;
    // collision detection
    while (htable[index].id !=0 && index!=oldindex) {
      if (strcmp(htable[index].shared_obj_name,obj_id) == 0) {
	kern_frestore(f);
	return index;
      }
      index=(index+1) % MAX_HASH_ENTRY;
    }
    
    // table is full
    if (index==oldindex) { 
      kern_frestore(f);
      return -1;
    }
  }

  kern_frestore(f);
  return -1;

}



int fsf_init_shared_object
   (fsf_shared_obj_id_t      id,
    fsf_shared_obj_handle_t *obj,
    pthread_mutex_t         *mutex) {
  int index;
  int oldindex;
  PISTAR_mutexattr_t a;
  SYS_FLAGS f;

  PISTAR_mutexattr_default(a); 
  //kern_printf("(SI SO)\n");
  f=kern_fsave();
  
  index=hash_fun(id);
  //kern_printf("Index %d Hash %d", index, htable[index].id);

  if (strcmp(htable[index].shared_obj_name,id) == 0) {
	kern_frestore(f);
	return -1;
  }

  if (htable[index].id!=0) {
    oldindex=index;
    index = (index + 1) % MAX_HASH_ENTRY;
    // collision detection
    while (htable[index].id !=0 && index!=oldindex) index=(index+1) % MAX_HASH_ENTRY;
    // table is full
    if (index==oldindex) { 
	kern_frestore(f);
	return -1;
    }
  }
  
  //obj->size=0;

  mutex_init(&(htable[index]).mx, &a);
  mutex=&(htable[index]).mx;
  strncpy(htable[index].shared_obj_name, id,MAX_SHARED_NAME);
  *obj=index;
  kern_frestore(f);

  return 0;

  //kern_printf("(EI SO)\n");
}

/* OLD VERSION
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
					fsf_shared_operation_t *op) {
  int i;
  SYS_FLAGS f;

  f=kern_fsave();

  for (i=0; i<obj->size; i++) {
    // fail if the operation already declared
    if (op->op_id==obj->shared_op[i].op_id) {
      kern_frestore(f);
      return -1;    
    }
  }
  
  // fail if the object is full
  if (obj->size>(FSF_MAX_SHARED_OPERATION-1)) { 
    kern_frestore(f);
    return -1;  
  }

  //kern_printf("(DO SO)");
  obj->size++;
  obj->shared_op[i].op_id = op->op_id;
  TIMESPEC_ASSIGN(&obj->shared_op[i].wcet,&op->wcet);
  obj->shared_op[i].obj_id = obj->obj_id;
  op->obj_id = obj->obj_id;

  kern_frestore(f);  

  return 0;


}
*/

/* OLD VERSION
int fsf_set_contract_synchronization_parameters(
    fsf_contract_parameters_t *contract,
    const fsf_shared_operation_t *shared_ops,
    size_t op_num)
{
  if (shared_ops && op_num < FSF_MAX_SHARED_OPERATION) 
    memcpy(&contract->shared_operations,shared_ops,op_num); 
  else return -1;
  
  return 0;
}
*/

// MARTE IMPLEMENTATION SPECIFIC MODULE

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
   fsf_preemption_level_t         preemption_level) {
  return 0;
}


//fsf_get_contract_preemption_level: The operation obtains from the
//specified contract parameters object its preemption level and copies
//it to the place pointed to by the specified input parameter.

int
fsf_get_contract_preemption_level
  (const fsf_contract_parameters_t *contract,
   fsf_preemption_level_t          *preemption_level)  {
  return 0;
}



//fsf_set_service_thread_preemption_level: this function sets the
//preemption level of the service thread to the specified value. The
//initial preemption level is a configurable parameter

int
fsf_set_service_thread_preemption_level
  (fsf_preemption_level_t         preemption_level) {
  return 0;
}


//fsf_get_service_thread_preemption_level: this function stores the
//current preemption level of the service thread in the variable
//pointed to by preemption_level

int
fsf_get_service_thread_preemption_level
  (fsf_preemption_level_t        *preemption_level) {
  return 0;
}



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
fsf_thread_exit (void *value_ptr) {
  return;
}



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
   fsf_preemption_level_t   preemption_level) {
  return 0;
}



//fsf_get_shared_obj_preemption_level: The operation obtains from the
//specified shared object its preemption level and copies
//it to the place pointed to by the specified input parameter.

int
fsf_get_shared_obj_preemption_level
  (fsf_shared_obj_handle_t  obj_handle,
   fsf_preemption_level_t  *preemption_level) {
  return 0;
}
