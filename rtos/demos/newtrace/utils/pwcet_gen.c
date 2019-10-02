/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *    Giacomo Guidi      <giacomo@gandalf.sssup.it>
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <FTrace.h>

#define READ_BUFFER 2000
#define DELTA_BUFFER 100

int main(int argc, char *argv[])
{

  char buffer[READ_BUFFER+DELTA_BUFFER]; 
  void *p, *last;
  int n,delta,size,tsc;

  unsigned long long ev = 0, tsc_value;

  FILE *input_file; 
  
  if (argc < 2) {
    printf("%s: Enter the input file name [%s filename]\n",argv[0],argv[0]);
    exit(1);
  }

  input_file = fopen(argv[1],"rb");

  last = buffer + READ_BUFFER;

  while(!feof(input_file)) {
 
    //move remaining byte
    delta = (unsigned int)(buffer) + READ_BUFFER - (unsigned int)(last);
    if (delta > 0) memcpy(buffer,last,delta);    

    n = fread(buffer+delta,1,READ_BUFFER+10,input_file);
    fseek(input_file,-(delta+10),SEEK_CUR);

    p = buffer;
    /*
    // specific server events
    #define FTrace_EVT_server_create      0x08 // Par1 [empty] Par2 [server]
    #define FTrace_EVT_server_replenish   0x18 // Par1 [empty] Par2 [server]
    #define FTrace_EVT_server_exhaust     0x28 // Par1 [empty] Par2 [server]
    #define FTrace_EVT_server_reclaiming  0x38 // Par1 [empty] Par2 [server]
    #define FTrace_EVT_server_remove      0x48 // Par1 [empty] Par2 [server]
    #define FTrace_EVT_server_active      0x58 // Par1 [empty] Par2 [server]
    #define FTrace_EVT_server_using_rec   0x68 // Par1 [reclaiming] Par2 [server]
    */

    while ((unsigned int)(p) + 16 <= (unsigned int)(buffer + READ_BUFFER) && 
	   (unsigned int)(p) + 16 <= (unsigned int)(buffer + n + delta)) {

	tsc = 0;

	switch (*(unsigned short int *)(p) & 0x00FF) {

	  case FTrace_EVT_ipoint:
		printf("%d ",*(unsigned short int *)(p+2));
		tsc = 1;
		break;

	  /* Tracer start */
          case FTrace_EVT_trace_start:
                printf("0 ");
		tsc = 1;
                break;

	  /* Tracer stop */
          case FTrace_EVT_trace_stop:
                printf("1 ");
		tsc = 1;
                break;

	  /* Interrupt start (int num) */
	  case FTrace_EVT_timer_wakeup_start:
	  case FTrace_EVT_interrupt_start:
                printf("2 ");
		tsc = 1;
                break;

          /* Interrupt Stop (int num, or context num is int 3) */
	  case FTrace_EVT_timer_wakeup_end:
          case FTrace_EVT_interrupt_end:
                printf("3 ");
		tsc = 1;
                break;

	  /* Context to (context num) */
	  case FTrace_EVT_context_switch:
                printf("4 ");
		tsc = 1;
                break;

	  /* Task Create (context num, pid num) */
	  case FTrace_EVT_task_create:
		printf("5 ");
		tsc = 1;
		break;

	  /* Task Activate (context num) */
	  case FTrace_EVT_task_activate:
		printf("6 ");
		tsc = 1;
		break;
	
	  /* Task End (context num) */
	  case FTrace_EVT_task_end:
		printf("7 ");
		tsc = 1;
		break;

	  /* Task End-Cycle (context num) */
	  case FTrace_EVT_task_end_cycle:
		printf("8 ");
		tsc = 1;
		break;

	  /* Task Context - PID declaration */
	  case FTrace_EVT_id:
		printf("9 ");
		tsc = 1;
		break;

	  case FTrace_EVT_cycles_per_msec:
		printf("10 ");
		tsc = 1;
                break;
		// specific server events

          case FTrace_EVT_server_active:
                printf("16 ");
                tsc=1;
                break;

	  case FTrace_EVT_task_deadline_miss:
		printf("20 ");
		tsc = 1;
		break;

	  case FTrace_EVT_task_wcet_violation:
                printf("21 ");
                tsc = 1;
                break;

	}

        tsc_value = (unsigned long long)(*(unsigned int *)(p+4)) << 32;
        tsc_value += (unsigned long long)(*(unsigned int *)(p+8));

	if (tsc == 1) {

	  printf("%llu ",tsc_value);

	  switch (*(unsigned short int *)(p) & 0x00FF) {

	    case FTrace_EVT_trace_start:
		printf("%d %d ",*(unsigned short int *)(p+2),*(unsigned int *)(p+12));
		break;

            case FTrace_EVT_interrupt_start:
                printf("%d ",*(unsigned short int *)(p+2));
                break;
	
	    case FTrace_EVT_timer_wakeup_start:
                printf("%d ",0);
                break;
          
	    case FTrace_EVT_timer_wakeup_end:
	    case FTrace_EVT_interrupt_end:
                printf("%d ",*(unsigned short int *)(p+2));
                break;

            case FTrace_EVT_context_switch:
                printf("%d ",*(unsigned short int *)(p+2));
                break;

	    case FTrace_EVT_task_create:
		printf("%d %d",*(unsigned short int *)(p+2),*(unsigned int *)(p+12));
		break;

	    case FTrace_EVT_task_deadline_miss:
		printf("%d %d",*(unsigned short int *)(p+2),*(unsigned int *)(p+12));
		break;
	
	    case FTrace_EVT_task_wcet_violation:
                printf("%d %d",*(unsigned short int *)(p+2),*(unsigned int *)(p+12));
                break;

	    case FTrace_EVT_task_activate:
		printf("%d ",*(unsigned short int *)(p+2));
		break;

	    case FTrace_EVT_task_end_cycle:
		printf("%d ",*(unsigned short int *)(p+2));
		break;

	    case FTrace_EVT_task_end:
		printf("%d ",*(unsigned short int *)(p+2));
		break;

	    case FTrace_EVT_id:
		printf("%d %d",*(unsigned short int *)(p+2),*(unsigned int *)(p+12));
		break;

	    case FTrace_EVT_cycles_per_msec:
                printf("%d ",*(unsigned int *)(p+12));
                break;
	  
  	    case FTrace_EVT_server_active:
	        printf("%d %d",*(unsigned short int *)(p+2),*(unsigned int *)(p+12));
		break;
	  }

	  printf("\n");

	}

	size = 16;

      ev++;
      
      p += 16;

      if ((unsigned int)(p) + 10 > (unsigned int)(buffer + n + delta)) break; 

      last = p;
 
    }

    if ((unsigned int)(p) + 10 > (unsigned int)(buffer + n + delta)) break;
      
  }

  fclose(input_file);

  return 0;

}

