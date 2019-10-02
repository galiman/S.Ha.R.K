/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi	 <giacomo@gandalf.sssup.it>
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
 */

/*
 * Advanced Timer Demo
 *
 */

#include "kernel/kern.h"
#include "ll/i386/cons.h"
#include "ll/i386/advtimer.h"

#include "drivers/shark_keyb26.h"
#include "drivers/shark_cpu26.h"

#define UPDATE_PERIOD 	10000
#define UPDATE_WCET 	1000

extern unsigned int clk_per_msec;
extern unsigned int apic_clk_per_msec;

int freq_number = 0;
int cur_frequency = 0;
int cur_freq_number = 0;
int dvs_status = 0;

TASK Update(void *arg)
{  
  struct timespec actual_timer;
  
  long nsec,sec,min,hrs,day;
  long mean_delay,tot_delay,num_delay;

  signed long long start,end,res;
  struct timespec s_test,startts,endts;

  /* Get the DVS status */
  int i;
  char dvs_frequencies[1000];

  dvs_status = CPU26_DVS_installed();

  if (dvs_status > 0) {

	printf_xy(0,11,WHITE,"Dynamic Frequency Scaling Supported");
	printf_xy(0,12,WHITE,"Press \"q\" to raise or \"a\" to lower CPU freq");

	CPU26_show_frequencies(dvs_frequencies);
	freq_number = CPU26_get_frequencies(cpu26_freqs);
	if (freq_number > 0) {
		printf_xy(0,13,WHITE,"Supported Frequencies [ %s: %d ]",dvs_frequencies,freq_number);
	} else {
		printf_xy(0,13,WHITE,"Not Pre-Defined Frequencies");
	}
	cur_frequency = CPU26_get_cur_frequency();
	printf_xy(0,14,WHITE,"Current Frequency [ %10d ]",cur_frequency);

	for (i=0;i<freq_number;i++)
		if (cpu26_freqs[i] == cur_frequency) break;
	cur_freq_number = i;

  } else {

	printf_xy(0,11,WHITE,"Dynamic Frequency Scaling NOT Supported");

  }

  task_nopreempt();

  num_delay = tot_delay = mean_delay = 0;

  while (1) {

        if (clk_per_msec != 0) {

          rdtscll(start);
	  sys_gettime(&actual_timer);
	  rdtscll(end);
	  res = end - start;
	  rdtscll(start);
	  rdtscll(end);
	  res -= (end - start);
	  s_test.tv_nsec = res * 1000000 / clk_per_msec;


        } else {

          sys_gettime(&startts);
	  sys_gettime(&actual_timer);
	  sys_gettime(&endts);
	  SUBTIMESPEC(&endts,&startts,&s_test);
	  sys_gettime(&startts);
	  sys_gettime(&endts);
	  SUBTIMESPEC(&endts,&startts,&endts);
          SUBTIMESPEC(&s_test,&endts,&s_test);

        }

        if (tot_delay < 1000000000) {
          tot_delay += s_test.tv_nsec;
          num_delay ++;
          mean_delay = tot_delay / num_delay;
        }
          
	nsec = actual_timer.tv_nsec;
	sec = actual_timer.tv_sec;
	min = sec / 60;
	sec %= 60;
	hrs = min / 60;
	min %= 60;
	day = hrs / 24;
	hrs %= 24;
	
	printf_xy(0,5,WHITE,"Actual CPU Clk/msec:  %12d",clk_per_msec);
        printf_xy(0,6,WHITE,"Actual APIC Clk/msec: %12d",apic_clk_per_msec);  
	printf_xy(0,7,WHITE,"Actual Timer: %2ld d %2ld h %2ld m %2ld s %12ld ns",day,hrs,min,sec,(long)nsec);

	printf_xy(0,9,WHITE,"Timer Access Delay: %12ld ns",mean_delay);

	if (dvs_status > 0)
	  printf_xy(0,14,WHITE,"Current Frequency [ %10d ]",CPU26_get_cur_frequency());
	
	task_endcycle();

  }

}

void set_screen()
{

	clear();

	printf_xy(20,0,WHITE,"          Advanced Timer Demo           ");
	printf_xy(20,1,WHITE,"Giacomo Guidi <giacomo@gandalf.sssup.it>");
	printf_xy(20,2,WHITE,"         Press Ctrl + C to exit          ");
	
}

void program_key_end(KEY_EVT* e)
{
        exit(1);
}

void up_freq(KEY_EVT* e)
{

	if (dvs_status > 0 && freq_number > 0) {
	  cur_freq_number++;
	  if (cur_freq_number >= freq_number) cur_freq_number--;
	  CPU26_set_frequency(cpu26_freqs[cur_freq_number],DVS_RELATION_H);
	}
}

void down_freq(KEY_EVT* e)
{
	if (dvs_status > 0 && freq_number > 0) {
	  cur_freq_number--;
          if (cur_freq_number < 0) cur_freq_number++;
          CPU26_set_frequency(cpu26_freqs[cur_freq_number],DVS_RELATION_L);
        }
}
				
int main(int argc, char **argv)
{
	
  HARD_TASK_MODEL   mp; //Show current setting
  PID               update;
  KEY_EVT	    k;
 
  k.flag = CNTL_BIT;
  k.scan = KEY_C;
  k.ascii = 'c';
  k.status = KEY_PRESSED;
  keyb_hook(k,program_key_end,FALSE);

  k.flag = CNTR_BIT;
  k.status = KEY_PRESSED;
  keyb_hook(k,program_key_end,FALSE);

  k.flag = 0;
  k.scan = KEY_Q;
  k.ascii = 'q';
  k.status = KEY_PRESSED;
  keyb_hook(k,up_freq,FALSE);

  k.flag = 0;
  k.scan = KEY_A;
  k.ascii = 'a';
  k.status = KEY_PRESSED;
  keyb_hook(k,down_freq,FALSE);

  set_screen();
  
  hard_task_default_model(mp);
  hard_task_def_ctrl_jet(mp);
  hard_task_def_group(mp, 1);
  hard_task_def_wcet(mp,UPDATE_WCET);
  hard_task_def_mit(mp,UPDATE_PERIOD);
  hard_task_def_usemath(mp);
  update = task_create("Update", Update, &mp, NULL);
  if (update != NIL) task_activate(update); 

  return 0;

}
