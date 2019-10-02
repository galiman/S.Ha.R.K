
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
// FIRST Scheduling Framework
// 
//=====================================================================



#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <fsf.h>

#include "timespec_operations.h"
#include "fsf_os_compatibility.h"

//#include <debug_marte.h>

/*

 * This is part of the common tests:
 
 ** Test to get the time it takes the negotiation of contracts and
    the maximum reacheable utilization
 
  -  Prepare the configuration files to accept up to 100 servers,
     enable the acceptance test, set the service thread with 100s period
     and 1s budget, and the scheduler priority to be 102

  -  Fill a contract with the values: Cmin 1s, Cmax 50s, Tmax 100s,
     Tmin 50s, INDETERMINATE workload, and the preemptibility level equal
     to 101 minus the number of already accepted servers. Create a server
     with that contract, and bound it to the main thread using the
     fsf_negotiate_contract_for_myself primitive.
     
  -  then as a servered task, negotiates as much of the prepared contracts 
     as possible, taking the utilization given to each of the contracts
     after each new contract is accepted and the time it took the
     negotiation.
  
 ** Test to get the time it takes re-negotiation of contracts and
    the change of quality and importance
  
  -  change the last parragraph of the test above, so that contracts are
     negotiated and bound at the same time to the ever running threads,
     and also after each succesful negotiation, the contracts are
     re-negotiated, and its quality and importance changed to the same
     values. They are taken the utilization given to each server and the
     time it took each operation.

*/


#define FSF_MAX_N_TIME_VALUES     1000

#define N_SERVERS          10

struct timespec            instant_zero = {0,0};
int                        main_priority = N_SERVERS+1;
struct timespec            last_time = {0,0};
fsf_server_id_t            server[N_SERVERS];
struct server_res_t {
  struct timespec        negotiation;
  struct timespec        renegotiation;
  struct timespec        change_q_and_i;
  struct timespec        cancellation;
  double                 min_u, max_u, tot_u;
}                          server_res[N_SERVERS];
  
int                        n_servers = 0;
pthread_t                  task[N_SERVERS];



struct pseudo_printed_results {
  char            *s;
  struct timespec  t;
}                                  res[FSF_MAX_N_TIME_VALUES];
int                                res_index = 0;


extern int cal_cycles;
extern int calibrate_cycle(void);
extern void eat(TIME wait);

//put_time has been reduced in this experiment
//so that it just reports the real-time it is
struct timespec
put_time(const struct timespec *time, char *s)
{
  struct timespec now = {0,0};
  int terror;

  if ((terror=clock_gettime(CLOCK_REALTIME,&now)))
    ERROR(terror,"clock_gettime failed");
  
  return now;
} /* End of put_time */

/*
int
print_time_results()
{
  struct timespec now = {0,0};
  struct timespec diff = {0,0};
  char            *s = NULL;
  int i;

  printf("\n");
  for (i=0;i<res_index;i++)
  {
    now  = res[i].t;
    s    = res[i].s;
    if (s==NULL) s = "  - timestamp - ";
    diff = now;
    decr_timespec(&diff, &last_time);
    last_time = now;
    printf("%-60s", s);
    printf(" %2d %9d", now.tv_sec,now.tv_nsec);
    printf(" [diff=%13.9f]\n", (double)diff.tv_sec+((double)diff.tv_nsec/(double)1000000000));
  }

  return 0;
} / * End of print_time_results * /
*/
/*
int
fsf_priority_map (unsigned long plevel)
{
  return plevel;
}
*/

void * fsf_indeterminate_server (void * arg)
{

  //executes for ever
  while (1)  eat(1000000);

} /* End of fsf_indeterminate_server */



int main()
{
   struct sched_param	param;
   //struct timespec      half_second={0,500000000};
   struct timespec      tmp = {0,0};
   int                  terror = 0;

  fsf_contract_parameters_t  contract;
  struct timespec    budget_min = {1,0};      //{20,710678118};
  //  struct timespec    budget_min = {21,210678118};
  struct timespec    period_max = {100,0};
  struct timespec    budget_max = {50,0}; //{20,710678118};
  struct timespec    period_min = {50,0};
  fsf_workload_t     workload = FSF_INDETERMINATE; //FSF_BOUNDED;

  bool               d_equals_t = true;
  struct timespec    deadline = {0,0};
  int                budget_overrun_sig_notify = FSF_NULL_SIGNAL;   // 0
  union sigval       budget_overrun_sig_value = {0};
  int                deadline_miss_sig_notify = FSF_NULL_SIGNAL;    // 0
  union sigval       deadline_miss_sig_value = {0};

  fsf_granularity_t      granularity = FSF_DEFAULT_GRANULARITY;       // CONTINUOUS
  fsf_utilization_set_t *utilization_set = FSF_NULL_UTILIZATION_SET;  // NULL
  int                    quality = 1;
  int                    importance = FSF_DEFAULT_IMPORTANCE;         // 1

  fsf_preemption_level_t   preemption_level;
  fsf_critical_sections_t *critical_sections = NULL;

  struct timespec    budget = {0,0};
  struct timespec    period = {0,0};
  struct timespec    start, end;
  int                i;
  double             u;

   INITIALIZATION_CODE;

   param.sched_priority = main_priority;
   if ((terror=pthread_setschedparam (pthread_self(), SCHED_FIFO, &param)))
     ERROR(terror,"pthread_setschedparam");

   instant_zero.tv_sec = 10000000;
   instant_zero.tv_nsec = 10000000;
   clock_settime(CLOCK_REALTIME,&instant_zero);
   clock_gettime(CLOCK_REALTIME,&instant_zero);
   last_time = instant_zero;
   put_time(&instant_zero, "instant_zero");
   put_time(NULL, "printing point 1");
   put_time(NULL, "printing point 2");
   



  if ((terror=fsf_initialize_contract(&contract)))
  {
     fsf_printf(" Initialize fail for server A\n");
     ERROR(terror,"fsf_initialize_contract failed");
  }

  if ((terror=fsf_set_contract_basic_parameters (&contract,
                                    &budget_min,
                                    &period_max,                               
                                    workload)))
  {
     fsf_printf("Set_Basic_Parameters failed for server A\n");
     ERROR(terror,"set_contract_basic_parameters failed");
  }

  if ((terror=fsf_set_contract_timing_requirements (&contract,
                                      d_equals_t,
                                      (d_equals_t?NULL:&deadline),
                                      budget_overrun_sig_notify,
                                      budget_overrun_sig_value,
                                      deadline_miss_sig_notify,
                                      deadline_miss_sig_value)))
  {
     fsf_printf("Set_Timing_Requirements failed for server A\n");
     ERROR(terror,"fsf_set_contract_timing_requirements failed");
  }

  if ((terror=fsf_set_contract_reclamation_parameters (&contract,
				          &budget_max,
					  &period_min,
                                          granularity,
                                          utilization_set,
                                          quality,
                                          importance)))
  {
     fsf_printf("Set_Reclamation_Parameters failed for server A\n");
     ERROR(terror,"fsf_set_contract_reclamation_parameters failed");
  }

  preemption_level = (fsf_preemption_level_t) param.sched_priority;
  if ((terror=fsf_set_contract_synchronization_parameters (&contract,
							   critical_sections)))
  {
     fsf_printf("Set_Synchronization_Parameters failed for server A\n");
     ERROR(terror,"fsf_set_contract_synchronization_parameters failed");
  }

  start = put_time(NULL, "start first server contract negotiation");
  terror = fsf_negotiate_contract_for_myself (&contract, &server[0]);
  end   = put_time(NULL, "  end first server contract negotiation");
  if (terror)
  {
    fsf_printf("Negotiate_Contract failed for server 0\n");
    ERROR(terror,"fsf_negotiate_contract_for_myself failed");
  }
  
  decr_timespec(&end, &start);
  server_res[0].negotiation = end;
  
  start = put_time(NULL, "start first server contract renegotiation");
  terror = fsf_renegotiate_contract (&contract, server[0]);
  end   = put_time(NULL, "  end first server contract renegotiation");
  if (terror)
  {
    fsf_printf("ReNegotiate_Contract failed for server 0\n");
    ERROR(terror,"fsf_renegotiate_contract failed");
  }
  
  decr_timespec(&end, &start);
  server_res[0].renegotiation = end;
  
  start = put_time(NULL, "start first server change quality and importance");
  terror = fsf_request_change_quality_and_importance(server[0],1,1);
  end   = put_time(NULL, "  end first server change quality and importance");
  if (terror)
  {
    fsf_printf("Change quality and importancet failed for server 0\n");
    ERROR(terror,"fsf_renegotiate_contract failed");
  }
  
  decr_timespec(&end, &start);
  server_res[0].change_q_and_i = end;

  tmp = server_res[0].renegotiation;
  incr_timespec(&tmp, &server_res[0].negotiation);
  
  //release the CPU for a while to get the quality change done
  if ((terror=nanosleep(&tmp,NULL)))
    ERROR(terror, "nanosleep failed");

//set_break_point_here;

  //ask for the utilization actually gotten
  terror = fsf_get_budget_and_period (server[0], &budget, &period);
  if (terror)
  {
    fsf_printf("fsf_get_budget_and_period failed for server 0\n");
    ERROR(terror,"fsf_get_budget_and_period failed");
  }
//  put_time(&budget,"the budget gotten is: ");
//  put_time(&period,"the period gotten is: ");
  server_res[0].min_u = server_res[0].max_u = server_res[0].tot_u = t2d(budget) / t2d(period);
/*
  budget_min.tv_sec = 1;
  budget_min.tv_nsec = 0;
//  budget_max = budget_min;
  if ((terror=fsf_set_contract_basic_parameters (&contract,
                                    &budget_min,
                                    &period_max,
                                    &budget_max,
                                    &period_min,
                                    workload)))
  {
     printf("Set_Basic_Parameters failed for a server \n");
  }
*/
  while(++n_servers < N_SERVERS) {

    //preparation of the rest of the servers
////if (n_servers==27) set_break_point_here;
    preemption_level = main_priority - n_servers;
    if ((terror=fsf_set_contract_synchronization_parameters (&contract,
							     critical_sections)))
    {
       fsf_printf("Set_Synchronization_Parameters failed for server %d\n", n_servers);
       ERROR(terror,"fsf_set_contract_synchronization_parameters failed");
    }
  
    start = put_time(NULL, "start server contract negotiation");
    //terror=fsf_negotiate_contract_for_new_thread (&contract, &server[n_servers], &task[n_servers], NULL, fsf_indeterminate_server, NULL);
    terror=fsf_negotiate_contract(&contract, &server[n_servers]);
    end   = put_time(NULL, "  end server contract negotiation");
    if (terror)
    {
      fsf_printf("Negotiate_Contract failed for server %d\n", n_servers);
      ERROR(terror,"fsf_negotiate_contract failed");
    }
  
    if (!server[n_servers]) {
      fsf_printf("The negotiation for the server number %d was not succesful!! (n_servers=%d)\n",n_servers+1, n_servers); 
      break;
    }
  
////set_break_point_here;
    
    decr_timespec(&end, &start);
    server_res[n_servers].negotiation = end;
    
    start = put_time(NULL, "start server contract renegotiation");
    terror = fsf_renegotiate_contract (&contract, server[n_servers]);
    end   = put_time(NULL, "  end server contract renegotiation");
    if (terror)
    {
      printf("ReNegotiate_Contract failed for server %d\n", n_servers);
      ERROR(terror,"fsf_renegotiate_contract failed");
    }
    
    decr_timespec(&end, &start);
    server_res[n_servers].renegotiation = end;
    
    start = put_time(NULL, "start first server change quality and importance");
    terror = fsf_request_change_quality_and_importance(server[n_servers],1,1);
    end   = put_time(NULL, "  end first server change quality and importance");
    if (terror)
    {
      fsf_printf("Change quality and importancet failed for server %d\n", n_servers);
      ERROR(terror,"fsf_request_change_quality_and_importance failed");
    }
    
    decr_timespec(&end, &start);
    server_res[n_servers].change_q_and_i = end;
  
    tmp = server_res[n_servers].renegotiation;
    incr_timespec(&tmp, &server_res[n_servers].negotiation);
    
    //release the CPU for a while long enough to get the change done
    if ((terror=nanosleep(&tmp,NULL)))
      ERROR(terror, "nanosleep failed");
  
////set_break_point_here;
  
    //ask for the utilization actually gotten
    terror = fsf_get_budget_and_period (server[0], &budget, &period);
    if (terror)
    {
      fsf_printf("fsf_get_budget_and_period failed for server 0\n");
      ERROR(terror,"fsf_get_budget_and_period failed");
    }
    if (period.tv_sec==0 && period.tv_nsec==0) {
      fsf_printf("while processing server %d, the period gotten for server %d is cero!!\n", n_servers, 0);
      exit(-1);
    }
    u = t2d(budget) / t2d(period);
    server_res[n_servers].min_u = server_res[n_servers].max_u = server_res[n_servers].tot_u = u;
  
    //get statistics for the utilization of all the accepted contracts up to the current one
    for (i=1;i<=n_servers;i++){
  
      terror = fsf_get_budget_and_period (server[i], &budget, &period);
      if (terror)
      {
        fsf_printf("fsf_get_budget_and_period failed for server %d\n", n_servers);
        ERROR(terror,"fsf_get_budget_and_period failed");
      }
      if (period.tv_sec==0 && period.tv_nsec==0) {
        fsf_printf("while processing server %d, the period gotten for server %d is cero!!\n", n_servers, i);
        exit(-1);
      }
      u = t2d(budget) / t2d(period);
      server_res[n_servers].tot_u += u;
      if (u > server_res[n_servers].max_u) server_res[n_servers].max_u = u;
      if (u < server_res[n_servers].min_u) server_res[n_servers].min_u = u;
    }
  
    //printf(" %d", n_servers);
  
  } /* End of the while */
  
  //cancellation of contracts
  for (i=n_servers-1; i>0;i--) {
    start = put_time(NULL, "start server contract cancellation");
    terror=fsf_cancel_contract(server[i]);
    end   = put_time(NULL, "  end server contract cancellation");
    if (terror)
    {
      fsf_printf("Cancel_Contract failed for server %d\n", i);
      ERROR(terror,"fsf_cancel_contract failed");
    }
  
    decr_timespec(&end, &start);
    server_res[i].cancellation = end;
  }
  
  fsf_printf("\n");
  fsf_printf("The number of accepted servers was: %d \n", n_servers);
  fsf_printf("==================================\n\n");
  
  fsf_printf("server    negotiation   renegotiation  qualityChange  cancellation  max_utilizati  min_utilizati total_utilization\n\n");
  
  for (i=0;i<n_servers;i++){
    fsf_printf("  %2d   ", i+1);
    fsf_printf(" %13.9f ", t2d(server_res[i].negotiation));
    fsf_printf(" %13.9f ", t2d(server_res[i].renegotiation));
    fsf_printf(" %13.9f ", t2d(server_res[i].change_q_and_i));
    fsf_printf(" %13.9f ", t2d(server_res[i].cancellation));
    fsf_printf(" %13.9f ", server_res[i].max_u);
    fsf_printf(" %13.9f ", server_res[i].min_u);
    fsf_printf(" %13.9f ", server_res[i].tot_u);
    fsf_printf("\n");
  }
  
  STANDARD_CONSOLE_INIT; //marte1.26i+
  
  fsf_printf("\n");
  fsf_printf("The number of accepted servers was: %d \n", n_servers);
  fsf_printf("==================================\n\n");
  
  fsf_printf("server    negotiation   renegotiation  qualityChange  cancellation  max_utilizati  min_utilizati total_utilization\n\n");
  
  for (i=0;i<n_servers;i++){
    fsf_printf("  %2d   ", i+1);
    fsf_printf(" %13.9f ", t2d(server_res[i].negotiation));
    fsf_printf(" %13.9f ", t2d(server_res[i].renegotiation));
    fsf_printf(" %13.9f ", t2d(server_res[i].change_q_and_i));
    fsf_printf(" %13.9f ", t2d(server_res[i].cancellation));
    fsf_printf(" %13.9f ", server_res[i].max_u);
    fsf_printf(" %13.9f ", server_res[i].min_u);
    fsf_printf(" %13.9f ", server_res[i].tot_u);
    fsf_printf("\n");
  }
  
  fsf_printf("\nThe end.\n");
 
//....
  exit(-1);
  return 0;
} /* End of main */
