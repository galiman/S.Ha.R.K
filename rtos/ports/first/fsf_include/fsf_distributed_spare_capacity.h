
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
   @file distributed spare capacity
*/

//fsf_distributed_spare_capacity.h
//=====================================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// FSF(FIRST Scheduling Framework)
// distributed spare capacity functionality
//=====================================================================

#include "fsf_core.h"
#include "fsf_distributed.h"

#ifndef _FSF_DISTRIBUTED_SPARE_CAPACITY_H_
#define _FSF_DISTRIBUTED_SPARE_CAPACITY_H_

#define FSF_DISTRIBUTED_SPARE_CAPACITY_MODULE_SUPPORTED       1

/**
   \ingroup distsparemodule

   This operation sets the granted capacity flag in the contract
   parameters object pointed to by contract to the boolean specified
   in granted_capacity_flag. This flag indicates to the scheduler that
   once the negotiation of the respective contract is finished, the
   first values for the budget and period given to the corresponding
   server must not be changed due to the negotiation or renegotiation
   of any other contract in the system. The period can change, though,
   if a renegotiation or a change of quality and importance is
   requested for the corresponding server.

   @param contract   the pointer to the contract object
   @param[in] granted_capacity_flag boolean value set to true to
     avoid the change in the period due to changes in other servers

   @retval  FSF_ERR_BAD_ARGUMENT :  if contract is NULL
*/
int
fsf_set_contract_granted_capacity_flag
  (fsf_contract_parameters_t *contract,
   bool                    granted_capacity_flag);


/**
   \ingroup distsparemodule

   This operation returns in the place pointed to by
   granted_capacity_flag the value of the corresponding flag in the
   contract parameters object pointed to by contract.
   
   @param[in] contract   the pointer to the contract object
   @param[in] granted_capacity_flag pointer to the variable that
     will contain the boolean value of the granted_capacity_flag

   @retval  FSF_ERR_BAD_ARGUMENT :  if any of the pointers is NULL 
*/
int
fsf_get_contract_granted_capacity_flag
  (fsf_contract_parameters_t *contract,
   bool                   *granted_capacity_flag);


/**
   \ingroup distsparemodule

   This operation is used to return spare capacity that was assigned
   to a server but that cannot be used due to restrictions in other
   servers of a distributed transaction.  This operation changes the
   cycle period and budget of the given server to the values given in
   new_period and new_budget, respectively.

   @param [in] server server id
   @param [in] new_period pointer to the new period value 
   @param [in] new_budget pointer to the new budget value 
   
   @retval  FSF_ERR_BAD_ARGUMENT :  if (the server does not have 
     the granted_capacity flag set) or
     (the new period is less than the current one), or
     (the new budget is larger than the current one), or
     (the new period is greater than the maximum period currently
       specified in the contract associated to the server), or
     (the new budget is smaller than the minimum budget in the contract) or
     (if the granularity is discrete and the new period and budget
      do not match any of the period-budget pairs in the utilization
      set of the server).
*/
int
fsf_set_server_capacity
   (fsf_server_id_t          server,
    const struct timespec   *new_period,
    const struct timespec   *new_budget);


#endif // _FSF_DISTRIBUTED_SPARE_CAPACITY_H_
