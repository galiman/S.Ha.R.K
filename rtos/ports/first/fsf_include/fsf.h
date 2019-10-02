
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

//fsf.h
//===========================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// FSF(FIRST Scheduling Framework) available functionality
//============================================================

#include "fsf_configuration_parameters.h"
#include "fsf_opaque_types.h"
#include "fsf_basic_types.h"
#include "fsf_core.h"
#include "fsf_implementation_specific.h"



// Configure theses modules by commenting out the appropiate lines

#include "fsf_hierarchical.h"                 //
#include "fsf_shared_objects.h"
#include "fsf_dynamic_reclaiming.h"           //
#include "fsf_spare_capacity.h"
#include "fsf_distributed.h"                  //
#include "fsf_distributed_spare_capacity.h"   //

#ifndef _FSF_H_
#define _FSF_H_

#ifndef FSF_HIERARCHICAL_MODULE_SUPPORTED
    #define FSF_HIERARCHICAL_MODULE_SUPPORTED        0

#endif  //FSF_HIERARCHICAL_MODULE_SUPPORTED

#ifndef FSF_SHARED_OBJECTS_MODULE_SUPPORTED
    #define FSF_SHARED_OBJECTS_MODULE_SUPPORTED      0
#endif  //FSF_SHARED_OBJECTS_MODULE_SUPPORTED

#ifndef FSF_DYNAMIC_RECLAIMING_MODULE_SUPPORTED
    #define FSF_DYNAMIC_RECLAIMING_MODULE_SUPPORTED  0
#endif  //FSF_DYNAMIC_RECLAIMING_MODULE_SUPPORTED

#ifndef FSF_SPARE_CAPACITY_MODULE_SUPPORTED
    #define FSF_SPARE_CAPACITY_MODULE_SUPPORTED      0

    //Return warnings if spare_capacity module is not included:
 
    #define fsf_set_contract_reclamation_parameters  \
      (contract, budget_max, period_min, granularity,\
       utilization_set, quality,  importance)        \
       ( FSF_WRN_MODULE_NOT SUPPORTED )
    
    #define fsf_get_contract_reclamation_parameters  \
      (contract, budget_max, period_min, granularity,\
       utilization_set, quality, importance)         \
       ( FSF_WRN_MODULE_NOT SUPPORTED )
   
    #define fsf_request_change_quality_and_importance\
      (server, new_importance, new_quality)          \
       ( FSF_WRN_MODULE_NOT SUPPORTED )
        
    #define fsf_get_total_quality (server,total_quality)\
       ( FSF_WRN_MODULE_NOT SUPPORTED )

#endif  //FSF_SPARE_CAPACITY_MODULE_SUPPORTED

#ifndef FSF_DISTRIBUTED_MODULE_SUPPORTED
    #define FSF_DISTRIBUTED_MODULE_SUPPORTED 0
#endif  //FSF_DISTRIBUTED_MODULE_SUPPORTED

#ifndef FSF_DISTRIBUTED_SPARE_CAPACITY_MODULE_SUPPORTED
    #define FSF_DISTRIBUTED_SPARE_CAPACITY_MODULE_SUPPORTED 0
   
    //Return warnings if distributed_spare_capacity module is not included:

    #define fsf_set_contract_granted_capacity_flag(contract,granted_capacity_flag)\
       ( FSF_WRN_MODULE_NOT SUPPORTED )
 
    #define fsf_get_contract_granted_capacity_flag(contract,granted_capacity_flag)\
       ( FSF_WRN_MODULE_NOT SUPPORTED )
   
    #definr fsf_set_server_capacity(server,new_period,new_budget)\
       ( FSF_WRN_MODULE_NOT SUPPORTED )

#endif  //FSF_DISTRIBUTED_SPARE_CAPACITY_MODULE_SUPPORTED


#endif // _FSF_H_
