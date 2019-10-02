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

#include <kernel/kern.h>
#include <tracer.h>

int main(int argc, char **argv)
{

  long long i;

  int a,b,c;
  struct timespec start,end,diff;

  a = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);
  b = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_JTN);
  c = FTrace_chunk_create(1000000, 1000000, FTRACE_CHUNK_FLAG_FREE | FTRACE_CHUNK_FLAG_CYC);

  FTrace_chunk_link(a,b);
  FTrace_chunk_link(b,c);

  FTrace_actual_chunk_select(a);

  kern_gettime(&start);
  FTrace_enable();

  for (i=0;i<1000000000;i++);
 
  TRACER_LOGEVENT(FTrace_EVT_next_chunk,0,0);

  for (i=0;i<1000000000;i++);

  FTrace_disable();
  kern_gettime(&end);

  SUBTIMESPEC(&end,&start,&diff);

  cprintf("Logged Time %d s %d us\n",(int)diff.tv_sec,(int)diff.tv_nsec/1000);

  FTrace_chunk_dump(a);

  return 0;

}
