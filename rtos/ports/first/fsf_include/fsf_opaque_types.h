
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

//fsf_opaque_types.h
//=======================================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// Basic FSF(FIRST Scheduling Framework) contract management opaque types
//=======================================================================
// Implementation dependent definitions

#include <signal.h>

#ifndef _FSF_OPAQUE_TYPES_H_
#define _FSF_OPAQUE_TYPES_H_


#define FSF_CONTRACT_PARAMETERS_T_OPAQUE struct {   \
\
  struct timespec         budget_min;                \
  struct timespec         period_max;                \
  struct timespec         budget_max;                \
  struct timespec         period_min;                \
  fsf_workload_t          workload;                  \
                                                     \
  bool                    d_equals_t;                \
  struct timespec         deadline;                  \
  int                     budget_overrun_sig_notify; \
  union sigval            budget_overrun_sig_value;  \
  int                     deadline_miss_sig_notify;  \
  union sigval            deadline_miss_sig_value;   \
                                                     \
  fsf_granularity_t       granularity;               \
  fsf_utilization_set_t   utilization_set;           \
  int                     quality;                   \
  int                     importance;                \
                                                     \
  fsf_preemption_level_t  preemption_level;          \
  fsf_critical_sections_t critical_sections;         \
                                                     \
  fsf_sched_policy_t      policy;                    \
                                                     \
  fsf_network_id_t        network_id;                \
  bool                    granted_capacity_flag;     \
                                                     \
}


//Default values for fsf_contract_parameters_t:
//  budget_min                 => {0,0};
//  period_max                 => {0,0};
//  budget_max                 => {0,0};
//  period_min                 => {0,0};
//  workload                   => DEFAULT_WORKLOAD;

//  d_equals_t                 => DEFAULT_D_EQUALS_T;
//                               (false or true)
//  deadline                   => DEFAULT_DEADLINE;
//  budget_overrun_sig_notify  => 0;  (signal number)
//  budget_overrun_sig_value   => {0, NULL};
//  deadline_miss_sig_notify   => 0;  (signal number)
//  deadline_miss_sig_value    => {0, NULL};
//
//  granularity                => DEFAULT_GRANULARITY;

//  utilization_set;           => size = 0
//  quality                    => DEFAULT_QUALITY; 
//                                (range 0..2**32-1)
//  importance                 => DEFAULT_IMPORTANCE;
//                                (range 1..5)
//
//  preemption_level           => 0;
//                                (range 1..2**32-1)
//  critical_sections          => size = 0
//
//  policy                     => DEFAULT_SCHED_POLICY;
//                                (FSF_NONE)             
//
//  network_id                 => FSF_NULL_NETWORK_ID;
//                                (0)
//  granted_capacity_flag      => false;


#define FSF_SYNCH_OBJ_HANDLE_T_OPAQUE int

#define FSF_SHARED_OBJ_HANDLE_T_OPAQUE int


//opaque types for fsf endpoints
#define FSF_SEND_ENDPOINT_T_OPAQUE int

#define FSF_RECEIVE_ENDPOINT_T_OPAQUE int


#endif // _FSF_OPAQUE_TYPES_H_
