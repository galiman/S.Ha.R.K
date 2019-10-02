
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
   @file Spare capacity file.
 */
//fsf_spare_capacity.h
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
// spare capacity sharing functionality
//===================================================

#include <time.h>
#include <stdint.h>
#include "fsf_basic_types.h"
#include "fsf_core.h"

#ifndef _FSF_SPARE_CAPACITY_H_
#define _FSF_SPARE_CAPACITY_H_

#define FSF_SPARE_CAPACITY_MODULE_SUPPORTED       1


//// The definition of this types is in fsf_basic_types.h
//
//// Granularity of spare capacity requirements
//typedef enum {FSF_CONTINUOUS, FSF_DISCRETE} fsf_granularity_t;
//
//// Utilization (budget and period) value
//typedef struct {
//  struct timespec    budget;    // Execution time
//  struct timespec    period;    // Period
//} fsf_utilization_value_t;
//
////List of utilization values
//typedef struct {
//  int                         size; // = 0
//  fsf_utilization_value_t     
//      value[FSF_MAX_N_UTILIZATION_VALUES]; 
//      //unit change to value.....
//} fsf_utilization_set_t;
//
//
//// Constants for assigning default values
//#define FSF_DEFAULT_GRANULARITY         FSF_CONTINUOUS
//#define FSF_DEFAULT_QUALITY             0
//#define FSF_DEFAULT_IMPORTANCE          1
//
//
//// Constants for omitting the assignment of values to specific
//// arguments in calls to initialization functions
//
//#define FSF_NULL_UTILIZATION_SET     (fsf_utilization_set_t *)NULL
//

/**
   \ingroup sparemodule

   The operation updates the specified contract parameters object by
   setting its maximum usable budget, minimum period, granularity,
   utilization set, quality, and importance to the specified input
   parameters.

   @param [in] contract pointer ot the contract
   @param [in] budget_max  maximum budget this contract can obtain
   @param [in] period_min  minimum period this contract can obtain
   @param [in] granularity can be either FSF_CONTINUOUS or FSF_DISCRETE
   @param [in] utilization_set in case the granularity is set to 
      FSF_DISCRETE it contains a list possible pairs (budget,period)
   @param [in] quality a number between FSF_MIN_QUALITY and FSF_MAX_QUALITY, 
               to control how the spare capacity is shared between 
               contracts with the same importance. The higher 
               this number is , the more likely we get a large increase 
               in the capacity
   @param [in] importance a numer between FSF_MIN_IMPORTANCE and 
               FSF_MAX_IMPORTANCE, used to control how the spare capacity
               is shared. The higher the number, the more likely we get 
               some spare capacity. 
 
   @retval 0 if the call is succesful
   @retval FSF_ERR_BAD_ARGUMENT if contract is NULL or one of the
       following conditions is true: 
       - (budget_max value is grater than period_max or smaller than
         budget_min);
       - (period_min is smaller than budget_mint or larger than period_max);
       - (granularity is neither FSF_CONTINUOUS nor FSF_DISCRETE);
       - (granularity is FSF_CONTINUOUS and 
        utilization_set is not FSF_NULL_UTILIZATION_SET)
       - (granularity is FSF_DISCRETE and utilization_set is 
          FSF_NULL_UTILIZATION_SET) 
       - (utilization_set is not FSF_NULL_UTILIZATION_SET and
         (size of utilization_set less than 2 or greater 
          than FSF_MAX_N_UTILIZATION_VALUES)
       - (quality < 0)
       - (importance is less than 1 or greater than FSF_N_IMPORTANCE_LEVELS)
       - (the utilization_set elements are not in increasing utilization order)
       - (the first utilization value in the utilization_set does not match
        the pair (budget_min, period_max) of the contract);
       - (the last utilization value in the utilization_set does not match
        the pair (budget_max, period_min) of the contract).
*/
int
fsf_set_contract_reclamation_parameters
  (fsf_contract_parameters_t   *contract,
   const struct timespec       *budget_max,
   const struct timespec       *period_min,
   fsf_granularity_t            granularity,
   const fsf_utilization_set_t *utilization_set,
   int                          quality,
   int                          importance);


/** 
   \ingroup sparemodule

    The operation obtains from the specified contract parameters
    object its granularity, utilization set, quality, and
    importance. Then copies them to the variables pointed to by the
    specified input parameters.  Only the utilization_values of the
    utilization_set that are in use, are copied (according to its size
    field).

    @retval 0 if the operation is succesful
    @retval FSF_ERR_BAD_ARGUMENT :  if contract is NULL

    @see fsf_set_contract_reclamation_parameters
*/
int
fsf_get_contract_reclamation_parameters
  (const fsf_contract_parameters_t *contract,
   struct timespec                 *budget_max,
   struct timespec                 *period_min,
   fsf_granularity_t               *granularity,
   fsf_utilization_set_t           *utilization_set,
   int                             *quality,
   int                             *importance);


/**
   \ingroup sparemodule

   The operation enqueues a request to change the quality and
   importance parameters of the specified server, and returns
   immediately. The change operation is performed as soon as it is
   practical; meanwhile the system operation will continue normally.

   @param server server id
   @param new_importance  the new importance
   @param new_quality   the new requested quality
   
   @retval FSF_ERR_BAD_ARGUMENT if 
       - the value of the server argument is not in range or
       - (quality < 0)
       - (importance is less than 1 or greater than FSF_N_IMPORTANCE_LEVELS).
   
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
       not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server has been cancelled 
       or it is not valid
*/
int
fsf_request_change_quality_and_importance
  (fsf_server_id_t server,
   int new_importance,
   int new_quality);


/**
   \ingroup sparemodule

   This operation calculates the sum of the quality parameters for all
   servers in the system of importance level equal to that of the
   specified server, and stores it in the variable pointed to by
   total_quality.

   @param [in] server server id
   @param [out] total_quality the total quality in the system
   
   @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument 
           is not in range or total_quality is NULL
   @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
       scheduled under the FSF
   @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
       not running
   @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server has been 
       cancelled or it is not valid
*/
int
fsf_get_total_quality 
   (fsf_server_id_t server, int *total_quality);


/** 
   \ingroup sparemodule

    This operation stores in the variable pointed to by capacity the
    spare capacity currently available for the importance level of the
    specified server. The capacity is the utilization (of the
    processor or of the network) and it is represented by an integer
    number between 0 (no utilization) and UINT32_MAX (all
    utilization).

    @retval FSF_ERR_BAD_ARGUMENT if the value of the server argument is 
            not in range or capacity is NULL
    @retval FSF_ERR_NOT_SCHEDULED_CALLING_THREAD if the calling thread is not
            scheduled under the FSF
    @retval FSF_ERR_INVALID_SCHEDULER_REPLY the scheduler is wrong or 
            not running
    @retval FSF_ERR_NOT_CONTRACTED_SERVER if the server has been cancelled 
            or it is not valid
*/
int
fsf_get_available_capacity (
    fsf_server_id_t server, uint32_t *capacity);

/* @} */

#endif // _FSF_SPARE_CAPACITY_H_
