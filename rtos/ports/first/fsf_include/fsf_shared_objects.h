
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
   @file Shared Objects
 */

//fsf_shared_objects.h
//==================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// FSF(FIRST Scheduling Framework) 
// shared objects functionality
//====================================================

#include <pthread.h>
#include "fsf_basic_types.h"
#include "fsf_core.h"

#ifndef _FSF_SHARED_OBJECTS_H_
#define _FSF_SHARED_OBJECTS_H_

#define FSF_SHARED_OBJECTS_MODULE_SUPPORTED       1


// These constants are defined in the fsf_configuration_parameters.h
// file:

//#define FSF_MAX_N_SHARED_OBJECTS    100
//#define FSF_MAX_N_CRITICAL_SECTIONS 20


//// The definition of this types is in fsf_basic_types.h
//
//// Shared object identifier (null character terminated string)
//typedef char  * fsf_shared_obj_id_t;     
//
//// Shared object handle (opaque type)
//typedef FSF_SHARED_OBJ_HANDLE_T_OPAQUE  fsf_shared_obj_handle_t; 
//
//// Critical section data
//typedef struct {
//   fsf_shared_obj_handle_t obj_handle;
//   struct timespec         wcet;  //Execution time
//} fsf_critical_section_data_t;
//
//// List of critical sections
//typedef struct {
//  int size; // = 0
//  fsf_critical_section_data_t  
//      section[FSF_MAX_N_CRITICAL_SECTIONS];
//} fsf_critical_sections_t;
//

/////////////////////////////////////////////////////
//           SHARED OBJECTS & OPERATIONS MANAGEMENT
/////////////////////////////////////////////////////

/**
   \ingroup shobjmodule

   Initialization of shared objects. If the object identified by
   obj_id does not yet exist it is created, a handle to the object is
   returned in the variable pointed to by obj_handle, and the
   specified mutex is initialized with the appropriate attributes
   necessary for the current implementation.  If the object already
   exists, the function fails.

   @param [in] obj_id shared object id
   @param [out] obj_handle pointer to a shared object handle
   @param [out] mutex pointer to a mutex variable

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if obj_id, obj_handle, or mutex are NULL
   @retval FSF_ERR_SHARED_OBJ_ALREADY_INITIALIZED if the object identified
       by obj_id already exists
   @retval others It may also return any of the error codes that are 
       returned by the pthread_mutex_init() POSIX function call
*/
int fsf_init_shared_object
   (fsf_shared_obj_id_t      obj_id,
    fsf_shared_obj_handle_t *obj_handle,
    pthread_mutex_t         *mutex);


/**
   \ingroup shobjmodule

   Getting the handle of shared
   objects. If the object already exists a handle to the object is
   returned in the variable pointed to by obj_handle. Otherwise, an
   error code is returned by the function.

   @param [in] obj_id shared object id
   @param [out] obj_handle pointer to a shared object handle

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if obj_id or obj_handle are NULL
   @retval FSF_ERR_SHARED_OBJ_NOT_INITIALIZED if the shared object identified
       by obj_id does not exist
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD : if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
       running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER : if the server of the calling 
       thread has been cancelled or it is not valid
*/
int fsf_get_shared_object_handle
   (fsf_shared_obj_id_t      obj_id,
    fsf_shared_obj_handle_t *obj_handle);

/**
   \ingroup shobjmodule
   
   Getting the mutex of shared objects. If the object exists, a
   pointer to its associated mutex is returned in the variable pointed
   to by mutex. Otherwise, an error code is returned by the function.

   @param [in] obj_handle  shared object handle
   @param [out] mutex pointer to a pointer to a mutex variable, 
        (remember that the function give back a pointer to a mutex!)

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if obj_handle or mutex are NULL  or obj_handle
       is not correct or reference a wrong shared object
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD : if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or not 
       running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server of the calling thread 
       has been cancelled or it is not valid
*/
int fsf_get_shared_object_mutex
   (fsf_shared_obj_handle_t  obj_handle,
    pthread_mutex_t          **mutex);


/////////////////////////////////////////////////////
//                       CONTRACT PARAMETERS
/////////////////////////////////////////////////////

/**
   \ingroup shobjmodule
   
   The operation updates the specified contract parameters object by
   setting its critical sections to the specified input parameter.


   @param[in] contract the service contract
   @param[in] critical_sections list of critical sections accessed by tasks
       belonging to the contract

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT :if any of the pointers is NULL or 
       the size of the critical_sections structure is less than zero
       or grater than FSF_MAX_N_CRITICAL_SECTIONS
*/
int
fsf_set_contract_synchronization_parameters
  (fsf_contract_parameters_t     *contract,
   const fsf_critical_sections_t *critical_sections);


/**
   \ingroup shobjmodule

   The operation obtains from the specified contract parameters object
   its critical sections, and copies them to the places pointed to by
   the specified input parameter.  Only those critical_section_data
   records that are in use in the critical_sections structure are
   copied (according to its size field).

   @param[in] contract pointer to a contract
   @param[out] critical_sections list of critical sections to be filled

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if any of the pointers is NULL
*/
int
fsf_get_contract_synchronization_parameters
  (const fsf_contract_parameters_t *contract,
   fsf_critical_sections_t         *critical_sections);


#endif // _FSF_SHARED_OBJECTS_H_
