/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   Tullio Facchinetti  <tullio.facchinetti@unipv.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 * http://robot.unipv.it
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

#include <kernel/kern.h>
#include <FTrace_chunk.h>
#include <FTrace_udp.h>
#include <tracer.h>

extern unsigned int clk_per_msec;

int main(int argc, char **argv)
{

  long long i;

  int a,b,c;
  struct timespec start,end,diff;
  SYS_FLAGS f;

  /** 
    *  Create 3 chunks for storing the tracer events.
    *  Explanation of flags:
    *  FTRACE_CHUNK_FLAG_FREE : the chunk is free to use
    *  FTRACE_CHUNK_FLAG_CYC  : the chunk stores events in a cyclical way. 
    *                           When the chunk is full, 
    *                           it will continue from the head.
    *  FTRACE_CHUNK_FLAG_JTN  : when the chunk is full, 
    *                           it jumps to the next chunk.
    */
  a = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);
  b = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_JTN);
  c = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);

  /**
   *  Link the 3 chunks together. 
   *  It is important to link before you can jump from one to another.
   */
  FTrace_chunk_link(a,b);
  FTrace_chunk_link(b,c);

  /** Select the first chunk for saving the events. */
  FTrace_actual_chunk_select(a);

  kern_gettime(&start);
  
  /** Start the tracer. */
  FTrace_enable();
  
  /** Enable filtering for timer related events. */
  FTrace_set_filter(FTrace_filter_timer, 1);

  TRACER_LOGEVENT(FTrace_EVT_trace_start,proc_table[exec_shadow].context,clk_per_msec);

  for (i=0;i<10;i++)
    if (proc_table[i].context != 0) TRACER_LOGEVENT(FTrace_EVT_id,
					(unsigned short int)proc_table[i].context,i);

  for (i=0;i<1000000000;i++);
  
  /** Enable filtering for timer related events. */
  FTrace_set_filter(FTrace_filter_timer, 0);
 
  /** Change the chunk where the events are stored. */
  TRACER_LOGEVENT(FTrace_EVT_next_chunk,0,0);
  
  f = kern_fsave();
  __asm__("cpuid":::"eax","ebx","ecx","edx");
  FAST_TRACER_LOGEVENT(FTrace_EVT_ipoint,1000,0);
  FAST_TRACER_LOGEVENT(FTrace_EVT_ipoint,2000,0);
  FAST_TRACER_LOGEVENT(FTrace_EVT_ipoint,3000,0);
  FAST_TRACER_LOGEVENT(FTrace_EVT_ipoint,4000,0);
  FAST_TRACER_LOGEVENT(FTrace_EVT_ipoint,5000,0);
  __asm__("cpuid":::"eax","ebx","ecx","edx");
  kern_frestore(f);
  
  TRACER_LOGEVENT(FTrace_EVT_ipoint,6000,0);
 
  for (i=0;i<1000000000;i++);

  /** Store a TFrace stop event. */
  TRACER_LOGEVENT(FTrace_EVT_trace_stop,0,0);

  /** Stop the tracer. */
  FTrace_disable();
  kern_gettime(&end);

  SUBTIMESPEC(&end,&start,&diff);

  cprintf("Logged Time %d s %d us\n",(int)diff.tv_sec,(int)diff.tv_nsec/1000);

  /** Initialize the network for remotely saving the trace.  */
  FTrace_OSD_init_udp(1, "192.168.1.10", "192.168.1.1");
  
  /** 
   * You may want to save the events to disk. In that case, simply change
   * the network initialization instruction with the following line:
   *  
   * FTrace_init_disk_writer("trace.dat", 0, NULL, NULL);
   *
   */

  /** Save the chunk. */
  FTrace_send_chunk(a, 0, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);
  FTrace_send_chunk(b, 0, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_JTN);

  return 0;

}
