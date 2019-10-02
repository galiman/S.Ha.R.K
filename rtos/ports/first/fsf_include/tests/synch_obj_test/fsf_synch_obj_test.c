
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
#include <fsf.h>
#include "timespec_operations.h"
#include "fsf_os_compatibility.h"


/*

 * This is part of the common tests:
 
 ** Test of the synchronization object primitives

  -  Create two servers, server A and server B. Each with 5 seconds budget 
     and 20 seconds period (minimum values equal to the maximum ones),
     preemption levels for A = 4 and for B = 3, A is INDETERMINATE workload
     and B is BOUNDED workload. Deadline is 19.9 seconds for A and it is
     equal to the period for B. Server A is bound to the main thread with 
     fsf_negotiate_contract_for_myself, server B is created with 
     fsf_negotiate_contract_for_new_thread.
     
  -  Once the main program becomes server A, it creates a Synch_object 
     and stores its reference in the global variable SO1_handle, then it 
     creates server B for a new thread and releases
     the CPU for 0.5 second. The new thread in server B after starting,
     just waits for SO1 to be signaled, after that it executes during 1 second,
     then it waits for the the synchronization object SO1_handle to be signaled
     again, and finally ends.
     
  -  After the first release of the CPU, the main thread in server A 
     signals SO1_handle and then releases the CPU for 0.5 second again, 
     after that it signals SO1_handle a second time and releases the CPU for 2 seconds.
     Finally it destroy the synchronization object OS1 and ends.
     
*/

#define LOCAL_ERROR(nn,ss) {if(errno==0) errno=(nn); perror(ss); return (nn);}
//#define ERROR(nn,ss) {if(errno==0) errno=(nn); perror(ss); exit (nn);}
#define FSF_MAX_N_TIME_VALUES     1000

struct timespec            instant_zero = {0,0};
int                        main_priority = 4;
struct timespec            last_time = {0,0};
fsf_server_id_t            server_A = 0;
fsf_server_id_t            server_B = 0;
fsf_synch_obj_handle_t     SO1_handle;

extern int cal_cycles;
extern int calibrate_cycle(void);
extern void eat(TIME wait);

struct pseudo_printed_results {
  char            *s;
  struct timespec  t;
}                                  res[FSF_MAX_N_TIME_VALUES];
volatile int                       res_index = 0;

//reports the real-time it is
//with a comment and the difference to the last reported time
int
put_time(const struct timespec *time, char *s)
{
  struct timespec now = {0,0};
  int terror;

//  if(time == (struct timespec *)NULL) {
    if ((terror=clock_gettime(CLOCK_REALTIME,&now)))
      ERROR(terror,"clock_gettime failed");
//  }
//  else {
//    now = *time;
//  }

  res[res_index].s = s;
  res[res_index].t = now;

  //  printf("(%2d)", res_index+1);
  //  printf("%-60s", s);
  //  printf(" %2d %9d\n", now.tv_sec,now.tv_nsec);

  if (res_index < FSF_MAX_N_TIME_VALUES) res_index++;
  
  return 0;
} /* End of put_time */


int
print_time_results()
{
  struct timespec now = {0,0};
  struct timespec diff = {0,0};
  char            *s = NULL;
  int i;

  fsf_printf("\n");
  for (i=0;i<res_index;i++)
  {
    now  = res[i].t;
    s    = res[i].s;
    if (s==NULL) s = "  - timestamp - ";
    diff = now;
    decr_timespec(&diff, &last_time);
    last_time = now;
    fsf_printf("(%2d)", i+1);
    fsf_printf("%-60s", s);
    fsf_printf(" %2d %9d", now.tv_sec,now.tv_nsec);
    fsf_printf(" [diff=%13.9f]\n", (double)diff.tv_sec+((double)diff.tv_nsec/(double)1000000000));
  }

  return 0;
} /* End of print_time_results */



int
fsf_priority_map (unsigned long plevel)
{
  return plevel;
}


void * fsf_bounded_server_B (void * arg)
{
  //fsf_server_id_t       in = *((fsf_server_id_t *)arg);
  int                   terror=0;

  //fsf_server_id_t     server;

  //struct timespec  next_activation_time = in.offset;
  struct timespec  next_budget;
  struct timespec  next_period;
  bool             was_deadline_missed = 0;
  bool             was_budget_overran = 0;

  //executes for 0.1 second

  put_time(NULL, "B start one tenth of second execution   ");
  eat(100000);
  put_time(NULL, "B complete one tenth of second execution");

  //wait for the the synchronization object SO1_handle to be signaled

  put_time(NULL, "B called first fsf_schedule_triggered_job     ");

  if ((terror=fsf_schedule_triggered_job (
                                       SO1_handle,
                                       &next_budget,
                                       &next_period,
                                       &was_deadline_missed,
                                       &was_budget_overran)))
  {
     ERROR(terror,"fsf_bounded_server: first call to fsf_schedule_triggered_job failed");
  }
  put_time(NULL, "B returned from first fsf_schedule_triggered_job");



  //executes for 1 second

  put_time(NULL, "B start 1 second execution   ");
  eat(1000000);
  put_time(NULL, "B complete the 1 second execution");

  //wait for the the synchronization object SO1_handle to be signaled again
  put_time(NULL, "B called fsf_schedule_triggered_job");
  terror=fsf_schedule_triggered_job (
                                       SO1_handle,
                                       &next_budget,
                                       &next_period,
                                       &was_deadline_missed,
                                       &was_budget_overran);
  put_time(NULL, "B returned from fsf_schedule_triggered_job");
  if (terror)
  {
     ERROR(terror,"fsf_bounded_server: second call to fsf_schedule_triggered_job failed");
  }


  //end
  put_time(NULL, "B ends");

  return 0;

} /* End of fsf_bounded_server_B */



int main()
{
   struct sched_param	param;
   struct timespec      half_second= {0,500000000};
   struct timespec      tmp = {0,0};
   int                  terror = 0;

  fsf_contract_parameters_t  contract;
  struct timespec    budget_min = {7,0};
  struct timespec    period_max = {20,0};
  struct timespec    budget_max = {7,0};
  struct timespec    period_min = {20,0};
  fsf_workload_t     workload = FSF_INDETERMINATE;

  bool               d_equals_t = false;
  struct timespec    deadline = {19,900000000};
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

  pthread_t                task_in_b;

  INITIALIZATION_CODE
  SERIAL_CONSOLE_INIT; 
  
  param.sched_priority = main_priority;
  if ((terror=pthread_setschedparam(pthread_self(), SCHED_FIFO, &param)))
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


  put_time(NULL, "A start first server contract negotiation");
  terror = fsf_negotiate_contract_for_myself (&contract, &server_A);
  put_time(NULL, "A end first server contract negotiation");
  if (terror)
  {
    fsf_printf("Negotiate_Contract failed for server A\n");
    ERROR(terror,"fsf_negotiate_contract_for_myself failed");
  }

  //creation of the synchronization object
  put_time(NULL, "A start creating synchronization object");
  terror=fsf_create_synch_obj(&SO1_handle);
  put_time(NULL, "A end creating synchronization object");
  if (terror)
  {
     ERROR(terror,"fsf_signal_synchobject failed");     
  }


  //preparation of server B
  workload = FSF_BOUNDED;
  d_equals_t = true;
  //deadline.tv_sec = deadline.tv_nsec = 0;

  if ((terror=fsf_set_contract_basic_parameters (&contract,
                                    &budget_min,
                                    &period_max,                                
                                    workload)))
  {
     fsf_printf("Set_Basic_Parameters failed for server B\n");
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
     fsf_printf("Set_Timing_Requirements failed for server B\n");
     ERROR(terror,"fsf_set_contract_timing_requirements failed");
  }

  preemption_level--;
  if ((terror=fsf_set_contract_synchronization_parameters (&contract,                                            
                                              critical_sections)))
  {
     fsf_printf("Set_Synchronization_Parameters failed for server B\n");
     ERROR(terror,"fsf_set_contract_synchronization_parameters failed");
  }

  put_time(NULL, "A starts server B contract negotiation");
  terror=fsf_negotiate_contract_for_new_thread (&contract, &server_B, &task_in_b, NULL, fsf_bounded_server_B, NULL);
  put_time(NULL, "A ends server B contract negotiation");
  if (terror)
  {
     fsf_printf("Negotiate_Contract failed for server B\n");
     ERROR(terror,"fsf_negotiate_contract_for_new_thread failed");
  }

  if(!server_B) ERROR((-1), "Contract B not accepted");
  
  put_time(NULL, "A starts a half second nanosleep");
  if ((terror=nanosleep(&half_second,NULL)))
  ERROR(terror, "nanosleep 1 failed");
  put_time(NULL, "A completes the half second nanosleep");
  
  put_time(NULL, "A starts post-signaling synchronization object");
  terror=fsf_signal_synch_obj(SO1_handle);
  put_time(NULL, "A ends post-signaling synchronization object");
  if (terror)
  {
     ERROR(terror,"fsf_signal_synchobject failed");
  }
  
  half_second.tv_sec = 3;
  half_second.tv_nsec = 0;
  put_time(NULL, "A starts another 3s nanosleep");
  if ((terror=nanosleep(&half_second,NULL)))
    ERROR(terror, "nanosleep 2 failed");
  put_time(NULL, "A completes the other half second nanosleep");
  
  put_time(NULL, "A starts pre-signaling synchronization object");
  terror=fsf_signal_synch_obj(SO1_handle);
  put_time(NULL, "A ends pre-signaling synchronization object");
  if (terror)
  {
     ERROR(terror,"fsf_signal_synchobject failed");
  }
 
  tmp.tv_sec = 2;
  tmp.tv_nsec = 0;
  put_time(NULL, "A starts a final 2 seconds nanosleep");
  if ((terror=nanosleep(&tmp,NULL)))
    ERROR(terror, "third nanosleep failed");
  put_time(NULL, "A ends the final 2 seconds nanosleep");
  
  put_time(NULL, "A starts destroying synchronization object"); 
  terror=fsf_destroy_synch_obj(SO1_handle);
  put_time(NULL, "A ends destroying synchronization object");
  if (terror)
  {
     ERROR(terror,"fsf_destroy_synchobject failed");
  }

  print_time_results();
  
  STANDARD_CONSOLE_INIT; //marte1.26i+

  //print_time_results();

  fsf_printf("\nThe End.\n");

  //stop_scheduler = 1;

  exit(0);
  return 0;
} /* End of main */
