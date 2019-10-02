/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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

/* The tracer instrumentation ideas come from the York PWCET analisys tool 
 *	
 *	Real-Time System Group
 *	University of York
 *
 */

#include <kernel/kern.h>
#include <time.h>
#include <tracer.h>

#define PWCET_Automatic_Ipoint(a) TRACER_LOGEVENT(FTrace_EVT_ipoint,(a),0);

extern int instrumented_function();

TASK ext_call(void)
{

  while(1) {

    instrumented_function();
	
    task_testcancel();
    task_endcycle();

  }

  return 0;

}

#define TASKMAX 4
#define TASKDELAY 1000000
#define TASKDELAY_DELTA 100000

int exec_code() {

  int num;
  struct timespec t;

  HARD_TASK_MODEL m;
  PID p;

  clear();

  cprintf("Start");

  hard_task_default_model(m);
  hard_task_def_mit(m,200000 + rand() % 100000);
  hard_task_def_wcet(m,40000);
  hard_task_def_group(m,2);

  num = 0;

  while(num < TASKMAX) {

    cprintf(".");

    p = task_create("Instr",ext_call,&m,NULL);
    if (p == -1) {
      sys_shutdown_message("Could not create task <instr> ...");
      exit(1);
    }

    num++;
    task_activate(p);

    usleep(TASKDELAY + rand() % TASKDELAY_DELTA);

  }

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 12);  

  cprintf("End\n");

  return 0;

}

extern unsigned int clk_per_msec;

int main(int argc, char **argv)
{

  int a,b,c,i;
  struct timespec t,start,end,diff;

  srand(sys_gettime(NULL));

  a = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);
  b = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_JTN);
  c = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);

  FTrace_chunk_link(a,b);
  FTrace_chunk_link(b,c);

  FTrace_actual_chunk_select(a);

  kern_gettime(&start);
  FTrace_enable();

  TRACER_LOGEVENT(FTrace_EVT_trace_start,proc_table[exec_shadow].context,clk_per_msec);

  for (i=0;i<10;i++)
	if (proc_table[i].context != 0) TRACER_LOGEVENT(FTrace_EVT_id,
						(unsigned short int)proc_table[i].context,i);

    exec_code();

  TRACER_LOGEVENT(FTrace_EVT_trace_stop,0,0);  

  FTrace_disable();
  kern_gettime(&end);

  SUBTIMESPEC(&end,&start,&diff);

  printf_xy(1,21,WHITE,"Logged Time %d s %d us",(int)diff.tv_sec,(int)diff.tv_nsec/1000);

  group_kill(2);

  do {
    sys_gettime(&t);
  } while (t.tv_sec < 12);

  FTrace_OSD_init_udp(1, "192.168.0.15", "192.168.0.12");
                                                                                                                             
  FTrace_send_chunk(a, 0, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);

  return 0;

}

