
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

#include "kernel/kern.h"
#include "calibrate.h"

int cal_cycles=0;

/* Runtime Calibration */
int calibrate_cycle()
{
  long long i;
  struct timespec start,end,diff;
  
  if (cal_cycles != 0) return 0;



  kern_cli();
  __asm__ __volatile__ ("xorl %%eax,%%eax\n\t"
                          "cpuid\n\t"
                          :::"eax","ebx","ecx","edx");
  kern_gettime(&start);
  for (i=0;i<CALIBRATION_DELTA;i++)
    __asm__ __volatile__ ("xorl %%eax,%%eax\n\t"
                          "cpuid\n\t"
                          :::"eax","ebx","ecx","edx");
  __asm__ __volatile__ ("xorl %%eax,%%eax\n\t"
                          "cpuid\n\t"
                          :::"eax","ebx","ecx","edx");
  kern_gettime(&end);
  kern_sti();
  
  SUBTIMESPEC(&end,&start,&diff);
  cal_cycles = TIMESPEC2USEC(&diff);
 
  cprintf("Calibration usec/[%d cycles] = %d\n",CALIBRATION_DELTA,cal_cycles);

  return 0;

}

void eat(TIME wait) {

    int exec_cycles=0;
    int i=0;
   
    exec_cycles = (long long)(wait) * CALIBRATION_DELTA / cal_cycles;

    /* Execution delay */
    for (i=0;i<exec_cycles;i++)
    __asm__ __volatile__ ("xorl %%eax,%%eax\n\t"
                          "cpuid\n\t"
                          :::"eax","ebx","ecx","edx");

}
