
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

#include <stdio.h>
#include <unistd.h>

#include <time.h>
#include <errno.h>

#include <pthread.h>
#include <sched.h>
//#include <tefses_stats.h>

#include "fsf.h"
#include "timespec_operations.h"
#include "fsf_os_compatibility.h"


/*

 * This is part of the common tests:
 
 ** Test for the calculus of the jitter

  -  Create a BOUNDED workload server, with 40ms minimum budget and 50ms
     as maximum and minimum period and also 50ms maximum budget,
     preemption level 4 and deadline equal to period. The server is 
     bound to the main thread with fsf_negotiate_contract_for_myself.
     
  -  The test consist of a periodic activation at absolutes points in time,
     the first instruction after each activation is the measurement of the
     absolute real-time of activation. Minimum, maximum and average
     differences are stored. The test runs 500 times.
     
    
*/

#define LOCAL_ERROR(nn,ss) {if(errno==0) errno=(nn); perror(ss); return (nn);}
//#define ERROR(nn,ss) {if(errno==0) errno=(nn); perror(ss); exit (nn);}

struct timespec            instant_zero = {0,0};
int                        main_priority = 4;
struct timespec            last_time = {0,0};
fsf_server_id_t            server_A = 0;

extern int cal_cycles;
extern int calibrate_cycle(void);
extern void eat(TIME wait);


int
fsf_priority_map (unsigned long plevel)
{
  return plevel;
}


int main()
{
   struct sched_param	param;
   //struct timespec      half_second={0,500000000};
   struct timespec      tmp = {0,0};
   int                  terror = 0;

  fsf_contract_parameters_t  contract;
  struct timespec    budget_min = {0,30000000};
  struct timespec    period_max = {0,50000000};
  struct timespec    budget_max = {0,50000000};
  struct timespec    period_min = {0,50000000};
  fsf_workload_t     workload = FSF_BOUNDED;

  bool               d_equals_t = true;
  struct timespec    deadline = {0,50000000};
  int                budget_overrun_sig_notify = FSF_NULL_SIGNAL;   // 0
  union sigval       budget_overrun_sig_value = {0};
  int                deadline_miss_sig_notify = FSF_NULL_SIGNAL;    // 0
  union sigval       deadline_miss_sig_value = {0};

  fsf_granularity_t      granularity = FSF_DEFAULT_GRANULARITY;       // CONTINUOUS
  fsf_utilization_set_t *utilization_set = FSF_NULL_UTILIZATION_SET;  // NULL
  int                    quality = 1;
  int                    importance = FSF_DEFAULT_IMPORTANCE;         // 1

  fsf_preemption_level_t   preemption_level = (fsf_preemption_level_t) main_priority;
  fsf_critical_sections_t *critical_sections = NULL;
  
  struct timespec  max_jitter={0,0};
  struct timespec  min_jitter={100000000,0};
  struct timespec  avg_jitter={0,0};
  int              my_counter = 0;
/*
  struct timespec  next_budget;
  struct timespec  next_period;
  bool             was_deadline_missed = 0;
  bool             was_budget_overran = 0;
*/
  //pthread_t                task_in_b;
  INITIALIZATION_CODE 
  
  SERIAL_CONSOLE_INIT; //marte1.26i+

   param.sched_priority = main_priority;
   if ((terror=pthread_setschedparam (pthread_self (), SCHED_FIFO, &param)))
     LOCAL_ERROR(terror,"pthread_setschedparam");

   instant_zero.tv_sec = 10000000;
   instant_zero.tv_nsec = 10000000;
   clock_settime(CLOCK_REALTIME,&instant_zero);
   clock_gettime(CLOCK_REALTIME,&instant_zero);
   last_time = instant_zero;
//   put_time(&instant_zero, "instant_zero");
//   put_time(NULL, "printing point 1");
//   put_time(NULL, "printing point 2");
   

   // Adjust the time eater (in load.c)
   //adjust ();

   
  if ((terror=fsf_initialize_contract(&contract)))
  {
     printf(" Initialize fail for server A\n");
     LOCAL_ERROR(terror,"fsf_initialize_contract failed");
  }

  if ((terror=fsf_set_contract_basic_parameters (&contract,
                                    &budget_min,
                                    &period_max,
                                    workload)))
  {
     printf("Set_Basic_Parameters failed for server A\n");
     LOCAL_ERROR(terror,"set_contract_basic_parameters failed");
  }

  if ((terror=fsf_set_contract_timing_requirements (&contract,
						    d_equals_t,
						    (d_equals_t?NULL:&deadline),
						    budget_overrun_sig_notify,
						    budget_overrun_sig_value,
						    deadline_miss_sig_notify,
						    deadline_miss_sig_value)))
  {
     printf("Set_Timing_Requirements failed for server A\n");
     LOCAL_ERROR(terror,"fsf_set_contract_timing_requirements failed");
  }

  if ((terror=fsf_set_contract_reclamation_parameters (&contract,
						       &budget_max,
						       &period_min,
						       granularity,
						       utilization_set,
						       quality,
						       importance)))
  {
     printf("Set_Reclamation_Parameters failed for server A\n");
     LOCAL_ERROR(terror,"fsf_set_contract_reclamation_parameters failed");
  }

  //preemption_level = (fsf_preemption_level_t) param.sched_priority;
  if ((terror=fsf_set_contract_synchronization_parameters (&contract,                                           
							   critical_sections)))
  {
     printf("Set_Synchronization_Parameters failed for server A\n");
     LOCAL_ERROR(terror,"fsf_set_contract_synchronization_parameters failed");
  }

  terror = fsf_negotiate_contract_for_myself(&contract, &server_A);
  if (terror)
  {
    printf("Negotiate_Contract failed for server A\n");
    ERROR(terror,"fsf_negotiate_contract_for_myself failed");
  }

  { /* Bounded workload task block  */
    struct timespec  next_budget;
    struct timespec  next_period;
    bool             was_deadline_missed = 0;
    bool             was_budget_overran = 0;
    struct timespec  at_absolute_time;
    struct timespec  my_period = {0,53000000}; // 53 miliseconds
    struct timespec  ahora;
    int              i;

    if ((terror=clock_gettime(CLOCK_REALTIME,&at_absolute_time)))
      LOCAL_ERROR(terror,"clock_gettime failed");
   
    for (i=0;i<500;i++) {

      incr_timespec(&at_absolute_time, &my_period);
    
      if ((terror=fsf_schedule_timed_job (
                                       &at_absolute_time,
                                       &next_budget,
                                       &next_period,
                                       &was_deadline_missed,
                                       &was_budget_overran)))
      {
         ERROR(terror,"fsf_bounded_server: a call to fsf_schedule_next_timed_job failed");
      }
    
    
      //printf("\ndebug: ONE-> \n");
      if ((terror=clock_gettime(CLOCK_REALTIME,&ahora)))
        LOCAL_ERROR(terror,"ahora clock_gettime failed");
      
      //printf("\ndebug: TWO\n");
      decr_timespec(&ahora, &at_absolute_time);

      if ( smaller_timespec(&ahora, &min_jitter) )
        min_jitter = ahora;
          
      if ( smaller_timespec(&max_jitter, &ahora) )
        max_jitter = ahora;
          
      incr_timespec(&avg_jitter, &ahora);
        
      my_counter++; 
    }  
    printf("\n\n        minimum jitter is: %8d\n", min_jitter.tv_nsec+ min_jitter.tv_sec*1000000000);
    printf("        maximum jitter is: %8d\n", max_jitter.tv_nsec+ max_jitter.tv_sec*1000000000);
    printf("        average jitter is: %8d\n\n", (avg_jitter.tv_nsec+ avg_jitter.tv_sec*1000000000)/my_counter);

  } /* End of bounded workload task block  */

  
  STANDARD_CONSOLE_INIT; //marte1.26i+

  fsf_printf("\n\n        minimum jitter is: %8d\n", min_jitter.tv_nsec+ min_jitter.tv_sec*1000000000);
  fsf_printf("        maximum jitter is: %8d\n", max_jitter.tv_nsec+ max_jitter.tv_sec*1000000000);
  fsf_printf("        average jitter is: %8d\n\n", (avg_jitter.tv_nsec+ avg_jitter.tv_sec*1000000000)/my_counter);

  //printf("\nThe end.\n");

  //stop_scheduler = 1;
  exit(0);
  return 0;
} /* End of main */
