
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

#ifndef _FSF_SERVER_H_
#define _FSF_SERVER_H_

#define FSF_GRUBSTAR
//#define FSF_CBSSTAR

int fsf_get_server_level(void);
int fsf_get_remain_budget(fsf_server_id_t );
int fsf_settask_nopreemptive(fsf_server_id_t *server, pthread_t thread);
int fsf_settask_preemptive(fsf_server_id_t *server, pthread_t thread);
int FSF_get_shared_object_level(void);
void FSF_start_service_task(void);
int FSF_register_module(int posix_level, int server_level, bandwidth_t max_bw);
void FSF_init_synch_obj_layer();

#ifdef FSF_CBSSTAR

#include "cbsstar.h"
#define SERVER_setbudget CBSSTAR_setbudget
#define SERVER_adjust_budget CBSSTAR_adjust_budget
#define SERVER_removebudget CBSSTAR_removebudget
#define SERVER_get_local_scheduler_id_from_budget CBSSTAR_get_local_scheduler_id_from_budget
#define SERVER_get_local_scheduler_id_from_pid CBSSTAR_get_local_scheduler_id_from_pid
#define SERVER_get_local_scheduler_level_from_budget CBSSTAR_get_local_scheduler_level_from_budget
#define SERVER_get_local_scheduler_level_from_pid CBSSTAR_get_local_scheduler_level_from_pid
#define SERVER_getbudgetinfo CBSSTAR_getbudgetinfo
#define SERVER_get_last_reclaiming CBSSTAR_get_last_reclaiming
#define SERVER_disable_server CBSSTAR_disable_server
#define SERVER_get_remain_capacity CBSSTAR_get_remain_capacity
#define SERVER_get_renegotiation_status CBSSTAR_get_renegotiation_status
#endif

#ifdef FSF_CBSNHSTAR
#include "cbsnhstar.h"
#define SERVER_setbudget CBSNHSTAR_setbudget
#define SERVER_adjust_budget CBSNHSTAR_adjust_budget
#define SERVER_removebudget CBSNHSTAR_removebudget
#define SERVER_get_local_scheduler_id_from_budget CBSNHSTAR_get_local_scheduler_id_from_budget
#define SERVER_get_local_scheduler_id_from_pid CBSNHSTAR_get_local_scheduler_id_from_pid
#define SERVER_get_local_scheduler_level_from_budget CBSNHSTAR_get_local_scheduler_level_from_budget
#define SERVER_get_local_scheduler_level_from_pid CBSNHSTAR_get_local_scheduler_level_from_pid
#define SERVER_getbudgetinfo CBSNHSTAR_getbudgetinfo
#define SERVER_get_last_reclaiming CBSNHSTAR_get_last_reclaiming
#define SERVER_get_remain_capacity CBSNHSTAR_get_remain_capacity
//#define SERVER_return_bandwidth GRUBSTAR_return_bandwidth 
#define SERVER_disable_server CBSNHSTAR_disable_server
#define SERVER_get_renegotiation_status CBSNHSTAR_get_renegotiation_status
#define SERVER_getdeadline CBSNHSTAR_getdeadline

#endif

#ifdef FSF_GRUBSTAR

#include "grubstar.h"
#define SERVER_setbudget GRUBSTAR_setbudget
#define SERVER_adjust_budget GRUBSTAR_adjust_budget
#define SERVER_removebudget GRUBSTAR_removebudget
#define SERVER_get_local_scheduler_id_from_budget GRUBSTAR_get_local_scheduler_id_from_budget
#define SERVER_get_local_scheduler_id_from_pid GRUBSTAR_get_local_scheduler_id_from_pid
#define SERVER_get_local_scheduler_level_from_budget GRUBSTAR_get_local_scheduler_level_from_budget
#define SERVER_get_local_scheduler_level_from_pid GRUBSTAR_get_local_scheduler_level_from_pid
#define SERVER_getbudgetinfo GRUBSTAR_getbudgetinfo
#define SERVER_get_last_reclaiming GRUBSTAR_get_last_reclaiming
#define SERVER_get_remain_capacity GRUBSTAR_get_remain_capacity
//#define SERVER_return_bandwidth GRUBSTAR_return_bandwidth 
#define SERVER_disable_server GRUBSTAR_disable_server
#define SERVER_get_renegotiation_status GRUBSTAR_get_renegotiation_status
#define SERVER_getdeadline GRUBSTAR_getdeadline
#endif

#endif
