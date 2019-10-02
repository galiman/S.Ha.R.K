
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

#include "ll/i386/64bit.h"
#include <kernel/kern.h>
#include <sem/sem/sem.h>
#include <hartport/hartport/hartport.h>
#include <cabs/cabs/cabs.h>

#include "fsf.h"
#include "fsf_service_task.h"
#include "fsf_server.h"
#include "message.h"

#include "posixstar.h"
#include "edfstar.h"
#include "nonestar.h"
#include "rmstar.h"
#include "tdstar.h"
#include "fedfstar.h"

#include <pthread.h>
#include <stdlib.h>
#include "pistar.h"
#include <posix/posix/comm_message.h>

//#define FSF_DEBUG

int current_server=0;
server_elem server_list[FSF_MAX_N_SERVERS];
bandwidth_t fsf_max_bw = 0;
 
int fsf_server_level;
int fsf_posix_level = -1;
int shared_object_level;
PID server_task;
fsf_contract_parameters_t contract;


PORT channel[2];

fsf_server_id_t service_server = -1;
fsf_contract_parameters_t service_contract;

bandwidth_t SERVER_return_bandwidth();

int FSF_register_shared_object_module(void) {
  fsf_register_shared_object();
  return PISTAR_register_module();
}


int FSF_get_shared_object_level() {
  return shared_object_level;
}

int FSF_register_module(int posix_level, int server_level, bandwidth_t max_bw)
{
  printk("FSF Module\n");
  current_server=0;
  fsf_server_level = server_level;
  fsf_posix_level = posix_level;
  fsf_max_bw = max_bw;
  shared_object_level = FSF_register_shared_object_module();
  
  return 0;

}

void FSF_start_service_task(void) {

  int err;
  struct timespec default_period = FSF_SERVICE_THREAD_PERIOD;
  struct timespec default_budget = FSF_SERVICE_THREAD_BUDGET;
  DUMMY_TASK_MODEL m;
  fsf_sched_params_t pr;
  int                budget_overrun_sig_notify = FSF_NULL_SIGNAL;
  union sigval       budget_overrun_sig_value = {0};
  int                deadline_miss_sig_notify = FSF_NULL_SIGNAL; 
  union sigval       deadline_miss_sig_value = {0};

  pr.policy=FSF_NONE;
  pr.params=&m;

  dummy_task_default_model(m);
 
  // create the service task
  // create the communication channel for negotiation and renegotiation
  
  channel[1] = port_create("CHANW",sizeof(struct mess),1,STREAM,WRITE);
  if (channel[1]<0) exit(-1);

  channel[0] = port_create("CHANR",sizeof(struct mess),1,STREAM,READ);
  if (channel[0]<0) exit(-1);

  //kern_printf("FSF port WRITE %d, READ %d\n", channel[1], channel[0]);

  fsf_initialize_contract(&service_contract);
  
  err=fsf_set_contract_basic_parameters(&service_contract,&default_budget,&default_period,FSF_DEFAULT_WORKLOAD); 
  if (err) exit(err);

  err=fsf_set_contract_timing_requirements (&service_contract,
					true,NULL,
					budget_overrun_sig_notify,
					budget_overrun_sig_value,
					deadline_miss_sig_notify,
					deadline_miss_sig_value);
  if (err) exit(err);

  negotiate_contract(&service_contract,&service_server);

  //server_task = task_create("stask",service_task,model,NULL);
  err = fsf_create_local_thread(service_server,&pr, &server_task,NULL,(fsf_thread_code_t)service_task,NULL);
  if (err) {
    cprintf("error creating service task\n");
    sys_shutdown_message("Could not create service_task");
    exit(1);
  }

  task_activate(server_task);

}


/* Convert the contract specification to
 * budget parameters
 */
int set_SERVER_budget_from_contract
  (const fsf_contract_parameters_t *contract,
   int *budget)
{

  int local_scheduler_level = 0;

   switch (contract->policy) {
     case FSF_RR:
       local_scheduler_level = POSIXSTAR_register_level(fsf_server_level,5000,32);
       break;
     case FSF_EDF:
       local_scheduler_level = EDFSTAR_register_level(fsf_server_level);
       break;
     case FSF_FEDF:
       local_scheduler_level = FEDFSTAR_register_level(fsf_server_level);
       break;
     case FSF_FP:
       local_scheduler_level = RMSTAR_register_level(fsf_server_level);
       break;
     case FSF_NONE:
       local_scheduler_level = NONESTAR_register_level(fsf_server_level);
       break;
     case FSF_TABLE_DRIVEN:
       kern_printf("Register level");
       local_scheduler_level = TDSTAR_register_level(fsf_server_level);
       break;
   }    

   if (contract->d_equals_t == true) {
  *budget = SERVER_setbudget(fsf_server_level,
			      TIMESPEC2USEC(&(contract->budget_min)),
                              TIMESPEC2USEC(&(contract->period_max)),
			      TIMESPEC2USEC(&(contract->period_max)),
                              local_scheduler_level,contract->policy);
  } else {
  *budget = SERVER_setbudget(fsf_server_level,
                              TIMESPEC2USEC(&(contract->budget_min)),
                              TIMESPEC2USEC(&(contract->period_max)),
                              TIMESPEC2USEC(&(contract->deadline)),
                              local_scheduler_level,contract->policy);
  }

  return 0;

}

int adjust_SERVER_budget
   (int budget, const TIME budget_actual, 
    const TIME period_actual, const TIME dline_actual)
{

  SERVER_adjust_budget(fsf_server_level, 
                       budget_actual,
                       period_actual,
		       dline_actual,
                       budget);

  return 0;

}

/* Admission Test function */
int add_contract(const fsf_contract_parameters_t *contract)
{
  bandwidth_t current_bandwidth,U;
  
  TIME T,Q;
  	                  
#ifdef FSF_DEBUG
  kern_printf("(GA TEST)");
#endif                                                                     
  T=TIMESPEC2USEC(&(contract->period_max));
  Q=TIMESPEC2USEC(&(contract->budget_min));
		                                                                                  
  mul32div32to32(MAX_BANDWIDTH,Q,T,U);
   /* The current bandwidth is the min bandwidth */
  current_bandwidth=SERVER_return_bandwidth(fsf_server_level);

  if (fsf_max_bw<current_bandwidth+U) return -1;

  return 0;

}

void contract_to_server(const fsf_contract_parameters_t *contract, int i) {

  TIME T,Q;
#ifdef FSF_DEBUG
  int temp;
#endif

  T=TIMESPEC2USEC(&contract->period_min);
  Q=TIMESPEC2USEC(&contract->budget_max);
  server_list[i].Cmax=Q;

  mul32div32to32(MAX_BANDWIDTH,Q,T,server_list[current_server].Umax);
	                                                                                  
  T=TIMESPEC2USEC(&contract->period_min);
  server_list[i].Tmin=T;
	                                                                                      
  T=TIMESPEC2USEC(&contract->period_max);
  server_list[i].Tmax=T;

  Q=TIMESPEC2USEC(&contract->budget_min);
  server_list[i].Cmin=Q;
		                                                                                  
  mul32div32to32(MAX_BANDWIDTH,Q,T,server_list[i].Umin);
  server_list[i].U=server_list[i].Umin;

  if (contract->d_equals_t == true) {
    server_list[i].deadline = 0;
    server_list[i].d_equals_t = true;
  } else {
    server_list[i].deadline = TIMESPEC2USEC(&contract->deadline);;
    server_list[i].d_equals_t = false;
  }
                                               
  server_list[i].Qs = contract->quality;
  server_list[i].Is = contract->importance;

#ifdef FSF_DEBUG
  mul32div32to32(server_list[i].Umax,100, MAX_BANDWIDTH, temp);
  kern_printf("(Umax %d)",temp);
  mul32div32to32(server_list[i].Umin,100, MAX_BANDWIDTH, temp);
  kern_printf("(Umin %d)",temp);
#endif

}

int relink_contract_to_server(const fsf_contract_parameters_t *contract,
		              fsf_server_id_t server)
{
  int i=0;
#ifdef FSF_DEBUG
  kern_printf("(Relink Server %d)",server);
#endif
  // find contract
  while(i<current_server) {
    if (server_list[i].server==server) break;
    i++;
  }

  server_list[i].server=server;
//  server_list[i].Qs=1;
   
  contract_to_server(contract, i);


 return 0; 
 
}

int link_contract_to_server(const fsf_contract_parameters_t *contract,
                            fsf_server_id_t server)
{
#ifdef FSF_DEBUG
  kern_printf("(Link Server %d)",server);
#endif
  
  server_list[current_server].server=server;
  //server_list[current_server].Qs=1;
  
  contract_to_server(contract,current_server);    
  
  current_server++;
  return 0;

}

int remove_contract(fsf_server_id_t server)
{
  int i=0;
  // find the contract
  while(i<current_server) {
     if (server_list[i].server==server) break;
     i++;
  }

  // compress the array;
  while (i<(current_server-1)) {
     server_list[i].server=server_list[i+1].server;
     server_list[i].Umin=server_list[i+1].Umin;
     server_list[i].U=server_list[i+1].Umin;     
     server_list[i].Umax=server_list[i+1].Umax;
     server_list[i].Cmin=server_list[i+1].Cmin;
     server_list[i].Cmax=server_list[i+1].Cmax;
     server_list[i].Tmin=server_list[i+1].Tmin;
     server_list[i].Tmax=server_list[i+1].Tmax;
     server_list[i].Qs=server_list[i+1].Qs;
     server_list[i].deadline = server_list[i+1].deadline;
     server_list[i].d_equals_t = server_list[i+1].d_equals_t;
     server_list[i].Is = server_list[i+1].Is;

     i++;
  }
  current_server--;


  return 0;

}

int
fsf_get_renegotiation_status
  (fsf_server_id_t server,
   fsf_renegotiation_status_t *renegotiation_status)
{

 if (SERVER_get_renegotiation_status(fsf_server_level,server))
   *renegotiation_status=FSF_IN_PROGRESS;
 else 
   *renegotiation_status=FSF_ADMITTED;

 return 0;


}

int
fsf_request_change_quality_and_importance
  (fsf_server_id_t server,
   int new_importance,
   int new_quality)
{
   struct mess m;

  // send response server is -1 if the operation fail
  m.type=CHANGE_PARAMETER;
  m.server=server;
  m.qi.quality = new_quality;
  m.qi.importance =  new_importance;
  //memmove(&m.contract,contract, sizeof(fsf_contract_parameters_t));

  port_send(channel[1],&m,BLOCK);
  
  port_receive(channel[0], &m, BLOCK);
  
  if (m.server==-1) 
    return FSF_ERR_CONTRACT_REJECTED;

  //*server=m.server;

  return 0;
}


void copy_contract(fsf_contract_parameters_t *contractd, const fsf_contract_parameters_t *contracts) {

  contractd->budget_min=contracts->budget_min;               
  contractd->period_max=contracts->period_max; 
  contractd->budget_max=contracts->budget_max;
  contractd->period_min=contracts->period_min;
  contractd->workload=contracts->workload;
  contractd->d_equals_t=contracts->d_equals_t;
  contractd->deadline=contracts->deadline;
  contractd->quality=contracts->quality;
  contractd->policy=contracts->policy;
  /*
  int                     budget_overrun_sig_notify; 
  union sigval            budget_overrun_sig_value;  
  int                     deadline_miss_sig_notify;  
  union sigval            deadline_miss_sig_value;   
                                                     
  fsf_granularity_t       granularity;               
  fsf_utilization_set_t   utilization_set;           
  int                     quality;                   
  int                     importance;                
                                                     
  fsf_preemption_level_t  preemption_level;          
  fsf_critical_sections_t critical_sections;         
                                                     
  fsf_sched_policy_t      policy;                    
                                                     
  fsf_network_id_t        network_id;                
  bool                    granted_capacity_flag;     
  */                                                 


}

int fsf_negotiate_contract
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t                 *server)
{
  struct mess m;

  // send response server is -1 if the operation fail
  m.type=NEGOTIATE_CONTRACT;
  copy_contract(&m.contract,contract);
  port_send(channel[1],&m,BLOCK);
  port_receive(channel[0], &m, BLOCK);
  if (m.server==-1) {
    *server=0;
    return FSF_ERR_CONTRACT_REJECTED;
  }

  *server=m.server;
  
  return 0;

}

int
fsf_negotiate_contract_for_new_thread
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t      *server,
   pthread_t            *thread,
   pthread_attr_t       *attr,
   fsf_thread_code_t     thread_code,
   void                 *arg) {

  int err=0;

  err = fsf_negotiate_contract(contract,server);
  if (!err) {    
    err = pthread_create(thread, attr, thread_code, arg);    
    if (!err) 
      err = fsf_bind_thread_to_server(*server,*thread);
  } else return err;

  return err;
}

int
fsf_negotiate_contract_for_myself
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t      *server) {

  int err=0;

  err = fsf_negotiate_contract(contract,server);
  if (!err) { 
#ifdef FSF_DEBUG
    kern_printf("Bind task");
#endif
     err = fsf_bind_thread_to_server(*server,exec_shadow);

  } else return err;

  return err;
}

int fsf_unbind_thread_from_server
  (pthread_t       thread)
{

  int local_scheduler_level, scheduler_id;
  SYS_FLAGS f;
  /* Move thread from the local scheduler module to posix level */

  #ifdef FSF_DEBUG
    kern_printf("(UnBind thread = %d)",thread);
  #endif
  f=kern_fsave();
  /* Check if thread exsists */
  if (thread == -1) {
    kern_frestore(f);
    return FSF_ERR_BAD_ARGUMENT;
  }

  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level,thread);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level,thread);
  
  /* Check if thread is already bind */
  if (scheduler_id == FSF_NONE) {
      /* Check if it is bind to a server */
    if (NONESTAR_getbudget(local_scheduler_level,thread) == -1) {
      kern_frestore(f);
        return FSF_ERR_BAD_ARGUMENT;
      }
      else {
        
        STD_command_message *msg;
        NRT_TASK_MODEL nrt;

        nrt_task_default_model(nrt);
        nrt_task_def_save_arrivals(nrt);
        nrt_task_def_weight(nrt,0);
        nrt_task_def_policy(nrt,NRT_RR_POLICY);
        nrt_task_def_inherit(nrt,NRT_EXPLICIT_SCHED);

        /* Send change level command to local scheduler */
        msg = (STD_command_message *)malloc(sizeof(STD_command_message));

        msg->command = STD_SET_NEW_MODEL;
        msg->param = (void *)(&nrt);
        level_table[fsf_posix_level]->public_message(fsf_posix_level,thread,msg);

        msg->command = STD_SET_NEW_LEVEL;
        msg->param = (void *)(fsf_posix_level);
        task_message(msg,thread,0);
	//if (exec_shadow!=thread) 
	level_table[proc_table[thread].task_level]->public_epilogue(proc_table[thread].task_level, thread);
	//else {
	//scheduler();
	//kern_context_load(proc_table[exec_shadow].context);
	//}
	kern_frestore(f);

        free(msg);
      }
  } else {
    kern_frestore(f);
    return FSF_ERR_BAD_ARGUMENT;
  }

  return 0;

}

int  
fsf_bind_local_thread_to_server
  (fsf_server_id_t      server,
   pthread_t            thread,
   fsf_sched_params_t  *sched_params) 
{

  STD_command_message *msg;
  int local_scheduler_level,scheduler_id;

  /* Move thread from the posix module to local scheduler */

  #ifdef FSF_DEBUG 
    kern_printf("(Bind thread = %d to Server = %d)",thread,server);
  #endif

  /* Check if server and thread exsist */
  if (server == -1 || thread == -1)
    return FSF_ERR_BAD_ARGUMENT;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_budget(fsf_server_level,server);
  if (local_scheduler_level==-1)
    return FSF_ERR_UNKNOWN_SCHEDULED_THREAD;

  scheduler_id = SERVER_get_local_scheduler_id_from_budget(fsf_server_level,server);
  
  if (scheduler_id!=sched_params->policy)
     return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;

  /* Check if thread is already bind */
  switch(scheduler_id) {
   
     case FSF_FP: 
       {
	 TASK_MODEL      *m=(TASK_MODEL*)(sched_params->params);
	 HARD_TASK_MODEL *h=(HARD_TASK_MODEL *)(sched_params->params);

	 if (m->pclass != HARD_PCLASS) 
	   return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;

	 h = (HARD_TASK_MODEL *)m;

	 if (!h->wcet || !h->mit) return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;
	 
	 /* now we know that m is a valid model */
	 if (RMSTAR_getbudget(local_scheduler_level,thread) != -1)
	   return FSF_ERR_BAD_ARGUMENT;
	 
	 /* Set server on local scheduler */
	 RMSTAR_setbudget(local_scheduler_level,thread,(int)(server));
	 
	 /* Send change level command to posix level */
       }
       break;

     case FSF_EDF: 
       {
	 TASK_MODEL      *m=(TASK_MODEL*)(sched_params->params);
	 HARD_TASK_MODEL *h=(HARD_TASK_MODEL *)(sched_params->params);

	 if (m->pclass != HARD_PCLASS)
	   return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;

	 h = (HARD_TASK_MODEL *)m;

	 if (!h->wcet || !h->mit) return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;
      
	 if (EDFSTAR_getbudget(local_scheduler_level,thread) != -1)
	   return FSF_ERR_BAD_ARGUMENT;
	 
	 /* Set server on local scheduler */
	 EDFSTAR_setbudget(local_scheduler_level,thread,(int)(server));
	 
       }
       break;
     
     case FSF_FEDF:
       {
         TASK_MODEL      *m=(TASK_MODEL*)(sched_params->params);
         HARD_TASK_MODEL *h=(HARD_TASK_MODEL *)(sched_params->params);
                                                                                
         if (m->pclass != HARD_PCLASS)
           return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;
                                                                                
         h = (HARD_TASK_MODEL *)m;
                                                                                
         if (!h->wcet || !h->mit) return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;
                                                                                
         if (FEDFSTAR_getbudget(local_scheduler_level,thread) != -1)
           return FSF_ERR_BAD_ARGUMENT;
                                                                                
         /* Set server on local scheduler */
         FEDFSTAR_setbudget(local_scheduler_level,thread,(int)(server));
                                                                                
       }

     case FSF_RR:
       {
	 TASK_MODEL      *m=(TASK_MODEL*)(sched_params->params);

	 if (m->pclass != NRT_PCLASS) 
	   return  FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;

	 if (POSIXSTAR_getbudget(local_scheduler_level,thread) != -1)
	   return FSF_ERR_BAD_ARGUMENT;
	 
	 /* Set server on local scheduler */
	 POSIXSTAR_setbudget(local_scheduler_level,thread,(int)(server));	 
       }
       break;

     default: 
       
       return FSF_ERR_BAD_ARGUMENT;
       
  }

  msg = (STD_command_message *)malloc(sizeof(STD_command_message));
  if (msg) {
    SYS_FLAGS f;
    f=kern_fsave(); 
    msg->command = STD_SET_NEW_MODEL;
    msg->param = (void *)(sched_params->params);
    level_table[local_scheduler_level]->public_message(local_scheduler_level,thread,msg);
    
    msg->command = STD_SET_NEW_LEVEL;
    msg->param = (void *)(local_scheduler_level);
    task_message(msg,thread,0);
    level_table[proc_table[thread].task_level]->public_dispatch(proc_table[thread].task_level, thread, 0);
    level_table[proc_table[thread].task_level]->public_epilogue(proc_table[thread].task_level, thread);
    //} else {
    if (cap_timer != NIL) {
      event_delete(cap_timer);
      cap_timer = NIL;
    }
    
    scheduler();
    kern_context_load(proc_table[exec_shadow].context);
    
    kern_frestore(f);
    free(msg);
  } else return FSF_ERR_INTERNAL_ERROR;
  
  return 0;

}


int fsf_bind_thread_to_server
  (fsf_server_id_t server,
   pthread_t       thread)
{

  STD_command_message *msg=0;
  int local_scheduler_level,scheduler_id;
  SYS_FLAGS f;
  /* Move thread from the posix module to local scheduler */

  f=kern_fsave();
  #ifdef FSF_DEBUG 
    kern_printf("(Bthr=%d to Sr=%d)",thread,server);
  #endif

  /* Check if server and thread exsist */
  if (server == -1 || thread == -1)
    return FSF_ERR_BAD_ARGUMENT;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_budget(fsf_server_level,server);
  if (local_scheduler_level==-1)
    return FSF_ERR_UNKNOWN_SCHEDULED_THREAD;

  scheduler_id = SERVER_get_local_scheduler_id_from_budget(fsf_server_level,server);
  /* Check if thread is already bind */
  if (scheduler_id == FSF_NONE) {
      DUMMY_TASK_MODEL rt_arg;
      if (NONESTAR_getbudget(local_scheduler_level,thread) != -1) {
	kern_frestore(f);
        return FSF_ERR_BAD_ARGUMENT;
      }
      /* Set server on local scheduler */
      NONESTAR_setbudget(local_scheduler_level,thread,(int)(server));
     
      /* Send change level command to posix level */
      msg = (STD_command_message *)malloc(sizeof(STD_command_message));
      if (!msg) exit(-1);      
      msg->command = STD_SET_NEW_MODEL;
      msg->param = (void *)(&rt_arg);
      level_table[local_scheduler_level]->public_message(local_scheduler_level,thread,msg);      
      msg->command = STD_SET_NEW_LEVEL;
      msg->param = (void *)(local_scheduler_level);
      task_message(msg,thread,0);      
      //if (thread!=exec_shadow) {
      	  level_table[proc_table[thread].task_level]->public_dispatch(proc_table[thread].task_level, thread, 0);
          level_table[proc_table[thread].task_level]->public_epilogue(proc_table[thread].task_level, thread);
      //} else {
      if (cap_timer != NIL) {
        event_delete(cap_timer);
        cap_timer = NIL;
      }
 
      scheduler();
      kern_context_load(proc_table[exec_shadow].context);
      //}
      kern_frestore(f);
 

      if (msg) free(msg);
   
   } else { 
       kern_frestore(f);
       return FSF_ERR_BAD_ARGUMENT;
   }
  return 0;
}

int fsf_create_local_thread
(fsf_server_id_t        server,
   fsf_sched_params_t    *local_scheduler_arg,
   pthread_t             *thread,
   pthread_attr_t        *attr,
   fsf_thread_code_t      thread_code,
   void                  *arg)
{

  int local_scheduler_level,scheduler_id;

  /* Check if server and thread exsist */
  if (server == NIL)
    return  FSF_ERR_BAD_ARGUMENT;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_budget(fsf_server_level,server);
  scheduler_id = SERVER_get_local_scheduler_id_from_budget(fsf_server_level,server);
#ifdef FSF_DEBUG
  kern_printf("sched policy %d", scheduler_id);
#endif

  if (scheduler_id!=local_scheduler_arg->policy)
    return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;

  /* Check if thread is already bind */
  switch (scheduler_id) {
     case FSF_RR:

      nrt_task_def_arg(*(NRT_TASK_MODEL *)(local_scheduler_arg->params),arg);
      nrt_task_def_level(*(NRT_TASK_MODEL *)(local_scheduler_arg->params),local_scheduler_level);

      *thread = task_create("POSIXSTAR", thread_code, local_scheduler_arg->params, NULL);
      if (*thread == NIL) {
        #ifdef FSF_DEBUG
          kern_printf("(FSF:Error creating thread)");
        #endif
        return FSF_ERR_INTERNAL_ERROR;
      }

      POSIXSTAR_setbudget(local_scheduler_level, *thread, (int)(server));

    break;
    case FSF_EDF:

      hard_task_def_arg(*(HARD_TASK_MODEL *)(local_scheduler_arg->params),arg);
      hard_task_def_level(*(HARD_TASK_MODEL *)(local_scheduler_arg->params),local_scheduler_level);

      *thread = task_create("EDFSTAR", thread_code, local_scheduler_arg->params, NULL);
      if (*thread == NIL)
        return  FSF_ERR_INTERNAL_ERROR;

      EDFSTAR_setbudget(local_scheduler_level, *thread, (int)(server));

      break;

    case FSF_FEDF:
      hard_task_def_arg(*(HARD_TASK_MODEL *)(local_scheduler_arg->params),arg);
      hard_task_def_level(*(HARD_TASK_MODEL *)(local_scheduler_arg->params),local_scheduler_level);
                                                                                
      *thread = task_create("FEDFSTAR", thread_code, local_scheduler_arg->params, NULL);
      if (*thread == NIL)
        return  FSF_ERR_INTERNAL_ERROR;
                                                                                
      FEDFSTAR_setbudget(local_scheduler_level, *thread, (int)(server));
                                                                                
      break;

    case FSF_FP:

      hard_task_def_arg(*(HARD_TASK_MODEL *)(local_scheduler_arg->params),arg);
      hard_task_def_level(*(HARD_TASK_MODEL *)(local_scheduler_arg->params),local_scheduler_level);

      *thread = task_create("RMSTAR", thread_code, local_scheduler_arg->params, NULL);
      if (*thread == NIL)
        return  FSF_ERR_INTERNAL_ERROR;

      RMSTAR_setbudget(local_scheduler_level, *thread, (int)(server));

      break;
    case FSF_NONE:
                                                                                                                             
      dummy_task_def_arg(*( DUMMY_TASK_MODEL *)(local_scheduler_arg->params),arg);
      dummy_task_def_level(*( DUMMY_TASK_MODEL *)(local_scheduler_arg->params),local_scheduler_level);
                                                                                                                      
      *thread = task_create("NONESTAR", thread_code, local_scheduler_arg->params, NULL);
      if (*thread == NIL)
        return  FSF_ERR_INTERNAL_ERROR;
                                                                                                                             
      NONESTAR_setbudget(local_scheduler_level, *thread, (int)(server));
                                                                                                                             
      break;

    case FSF_TABLE_DRIVEN:
      {
	HARD_TASK_MODEL ht;
	hard_task_default_model(ht);
	hard_task_def_aperiodic(ht);
      
	hard_task_def_arg(ht,arg);
	hard_task_def_level(ht,local_scheduler_level);
	
	*thread = task_create("TDSTAR", thread_code, &ht, NULL);
	if (*thread == NIL)
	  return  FSF_ERR_INTERNAL_ERROR;
	
	TDSTAR_setbudget(local_scheduler_level, *thread, (int)(server));
	TDSTAR_settable(local_scheduler_level, (fsf_table_driven_params_t *)(local_scheduler_arg->params),*thread);
      }
      break;

 
    default:
      return FSF_ERR_INTERNAL_ERROR;
      break;
  }
  
  #ifdef FSF_DEBUG
    kern_printf("(FSF:Insert thread = %d to Server = %d)",*thread,server);
  #endif

  return 0;
  
}

int  fsf_settask_nopreemptive
  (fsf_server_id_t *server,
   pthread_t       thread)
{
  int local_scheduler_level, scheduler_id;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level,thread);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level, thread);

  switch (scheduler_id) {
    case FSF_RR:
      POSIXSTAR_set_nopreemtive_current(local_scheduler_level);
      return 1;
      break;
    case FSF_EDF:
      EDFSTAR_set_nopreemtive_current(local_scheduler_level);
      return 1;
      break;
    case FSF_FEDF:
      FEDFSTAR_set_nopreemtive_current(local_scheduler_level);
      return 1;
      break;

    case FSF_FP:
      RMSTAR_set_nopreemtive_current(local_scheduler_level);
      return 1;
      break;
    case FSF_NONE:
      break;
    default:
      return -1;
  }
  return -1;
}


int  fsf_settask_preemptive
  (fsf_server_id_t *server,
   pthread_t       thread)
{
  int local_scheduler_level, scheduler_id;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level,thread);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level, thread);

  switch (scheduler_id) {
    case FSF_RR:
      POSIXSTAR_unset_nopreemtive_current(local_scheduler_level);
      return 1;
      break;
    case FSF_EDF:
      EDFSTAR_unset_nopreemtive_current(local_scheduler_level);
      return 1;
      break;

    case FSF_FEDF:
      EDFSTAR_unset_nopreemtive_current(local_scheduler_level);
      return 1;
      break;

    case FSF_FP:
      RMSTAR_unset_nopreemtive_current(local_scheduler_level);
      return 1;
      break;
    case FSF_NONE:
      break;
    default:
      return -1;
  }

  return -1;

}


int fsf_get_server
  (pthread_t       thread,
   fsf_server_id_t *server)
{
  int local_scheduler_level, scheduler_id;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_pid(fsf_server_level,thread);
  scheduler_id = SERVER_get_local_scheduler_id_from_pid(fsf_server_level, thread);
  
  switch (scheduler_id) {
    case FSF_RR:  
      *server = POSIXSTAR_getbudget(local_scheduler_level,thread);
      return 0;
    case FSF_EDF:
      *server = EDFSTAR_getbudget(local_scheduler_level,thread);
      return 0;
   case FSF_FEDF:
      *server = FEDFSTAR_getbudget(local_scheduler_level,thread);
      return 0;

    case FSF_FP:
      *server = RMSTAR_getbudget(local_scheduler_level,thread);
      return 0;
    case FSF_NONE:
      *server = NONESTAR_getbudget(local_scheduler_level,thread);
      return 0;
    default:
      return -1;
  }

  return -1;

}

int fsf_get_server_level(void)
{

  return fsf_server_level;

}

int fsf_cancel_contract
  (fsf_server_id_t server)
{

  int local_scheduler_level, scheduler_id;
  SYS_FLAGS f;
  TIME T,Q;
  int i=0,err=0;


  #ifdef FSF_DEBUG
    kern_printf("(Remove server %d)",server);
  #endif

  /* Check server id */
  if (server < 0)
    return FSF_ERR_BAD_ARGUMENT;

  local_scheduler_level = SERVER_get_local_scheduler_level_from_budget(fsf_server_level,server);
  scheduler_id = SERVER_get_local_scheduler_id_from_budget(fsf_server_level,server);

  switch (scheduler_id) {
    case FSF_RR:
  
      /* Check if some thread use the server */
      if(POSIXSTAR_budget_has_thread(local_scheduler_level,server))
        return FSF_ERR_NOT_CONTRACTED_SERVER;
 
      break;
    case FSF_EDF:
      /* Check if some thread use the server */
      if(EDFSTAR_budget_has_thread(local_scheduler_level,server))
        return FSF_ERR_NOT_CONTRACTED_SERVER;
      break;

    case FSF_FEDF:
      /* Check if some thread use the server */
      if(FEDFSTAR_budget_has_thread(local_scheduler_level,server))
        return FSF_ERR_NOT_CONTRACTED_SERVER;
      break;

    case FSF_FP:
      /* Check if some thread use the server */
      if(RMSTAR_budget_has_thread(local_scheduler_level,server))
        return FSF_ERR_NOT_CONTRACTED_SERVER;

      break;

    case FSF_NONE:
      /* Check if some thread use the server */
      if(NONESTAR_budget_has_thread(local_scheduler_level,server)) {
        err=fsf_unbind_thread_from_server(NONESTAR_get_current(local_scheduler_level));
	if (err) return err;
      }
                                                                                                                             
      break;

  }
 
  SERVER_removebudget(fsf_server_level,server);

  level_free_descriptor(local_scheduler_level); 
	
  remove_contract(server);
                                              
  f=kern_fsave();               
  if (recalculate_contract(fsf_max_bw)==-1)  {
       kern_frestore(f);
       return  FSF_ERR_INTERNAL_ERROR;
  }
#ifdef  FSF_DEBUG
  kern_printf("(Adjust budget)");
#endif    
  for (i=0; i<current_server; i++) {
    
    mul32div32to32(MAX_BANDWIDTH,server_list[i].Cmin,server_list[i].U,T);
   
    if (T > server_list[i].Tmin ) {
      server_list[i].actual_budget = server_list[i].Cmin;

      if (T > server_list[i].Tmax)
	T=server_list[i].Tmax;

      server_list[i].actual_period = T;
      server_list[i].actual_budget = server_list[i].Cmin;
     
      #ifdef FSF_DEBUG
        kern_printf("(1 - Q %ld T %ld)", server_list[i].actual_budget, server_list[i].actual_period);
      #endif

      if (server_list[i].d_equals_t == true)
	adjust_SERVER_budget(server_list[i].server,server_list[i].Cmin, T, T);
      else
	adjust_SERVER_budget(server_list[i].server,server_list[i].Cmin, T, T); // server_list[i].deadline);
     
    } else {
     
      mul32div32to32(server_list[i].Tmin,server_list[i].U,MAX_BANDWIDTH,Q);
      

      if (Q>server_list[i].Cmax)
	Q=server_list[i].Cmax;

      server_list[i].actual_budget = Q;
      server_list[i].actual_period = server_list[i].Tmin;
      #ifdef FSF_DEBUG
         kern_printf("(2 - Q %ld T %ld)", server_list[i].actual_budget, server_list[i].actual_period);
      #endif
     
      if (server_list[i].d_equals_t == true)
	adjust_SERVER_budget(server_list[i].server,Q, server_list[i].Tmin, server_list[i].Tmin);
      else
	adjust_SERVER_budget(server_list[i].server,Q, server_list[i].Tmin, server_list[i].Tmin); //server_list[i].deadline);  


    }
    server_list[i].U=server_list[i].Umin;
    
  }                                           


  kern_frestore(f);

  return 0;
  
}

bandwidth_t SERVER_return_bandwidth() {
  int i=0;
  bandwidth_t U;
  U=0;
  for(i=0;i<current_server;i++) {

    U+=server_list[i].Umin;

  }

  return U;
}

int recalculate_contract(bandwidth_t U) {
  bandwidth_t current_bandwidth;
  unsigned int temp_U;
  int        Qt;
  int isok=0;
  int i=0;
  int target_importance=FSF_DEFAULT_IMPORTANCE;

#define MAX_IMPORTANCE 5
  
#ifdef FSF_DEBUG
  int temp;
    
  kern_printf("(RC)");
#endif

  /* The current bandwidth is the min bandwidth */
  current_bandwidth=SERVER_return_bandwidth(fsf_server_level);
  #ifdef FSF_DEBUG
     kern_printf("(SER%d)", current_server);
  #endif  
  //kern_printf("(CS:%d)", current_server); 
  do  {
    current_bandwidth=0;
    Qt=0;
    for (i=0; i<current_server; i++) {
      if (server_list[i].Is==target_importance 
	  && server_list[i].U<server_list[i].Umax && server_list[i].Qs>0)
         Qt+=server_list[i].Qs; 
       current_bandwidth+=server_list[i].U;
#ifdef FSF_DEBUG
       kern_printf("(Qs %d, Qt %d, Is %d)", server_list[i].Qs, Qt,server_list[i].Is);
#endif
    }

#ifdef FSF_DEBUG
    kern_printf("(TQ%d)", Qt);
#endif
    isok=1;
    for (i=0; i<current_server; i++) {
      if (server_list[i].Is==target_importance && server_list[i].U<server_list[i].Umax && server_list[i].Qs>0) {
	temp_U=server_list[i].U;
        server_list[i].U=U-current_bandwidth;
	//kern_printf("before mull");
	mul32div32to32(server_list[i].U, server_list[i].Qs, Qt, server_list[i].U);
	//kern_printf("after mull");
	temp_U+=server_list[i].U;
	
        if (temp_U<=server_list[i].Umin) {
	   server_list[i].U=server_list[i].Umin;
	} else if (temp_U>server_list[i].Umax)  {
	   server_list[i].U=server_list[i].Umax;
	   isok=0;
	} else server_list[i].U=temp_U; 
 	
#ifdef FSF_DEBUG
	mul32div32to32(server_list[i].U,100, MAX_BANDWIDTH, temp);
        kern_printf("(SER %d BW %d)", server_list[i].server, temp);
#endif 
      } 
    }  
    target_importance++;
  } while (!isok || target_importance<=MAX_IMPORTANCE);

 return 0;
}
	

int fsf_negotiate_group
   (const fsf_contracts_group_t *contracts_up,
    const fsf_servers_group_t   *severs_down,
    fsf_servers_group_t         *severs_up,
    bool                        *accepted) {

  return 0;
}

int fsf_renegotiate_contract
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t server)
{
 
  struct mess msg;

  // send response server is -1 if the operation fail
  msg.type=RENEGOTIATE_CONTRACT;
  copy_contract(&msg.contract,new_contract);
  msg.server = server;
  //kern_printf("(REN %d)", server);
  port_send(channel[1],&msg,BLOCK);
  //kern_printf("After send\n");
  port_receive(channel[0], &msg, BLOCK);
  //kern_printf("After receive\n");
  //kern_printf("(EREN %d)", msg.server);
  if (msg.server==-1) return FSF_ERR_CONTRACT_REJECTED;
  
   return 0;
}

int fsf_request_contract_renegotiation
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t                  server,
   int                              sig_notify,
   union sigval                     sig_value)
{
 
  struct mess msg;

  // send response server is -1 if the operation fail
  msg.type=REQUEST_RENEGOTIATE_CONTRACT;
  copy_contract(&msg.contract,new_contract);
  msg.server = server;
  msg.sig_notify=sig_notify;
  msg.sig_value=sig_value;
  msg.process=exec_shadow;

  port_send(channel[1],&msg,BLOCK);

  port_receive(channel[0], &msg, BLOCK);

  if (msg.server==-1) return FSF_ERR_CONTRACT_REJECTED;
  
   return 0;
}

void print_server_list()
{

  int i;
  
  kern_printf("Server List\n");
 
  for(i=0;i<current_server;i++) {

    kern_printf("[%d] Q:%d T:%d D:%d [DeT = %d]\n",server_list[i].server,(int)server_list[i].actual_budget,(int)server_list[i].actual_period,(int)server_list[i].deadline,(int)server_list[i].d_equals_t);

  }

}

int
fsf_get_remaining_budget
   (fsf_server_id_t server,
    struct timespec *budget)  {
  TIME t;
  NULL_TIMESPEC(budget);
  t=SERVER_get_remain_capacity(fsf_server_level, server);
  ADDUSEC2TIMESPEC(t, budget);
  return 0;
 
  
}
/*
int fsf_get_remaining_budget(fsf_server_id_t server) {
  
  return SERVER_get_remain_capacity(fsf_server_level, server);
}
*/
int fsf_get_budget_and_period
   (fsf_server_id_t server,
    struct timespec *budget,
    struct timespec *period) {
  TIME bg;
  TIME pd;

  if (!SERVER_getbudgetinfo(fsf_server_level, &bg, &pd, NULL, server)) {
    if (budget) {
      NULL_TIMESPEC(budget);
      ADDUSEC2TIMESPEC(bg, budget);
    }
    if (period) {
      NULL_TIMESPEC(period);
      ADDUSEC2TIMESPEC(pd, period);
    }
    
    return 0;
  }
  return FSF_ERR_BAD_ARGUMENT;
}

int
fsf_set_service_thread_data
   (const struct timespec *budget,
    const struct timespec *period,
    bool                  *accepted) {

  if (budget==NULL && period==NULL) return FSF_ERR_BAD_ARGUMENT;
  fsf_set_contract_basic_parameters(&service_contract,budget,period,FSF_DEFAULT_WORKLOAD); 
  *accepted = !fsf_renegotiate_contract(&service_contract,service_server)?true:false;
  return 0;

}



int fsf_get_service_thread_data
   (struct timespec *budget,
    struct timespec *period) {

  return fsf_get_budget_and_period(service_server, budget, period);

}

int fsf_init_local_scheduler(fsf_server_id_t server,
			     fsf_sched_init_info_t info)  {
  int scheduler_id, local_scheduler_level;

  struct timespec *duration=(struct timespec *)info;

  local_scheduler_level=SERVER_get_local_scheduler_level_from_budget(fsf_server_level, server);
  scheduler_id = SERVER_get_local_scheduler_id_from_budget(fsf_server_level,server);
  
  if (scheduler_id!=FSF_TABLE_DRIVEN)
    return FSF_ERR_SCHED_POLICY_NOT_COMPATIBLE;

  if (duration->tv_sec < 0 || duration->tv_nsec > 1000000000)
    return FSF_ERR_BAD_ARGUMENT;

  //TDSTAR_debugtable(local_scheduler_level);
  TDSTAR_start_simulation(local_scheduler_level); 
  return 0;

}  

int
fsf_get_total_quality 
(fsf_server_id_t server, int *total_quality) {
  int server_importance;
  int i=0,Qt=0;

  while(i<current_server) {
    if (server_list[i].server==server) break;
    i++;
  }
  if (i==current_server) return  FSF_ERR_BAD_ARGUMENT;
  else server_importance=server_list[i].Is;

  for (i=0; i<current_server; i++) {
    if (server_list[i].Is==server_importance)
      Qt+=server_list[i].Qs; 
  }

  return Qt;
}

int
fsf_get_available_capacity (
    fsf_server_id_t server, uint32_t *capacity){

  return 0;
}

bool
fsf_is_admission_test_enabled() {
  return true;
}


int
fsf_get_cpu_time
   (fsf_server_id_t server,
    struct timespec *cpu_time) {

  return 0;
}


int
fsf_get_contract
   (fsf_server_id_t server,
    fsf_contract_parameters_t *contract) {

  return 0;
}
