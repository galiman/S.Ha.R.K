
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

#include "fsf_configuration_parameters.h"
#include "fsf_core.h"
#include "fsf_server.h"

#include "fedfstar.h"
#include "posixstar.h"
#include "edfstar.h"
#include "nonestar.h"
#include "rmstar.h"

extern int fsf_server_level;

struct hash_entry {
  IQUEUE             threads;
  int                events;
  FSF_SYNCH_OBJ_HANDLE_T_OPAQUE id;
};

#define MAX_HASH_ENTRY FSF_MAX_N_SYNCH_OBJECTS 
struct 	hash_entry htable[MAX_HASH_ENTRY];

void FSF_init_synch_obj_layer() {
   int i;
   for (i=0; i<MAX_HASH_ENTRY; i++) 
	htable[i].id=-1;
}

/*----------------------------------------------------------------------*/
/* hash_fun() : address hash table				 	*/
/*----------------------------------------------------------------------*/
static int hash_fun(FSF_SYNCH_OBJ_HANDLE_T_OPAQUE *id)
{
    return ((int)id % MAX_HASH_ENTRY);
}

//#define FSF_DEBUG

int
fsf_create_synch_obj(fsf_synch_obj_handle_t *synch_handle)
{
  int index,oldindex;
  SYS_FLAGS f;
  f=kern_fsave();

  index=hash_fun(synch_handle); 
  
  if (index<0 || index>=MAX_HASH_ENTRY)  {
    kern_frestore(f);
    return FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }

  if (htable[index].id==1) {
    oldindex=index;
    index = (index + 1) % MAX_HASH_ENTRY;
    // find 
    while (htable[index].id == 1 && index!=oldindex) index=(index+1) % MAX_HASH_ENTRY;
    if (index==oldindex) {
      kern_frestore(f); 
      return FSF_ERR_TOO_MANY_SYNCH_OBJS;
    }  
  }
  //if (!synch_handle) return FSF_ERR_INVALID_SYNCH_OBJECT_HANDLE;

  iq_init(&(htable[index].threads), NULL, 0);  
  htable[index].events = 0;
  htable[index].id = 1;

  kern_frestore(f);

  *synch_handle=index;

  return 0;

}

int
fsf_signal_synch_obj(fsf_synch_obj_handle_t synch_handle)
{

  PID p;
  int index=synch_handle;
  fsf_server_id_t server;
  SYS_FLAGS f;
  f=kern_fsave();

  if (synch_handle<0 || synch_handle>=MAX_HASH_ENTRY)  {
    kern_frestore(f);
    return FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }

  if (htable[index].id==-1) { 
    kern_frestore(f);
    return  FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }
 
  //if (!synch_handle) return FSF_ERR_INVALID_SYNCH_OBJECT_HANDLE;
  
  if ((p = iq_getfirst(&(htable[index].threads))) != NIL) {
    fsf_get_server(p, &server);
    fsf_settask_preemptive(&server, p);
    task_activate(p);
  }
  else
    htable[index].events++;

  kern_frestore(f);
  return 0;

}

int
fsf_destroy_synch_obj(fsf_synch_obj_handle_t synch_handle)
{
  int index=synch_handle;
  SYS_FLAGS f;
  f=kern_fsave();
  
  if (synch_handle<0 || synch_handle>=MAX_HASH_ENTRY) {
    kern_frestore(f);
    return FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }

  if (htable[index].id==-1) {
    kern_frestore(f);
    return FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }
  htable[index].id=-1;
  //if (!synch_handle) return FSF_ERR_INVALID_SYNCH_OBJECT_HANDLE;

  while (iq_getfirst(&(htable[index].threads)) != NIL);  
  htable[index].events = 0;
  kern_frestore(f);

  return 0;

}

int restart_task(PID p) {
  fsf_server_id_t server;

  fsf_get_server(p, &server);
  fsf_settask_preemptive(&server, p);
  return task_activate(p);
}

int fsf_schedule_timed_job
  (const struct timespec *at_absolute_time,
   struct timespec       *next_budget,
   struct timespec       *next_period,
   bool                  *was_deadline_missed,
   bool                  *was_budget_overran)
{
  TIME T,Q,D;
  int budget, local_scheduler_level, scheduler_id;
  SYS_FLAGS f;
  fsf_server_id_t server;

  if (!at_absolute_time && (at_absolute_time->tv_sec < 0 || at_absolute_time->tv_nsec > 1000000000))
     return FSF_ERR_BAD_ARGUMENT;

  f=kern_fsave();

  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level, exec_shadow);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level, exec_shadow);

  if (proc_table[exec_shadow].task_level != local_scheduler_level) return 0;

  switch (scheduler_id) {
    case FSF_RR:
      budget = POSIXSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_EDF:
      budget = EDFSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_NONE:
      budget = NONESTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_FP:
      budget = RMSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
  case FSF_FEDF:
      budget = FEDFSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;

    default:
      budget = -1;
      break;
  }

  if (budget == -1) {
    kern_frestore(f);
    return FSF_ERR_BAD_ARGUMENT; 
  }

  if (next_budget != NULL && next_period != NULL) {

    SERVER_getbudgetinfo(fsf_server_level, &Q, &T, &D, budget);
 
    #ifdef FSF_DEBUG
      kern_printf("(budget %d Q=%d T=%d)",budget,(int)Q,(int)T);
    #endif

    next_budget->tv_sec = Q / 1000000;
    next_budget->tv_nsec = (Q % 1000000) * 1000;
    next_period->tv_sec = T / 1000000;
    next_period->tv_nsec = (T % 1000000) * 1000;

  }

  if (was_deadline_missed != NULL) 
    *was_deadline_missed = false;
  if (was_budget_overran != NULL) 
    *was_budget_overran = false;

  if (at_absolute_time != NULL) {    
    fsf_get_server(exec_shadow, &server);
    fsf_settask_nopreemptive(&server, exec_shadow);
    kern_event_post(at_absolute_time, (void (*)(void *))restart_task, (void *)(exec_shadow));
  }

#ifdef FSF_DEBUG
    if (at_absolute_time != NULL)
      kern_printf("(Next act s%d:us%d)",(int)at_absolute_time->tv_sec,(int)at_absolute_time->tv_nsec/1000);
    else
      kern_printf("(End Cycle %d)",exec_shadow);
#endif

  kern_frestore(f);
  task_endcycle();
 


  return 0;

}

int fsf_schedule_triggered_job
  (fsf_synch_obj_handle_t  synch_handle,
   struct timespec         *next_budget,
   struct timespec         *next_period,
   bool                    *was_deadline_missed,
   bool                    *was_budget_overran)
{

  TIME T,Q,D;
  int index=synch_handle;
  int budget, local_scheduler_level, scheduler_id;
  fsf_server_id_t server;
  SYS_FLAGS f;

  f=kern_fsave();
  
  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level, exec_shadow);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level, exec_shadow);

  if (proc_table[exec_shadow].task_level != local_scheduler_level) {
    kern_frestore(f);
      return FSF_ERR_BAD_ARGUMENT; 
  }
 
  switch (scheduler_id) {
    case FSF_RR:
      budget = POSIXSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_EDF:
      budget = EDFSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_FP:
      budget = RMSTAR_getbudget(local_scheduler_level, exec_shadow);
   case FSF_NONE:
      budget = NONESTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    default:
      budget = -1;
      break;
  }

  if (budget == -1) { 
    kern_frestore(f);
    return FSF_ERR_BAD_ARGUMENT; 

  }

  if (next_budget != NULL && next_period != NULL) {

    SERVER_getbudgetinfo(fsf_server_level, &Q, &T, &D, budget);
 
    #ifdef FSF_DEBUG
      kern_printf("(budget %d Q=%d T=%d)",budget,(int)Q,(int)T);
    #endif

    next_budget->tv_sec = Q / 1000000;
    next_budget->tv_nsec = (Q % 1000000) * 1000;
    next_period->tv_sec = T / 1000000;
    next_period->tv_nsec = (T % 1000000) * 1000;

  }

  if (was_deadline_missed != NULL) 
    *was_deadline_missed = false;
  if (was_budget_overran != NULL) 
    *was_budget_overran = false; 
  
  if (htable[index].id==-1) {
    kern_frestore(f);
    return FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }
  
  if (htable[index].events > 0) {
    task_activate(exec_shadow);
    htable[index].events--;
  } else {
    fsf_get_server(exec_shadow, &server);
    fsf_settask_nopreemptive(&server, exec_shadow);
    iq_insertlast(exec_shadow,&(htable[index].threads));
  }

  #ifdef FSF_DEBUG
    kern_printf("(Synch_Handle Events %d)",synch_handle->events);
  #endif

  kern_frestore(f);

  task_endcycle();
  return 0;

}

 int fsf_timed_schedule_triggered_job(fsf_synch_obj_handle_t   	 synch_handle,
				      const struct timespec *  	abs_timeout,
				      bool *  	timed_out,
				      struct timespec *  	next_budget,
				      struct timespec *  	next_period,
				      bool *  	was_deadline_missed,
				      bool *  	was_budget_overran) {
 TIME T,Q,D;
  int index=synch_handle;
  int budget, local_scheduler_level, scheduler_id;
  fsf_server_id_t server;

  SYS_FLAGS f;

  if (!abs_timeout && (abs_timeout->tv_sec < 0 || abs_timeout->tv_nsec > 1000000000))
     return FSF_ERR_BAD_ARGUMENT;

  f=kern_fsave();
  
  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level, exec_shadow);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level, exec_shadow);

  if (proc_table[exec_shadow].task_level != local_scheduler_level) {
    kern_frestore(f);
    return 0; 
  }
 
  switch (scheduler_id) {
    case FSF_RR:
      budget = POSIXSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_EDF:
      budget = EDFSTAR_getbudget(local_scheduler_level, exec_shadow);
      break;
    case FSF_FP:
      budget = RMSTAR_getbudget(local_scheduler_level, exec_shadow);
    default:
      budget = -1;
      break;
  }

  if (budget == -1) { 
    kern_frestore(f);
    return FSF_ERR_BAD_ARGUMENT; 

  }

  if (next_budget != NULL && next_period != NULL) {

    SERVER_getbudgetinfo(fsf_server_level, &Q, &T, &D, budget);
 
    #ifdef FSF_DEBUG
      kern_printf("(budget %d Q=%d T=%d)",budget,(int)Q,(int)T);
    #endif

    next_budget->tv_sec = Q / 1000000;
    next_budget->tv_nsec = (Q % 1000000) * 1000;
    next_period->tv_sec = T / 1000000;
    next_period->tv_nsec = (T % 1000000) * 1000;

  }

  if (was_deadline_missed != NULL) 
    *was_deadline_missed = false;
  if (was_budget_overran != NULL) 
    *was_budget_overran = false; 
  

  if (htable[index].id==-1) {
    kern_frestore(f);
    return FSF_ERR_INVALID_SYNCH_OBJ_HANDLE;
  }


  if (htable[index].events > 0) {
    task_activate(exec_shadow);
    htable[index].events--;
  } else {
    fsf_get_server(exec_shadow, &server);
    fsf_settask_nopreemptive(&server, exec_shadow);
    iq_insertlast(exec_shadow,&(htable[index].threads));
    //kern_event_post(ats_timeout, (void (*)(void *))restart_task, (void *)(exec_shadow));
  }

  #ifdef FSF_DEBUG
    kern_printf("(Synch_Handle Events %d)",synch_handle->events);
  #endif

  kern_frestore(f);

  task_endcycle();
  return 0;

};
