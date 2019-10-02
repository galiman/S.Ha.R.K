/*
 * Project: S.Ha.R.K.
 *
 * Coordinators:
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     :
 *   Trimarchi Michael   <trimarchi@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * Copyright (C) 2000 Giorgio Buttazzo, Paolo Gai
 *
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
 *
 * CVS :        $Id: server-task.c,v 1.17 2005/02/26 14:12:28 trimarchi Exp $
 */

#include "ll/i386/64bit.h"
#include "fsf_configuration_parameters.h"
#include "fsf.h"
#include "fsf_service_task.h"
#include "message.h"
#include <kernel/kern.h>
#include <sem/sem/sem.h>
#include <hartport/hartport/hartport.h>
#include <cabs/cabs/cabs.h>

//#define FSF_DEBUG
extern fsf_server_id_t service_server;

typedef enum {WAIT_SIGNAL, WAIT_RENEGOTIATION, ACTIVE} service_task_status;

TASK service_task(void) {
  
  struct mess msg; 
  fsf_renegotiation_status_t status;
  service_task_status stask_status=ACTIVE;

  PORT rec,tra;
 
  rec = port_connect("CHANW",sizeof(struct mess),STREAM,READ);
  
  tra = port_connect("CHANR",sizeof(struct mess),STREAM,WRITE);
  
  while(1) {
    //kern_printf("(Status %d)", stask_status); 
    switch(stask_status) {
    // wait for operation    
      case WAIT_RENEGOTIATION: 
	if (!fsf_get_renegotiation_status(msg.server,&status)) {
	  if (status==FSF_ADMITTED) {
	    // send response server is -1 if the operation fail
#ifdef FSF_DEBUG
	    kern_printf("ADM");
#endif
	    stask_status=ACTIVE;
	    //kern_printf("(REN %d)", msg.server);
	    port_send(tra,&msg,BLOCK);
	    //kern_printf("(--END-- 1)");

	    continue;
	  } else
#ifdef FSF_DEBUG
	    kern_printf("WAIT")
#endif
;
	}
	break;

    case WAIT_SIGNAL: 
      if (!fsf_get_renegotiation_status(msg.server,&status)) {
	if (status==FSF_ADMITTED) {	 	
	  //sigqueue(msg.process, msg.sig_notify, msg.sig_value);	 
	  stask_status=ACTIVE;
	  port_send(tra,&msg,BLOCK);
	  //kern_printf("(--END-- 2)");
	  continue;
	} 
      }
      break;

    case ACTIVE: 
      {
	port_receive(rec,&msg,BLOCK);
#ifdef FSF_DEBUG
	kern_printf("Message ");
#endif
	switch (msg.type) {
       
	case NEGOTIATE_CONTRACT:	  
	  if (negotiate_contract(&msg.contract, &msg.server)==FSF_ERR_CONTRACT_REJECTED)
	    msg.server=-1;	
	  else {
	    stask_status=WAIT_RENEGOTIATION;
	    //task_endcycle();
	    continue;
	  }
	  break;

	case REQUEST_RENEGOTIATE_CONTRACT:
	  if (renegotiate_contract(&msg.contract, msg.server)==FSF_ERR_CONTRACT_REJECTED)
	    msg.server=-1;
	  else {
	    stask_status=WAIT_SIGNAL;
	    //task_endcycle();
	    continue;
	  }
	  
	  break;
	  
	case RENEGOTIATE_CONTRACT:
	  if (renegotiate_contract(&msg.contract, msg.server)==FSF_ERR_CONTRACT_REJECTED)
	    msg.server=-1;
	  else {
	    stask_status=WAIT_RENEGOTIATION;
	    //task_endcycle();
	    continue;
	  }
	  break;			 
	case CHANGE_PARAMETER: 
	  {
	    fsf_contract_parameters_t new_contract;
	    struct timespec period;
	    struct timespec budget;
	    int i=0;
	    // this CASE change the quality and importance
	    //fsf_initialize_contract(&new_contract);
	    // find contract
	    while(i<current_server) {
	      if (server_list[i].server==msg.server) break;
	      i++;
	    }
	    //kern_printf("(RCQI)");
	    NULL_TIMESPEC(&period);
	    ADDUSEC2TIMESPEC(server_list[i].Tmax, &period);
	    NULL_TIMESPEC(&budget);
	    ADDUSEC2TIMESPEC(server_list[i].Cmin, &budget);
	    
	    fsf_set_contract_basic_parameters(&new_contract,&budget,&period,FSF_DEFAULT_WORKLOAD); 
	    NULL_TIMESPEC(&period);
	    ADDUSEC2TIMESPEC(server_list[i].Tmin, &period);
	    NULL_TIMESPEC(&budget);
	    ADDUSEC2TIMESPEC(server_list[i].Cmax, &budget);
	    
	    fsf_set_contract_reclamation_parameters(&new_contract, &budget, &period, FSF_DEFAULT_GRANULARITY,NULL,
						    msg.qi.quality,msg.qi.importance);
	  
	    if (server_list[i].d_equals_t == true) {
	      NULL_TIMESPEC(&new_contract.deadline);
	      new_contract.d_equals_t = true;
	    } else {
	      NULL_TIMESPEC(&period);
	      ADDUSEC2TIMESPEC(server_list[i].deadline, &period);
	    TIMESPEC_ASSIGN(&new_contract.deadline, &period); 
	    new_contract.d_equals_t = false;
	    }
	    if (renegotiate_contract(&new_contract, msg.server)==FSF_ERR_CONTRACT_REJECTED)
	      msg.server=-1;	
	}
	  break;
	  
	default :
	  break;
	  
	}
	break;
      }
    }
    
    port_send(tra,&msg,BLOCK);
    //kern_printf("(--END--)");
    
  }
}


static __inline void set_contract_parameter(void) {

  int i=0;
  TIME T,Q;
   
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
    
        server_list[i].actual_budget = Q;
        server_list[i].actual_period = server_list[i].Tmin;

	if (Q>server_list[i].Cmax)
	  Q=server_list[i].Cmax;
	
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
 
}


int renegotiate_contract 
  (const fsf_contract_parameters_t *new_contract,
   fsf_server_id_t                 server)
{
  
  #ifdef FSF_DEBUG
    kern_printf("(Renegotiate for server %d)",server);
  #endif

  if (!new_contract)
    return  FSF_ERR_BAD_ARGUMENT;

  if (server < 0)
    return  FSF_ERR_BAD_ARGUMENT;
 
   // change the parameter
   relink_contract_to_server(new_contract, server); 
   if (recalculate_contract(fsf_max_bw)==-1)  {
       return  FSF_ERR_CONTRACT_REJECTED;
   }
   
   set_contract_parameter();
   //kern_printf("(ERNC %d)", server);
   return 0;
}

int negotiate_contract
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t                 *server)
{
  /* Check if contract is initialized */
  if (!contract) return  FSF_ERR_BAD_ARGUMENT;

  /* Admission Test */
  if (FSF_ADMISSION_TEST_IS_ENABLED)
    if (add_contract(contract))
      return FSF_ERR_CONTRACT_REJECTED; 

  /* SERVER => BUDGET */    
  set_SERVER_budget_from_contract(contract,server);

#ifdef FSF_DEBUG
  kern_printf("(New Server %d)",*server);
#endif

  if (*server >= 0) {
    link_contract_to_server(contract,*server);
    if (recalculate_contract(fsf_max_bw)==-1)  {
      remove_contract(server);
      return  FSF_ERR_CONTRACT_REJECTED;
    }
    set_contract_parameter();
#ifdef  FSF_DEBUG
    kern_printf("(Adjust budget)");
#endif    
    
  }
  else  {
    return  FSF_ERR_CONTRACT_REJECTED;
  } 
  return 0;

}

