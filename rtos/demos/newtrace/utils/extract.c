
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
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#define MAXCONTEXT 100
#define MAXJOB 300000

#define INT_CTX 1
#define INT_PID 9999
#define PID_NO_DEF -1
#define SERVER_NO_DEF -1

#define BACKGROUND 0
#define PERIODICAL 1
#define INTERRUPT  2

#define DRAW_NUM 1000

#define DISABLE_PLOT

struct ctx_exec {
	int ctx;
	unsigned long long dtsc;
	unsigned long long dnsec;
	unsigned long long tsc_start;
	unsigned long long nsec_start;
};

struct ctx_to_pid {
	int ctx;
	int pid;
};

struct ctx_server {
        int ctx;
        int server_id;

};

struct task_event {
	int ctx;
	unsigned long long tsc;
	unsigned long long nsec;
};

struct server_event {
        int server_id;
        unsigned long long tsc;
        unsigned long long nsec;
};

void Error(int num, int line) {
	printf("Finite-State machine error %d at line %d\n",num,line);
	exit(2);
}

int context_total = 0,endcycle_total = 0,job_total = 0,exec_total = 0;
struct ctx_exec *exec_list;
struct ctx_to_pid *context_list;
struct task_event *endcycle_list;
struct task_event *deadline_miss_list;
struct task_event *wcet_miss_list;
struct task_event *act_list;
struct ctx_exec *job_list;
struct ctx_server *ctx_server_list;

int server_exec_total = 0, server_end_total=0, server_act_total=0, ctx_server_total = 0, server_total=0;
struct server_event *act_server_list;
struct server_event *server_budget_end_list;

int deadline_miss = 0, wcet_miss = 0, act_total = 0;

unsigned int clk_per_msec = 0;
unsigned int skip_clk_per_msec = 0;

unsigned long long log_start_tsc = 0;
unsigned long long log_end_tsc = 0;
unsigned long long total_tsc = 0;
unsigned long long total_nsec = 0;

/* Data for gnuplot external call */
int draw_data[DRAW_NUM+1];

int gnuplot_clear() {

   int i;

   for (i=0;i<DRAW_NUM;i++)
     draw_data[i] = 0;

   return 0;

}

int gnuplot_draw(char *title,unsigned long long max_limit,int type) {

#ifndef DISABLE_PLOT

   FILE *gnuplot_data, *gnuplot_command;
   char temp_name[30];
   int i,pid,*current_mem;

   current_mem = malloc(sizeof(int)*(DRAW_NUM+1));
   memcpy(current_mem,draw_data,sizeof(int)*(DRAW_NUM+1));

   pid = fork();
   if (pid == 0) {

     srand(getpid());

     sprintf(temp_name,"/tmp/pwcet%d",rand()%10000);

     gnuplot_data = fopen(temp_name,"w");
     gnuplot_command = popen("gnuplot -persist","w");

     for (i=0;i<DRAW_NUM;i++)
       fprintf(gnuplot_data,"%f\t%f\n",(double)i * (double)max_limit / (double)DRAW_NUM / 1000.0,(float)(current_mem[i]));

     fflush(gnuplot_data);
     fclose(gnuplot_data);

     fprintf(gnuplot_command,"set xlabel \"Time [us]\"\n");
     if (type == 0) {
	fprintf(gnuplot_command,"set ylabel \"Frequency [#]\"\n");

	fprintf(gnuplot_command,"plot \"%s\" using 1:2 title \"%s\" with lines\n",temp_name,title);
	fflush(gnuplot_command);
     } else {
       fprintf(gnuplot_command,"set ylabel \"Time [us]\"\n");

       fprintf(gnuplot_command,"plot \"%s\" using 1:2 title \"%s\" with lines\n",temp_name,title);
       fflush(gnuplot_command);
     }

     pclose(gnuplot_command);
     exit(0);

   }

#endif

   return 0;
  
}

int stats_from_execs(int ctx_num, unsigned long long *tot_nsec,
				unsigned long long *min_nsec, 
				unsigned long long *mean_nsec, 
				unsigned long long *max_nsec,
				unsigned long long *first_nsec, 
				int *number) {

   unsigned long long temp_nsec;
   int k,i;

   temp_nsec = 0;
   *max_nsec = 0;
   *mean_nsec = 0;
   *min_nsec = 0xFFFFFFFF;
   *first_nsec = 0;
   k = 0;
   for (i=0;i<exec_total;i++)
     if (exec_list[i].ctx == context_list[ctx_num].ctx) {
       if (*first_nsec == 0) *first_nsec = exec_list[i].nsec_start;
       if (exec_list[i].dnsec > *max_nsec) *max_nsec = exec_list[i].dnsec;
       if (exec_list[i].dnsec < *min_nsec) *min_nsec = exec_list[i].dnsec;
       temp_nsec += exec_list[i].dnsec;
       k++;
     }

   *number = k;
   *tot_nsec = temp_nsec;
   if (k != 0) *mean_nsec = temp_nsec / k;

   return 0;

} 

int stats_from_jobs(int ctx_num, unsigned long long *tot_nsec,
				unsigned long long *min_nsec, 
				unsigned long long *mean_nsec, 
				unsigned long long *max_nsec,
				unsigned long long *first_nsec, 
				int *number) {

   unsigned long long temp_nsec;
   int k,i;

   temp_nsec = 0;
   *max_nsec = 0;
   *mean_nsec = 0;
   *min_nsec = 0xFFFFFFFF;
   *first_nsec = 0;
   k = 0;
   for (i=0;i<job_total;i++)
     if (job_list[i].ctx == context_list[ctx_num].ctx) {
       if (*first_nsec == 0) *first_nsec = job_list[i].nsec_start;
       if (job_list[i].dnsec > *max_nsec) *max_nsec = job_list[i].dnsec;
       if (job_list[i].dnsec < *min_nsec) *min_nsec = job_list[i].dnsec;
       temp_nsec += job_list[i].dnsec;
       k++;
     }

   *number = k;
   *tot_nsec = temp_nsec;
   if (k != 0) *mean_nsec = temp_nsec / k;

   return 0;

}

int arr_stats_from_execs(int ctx_num, unsigned long long *min_nsec, 
				unsigned long long *mean_nsec, 
				unsigned long long *max_nsec) {

   unsigned long long last_start,temp_nsec,delta_start;
   int i,k;

   last_start = 0;
   temp_nsec = 0;
   *max_nsec = 0;
   *min_nsec = 0xFFFFFFFF;
   *mean_nsec = 0;
   k = 0;
   for (i=0;i<exec_total;i++)
     if (exec_list[i].ctx == context_list[ctx_num].ctx) {
	if (last_start == 0) {
		last_start = exec_list[i].nsec_start;
	} else {
		delta_start = exec_list[i].nsec_start - last_start;
        	if (delta_start > *max_nsec) *max_nsec = delta_start;
		if (delta_start < *min_nsec) *min_nsec = delta_start;
        	temp_nsec += delta_start;
        	k++;
		last_start = exec_list[i].nsec_start;
	}
      }

   if (k != 0) *mean_nsec = temp_nsec / k;

   return 0;

}

int arr_stats_from_jobs(int ctx_num, unsigned long long *min_nsec, 
				unsigned long long *mean_nsec, 
				unsigned long long *max_nsec) {

   unsigned long long last_start,temp_nsec,delta_start;
   int i,k;

   last_start = 0;
   temp_nsec = 0;
   *max_nsec = 0;
   *min_nsec = 0xFFFFFFFF;
   *mean_nsec = 0;
   k = 0;
   for (i=0;i<job_total;i++)
     if (job_list[i].ctx == context_list[ctx_num].ctx) {
	if (last_start == 0) {
		last_start = job_list[i].nsec_start;
	} else {
		delta_start = job_list[i].nsec_start - last_start;
        	if (delta_start > *max_nsec) *max_nsec = delta_start;
		if (delta_start < *min_nsec) *min_nsec = delta_start;
        	temp_nsec += delta_start;
        	k++;
		last_start = job_list[i].nsec_start;
	}
      }

   if (k != 0) *mean_nsec = temp_nsec / k;

   return 0;

}

int plot_exec_demand_function(int ctx_num, char *pidstr) {

   unsigned long long max_limit;
   char tmpstr[50];
   int i;

   gnuplot_clear();
                                                                                                                             
   max_limit = total_nsec;
                                                                                                                             
   for (i=0;i<exec_total;i++)
     if (exec_list[i].ctx == context_list[ctx_num].ctx) {
       int h1,h2,h3;
                                                                                                                             
       h1 = exec_list[i].nsec_start * DRAW_NUM / max_limit;
       h2 = (exec_list[i].nsec_start+exec_list[i].dnsec) * DRAW_NUM / max_limit;
       for (h3=h1;h3<h2;h3++)
         if (h3 <= DRAW_NUM) draw_data[h3] += exec_list[i].dnsec/1000*(h3-h1)/(h2-h1);
       for (h3=h2;h3<=DRAW_NUM;h3++)
         if (h3 <= DRAW_NUM) draw_data[h3] += exec_list[i].dnsec/1000;
                                                                                                                             
     }
                                                                                                                             
   sprintf(tmpstr,"Ctx [%d:%s] Demand-Function",context_list[ctx_num].ctx,pidstr);
   gnuplot_draw(tmpstr,max_limit,1);

   return 0;

}

int plot_exec_c_distrib(int ctx_num, unsigned long long max_nsec, char *pidstr) {

   unsigned long long max_limit;
   char tmpstr[50];
   int i,h;

   if (max_nsec == 0) return 0;

   gnuplot_clear();
   
   max_limit = max_nsec;

   for (i=0;i<exec_total;i++)
      if (exec_list[i].ctx == context_list[ctx_num].ctx) {
	h = exec_list[i].dnsec * DRAW_NUM / max_limit;
        if (h <= DRAW_NUM) draw_data[h]++;
      }

   sprintf(tmpstr,"Ctx [%d:%s] Exec C-dist",context_list[ctx_num].ctx,pidstr);
   gnuplot_draw(tmpstr,max_limit,0);

   return 0;

}

int plot_job_c_distrib(int ctx_num, unsigned long long max_nsec, char *pidstr) {

   unsigned long long max_limit;
   char tmpstr[50];
   int i,h;

   if (max_nsec == 0) return 0;

   gnuplot_clear();
                                                                                                                             
   max_limit = max_nsec;

   for (i=0;i<job_total;i++)
      if (job_list[i].ctx == context_list[ctx_num].ctx) {
	h = job_list[i].dnsec * DRAW_NUM / max_limit;
        if (h <= DRAW_NUM) draw_data[h]++;
      }

   sprintf(tmpstr,"Ctx [%d:%s] Job C-dist",context_list[ctx_num].ctx,pidstr);
   gnuplot_draw(tmpstr,max_limit,0);

   return 0;

}

int plot_exec_arr_distrib(int ctx_num, unsigned long long max_nsec, char *pidstr) {

  unsigned long long max_limit,last_start,delta_start;
  char tmpstr[50];
  int i,h;

  if (max_nsec == 0) return 0;

  gnuplot_clear();

  max_limit = max_nsec;

  last_start = 0;
  for (i=0;i<exec_total;i++)
    if (exec_list[i].ctx == context_list[ctx_num].ctx) {
      if (last_start == 0) {
              last_start = exec_list[i].nsec_start;
      } else {
              delta_start = exec_list[i].nsec_start - last_start;

	      h = delta_start * DRAW_NUM / max_limit;
       	      if (h <= DRAW_NUM) draw_data[h]++;

              last_start = exec_list[i].nsec_start;
      }
    }

  sprintf(tmpstr,"Ctx [%d:%s] Exec Arr.Delta",context_list[ctx_num].ctx,pidstr);
  gnuplot_draw(tmpstr,max_limit,0);

  return 0;

}

int plot_job_arr_distrib(int ctx_num, unsigned long long max_nsec, char *pidstr) {

  unsigned long long max_limit,last_start,delta_start;
  char tmpstr[50];
  int i,h;

  if (max_nsec == 0) return 0;

  gnuplot_clear();

  max_limit = max_nsec;

  last_start = 0;
  for (i=0;i<job_total;i++)
    if (job_list[i].ctx == context_list[ctx_num].ctx) {
      if (last_start == 0) {
              last_start = job_list[i].nsec_start;
      } else {
              delta_start = job_list[i].nsec_start - last_start;

	      h = delta_start * DRAW_NUM / max_limit;
       	      if (h <= DRAW_NUM) draw_data[h]++;

              last_start = job_list[i].nsec_start;
      }
    }

  sprintf(tmpstr,"Ctx [%d:%s] Job Arr.Delta",context_list[ctx_num].ctx,pidstr);
  gnuplot_draw(tmpstr,max_limit,0);

  return 0;

}

int create_lists(char *filename) {

  FILE *input_file;

  int type,par1,par2,k,i,state;

  int current_context = 0;
  int current_exec = 0;
  int current_endcycle = 0;

  int kill_delta = 0;

  unsigned long long last_tsc, tsc;
  unsigned long long current_nsec = 0;

  input_file = fopen(filename,"r");

  /* Memory alloc */
  exec_list = malloc(sizeof(struct ctx_exec) * MAXJOB);
  context_list = malloc(sizeof(struct ctx_to_pid) * MAXCONTEXT);
  ctx_server_list = malloc(sizeof(struct ctx_server) * MAXCONTEXT);

  endcycle_list = malloc(sizeof(struct task_event) * MAXJOB);
  deadline_miss_list = malloc(sizeof(struct task_event) * MAXJOB);
  wcet_miss_list = malloc(sizeof(struct task_event) * MAXJOB);
  act_list = malloc(sizeof(struct task_event) * MAXJOB);

  act_server_list = malloc(sizeof(struct server_event) * MAXJOB );
  server_budget_end_list = malloc(sizeof(struct server_event) * MAXJOB);


  /* Finite-State machine
   *
   * FS-Machine states:
                                                                                                                             
        0 - Start
        1 - Context running
        2 - Interrupt running
                                                                                                                             
        10 - End
                                                                                                                             
   */

  for(i=0;i<MAXCONTEXT;i++) {
    context_list[i].ctx = 0;
    context_list[i].pid = PID_NO_DEF;
  }

  for(i=0; i<MAXCONTEXT; i++) {
    ctx_server_list[i].ctx=0;
    ctx_server_list[i].server_id=SERVER_NO_DEF;
    
  }

  /* The start context + interrupt context */
  context_total = 2;
  current_context = 0;
  last_tsc = 0;
  context_list[0].ctx = 0;
  context_list[0].pid = PID_NO_DEF;
  context_list[1].ctx = INT_CTX;
  context_list[1].pid = INT_PID;

  state = 0;

  k = 0;
  while(!feof(input_file)) {

    fscanf(input_file,"%d %llu",&type,&tsc);
    k++;

    switch (type) {

	case 1:

		/* No par */
		break;

	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
	case 8:
	case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
		/* 1 par */
		fscanf(input_file,"%d",&par1);
		break;

	case 5:
	case 9:
	case 0:
	case 16:
        case 17:
	case 20:
	case 21:

		/* 2 par */
		fscanf(input_file,"%d %d",&par1,&par2);
		break;

    }

    switch (type) {

	case 0:
		if (state != 0) Error(1,k);
		printf("EVT:Log starts at [%12llu]\n",tsc);
		last_tsc = tsc;
		log_start_tsc = tsc;
		current_nsec = 0;

		if (par1 == 0) Error(11,k);
		if (par2 == 0) Error(12,k);

		current_context = par1;
                                                                                                                             
                for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) break;
                if (i == context_total) {
                        context_list[context_total].ctx = par1;
                        context_total++;
                }

		clk_per_msec = par2;

		exec_list[current_exec].tsc_start = tsc;
		exec_list[current_exec].nsec_start = current_nsec;
		state = 1;
		break;

	case 1:
		printf("EVT:Log   ends at [%12llu]\n",tsc);
		exec_list[current_exec].dtsc = tsc - last_tsc;
		exec_list[current_exec].dnsec = exec_list[current_exec].dtsc * 1000000 / clk_per_msec;
		current_nsec += exec_list[current_exec].dnsec;
                exec_list[current_exec].ctx = current_context;
                current_exec++;
		last_tsc = tsc;
		log_end_tsc = tsc;
		total_nsec = current_nsec;
		state = 10;
		break;
	
	/* Int start */
	case 2:
		if (state == 0) Error(2,k);
		exec_list[current_exec].dtsc = tsc - last_tsc;
		exec_list[current_exec].dnsec = exec_list[current_exec].dtsc * 1000000 / clk_per_msec;
		current_nsec += exec_list[current_exec].dnsec;
		exec_list[current_exec].ctx = current_context;
		current_exec++;
		last_tsc = tsc;
		current_context = INT_CTX;
		exec_list[current_exec].tsc_start = tsc - log_start_tsc;
		exec_list[current_exec].nsec_start = current_nsec;
		state = 2;
		break;

	/* Int end */
	case 3:
		if (state != 2) Error(3,k);		
		exec_list[current_exec].dtsc = tsc - last_tsc;
		exec_list[current_exec].dnsec = exec_list[current_exec].dtsc * 1000000 / clk_per_msec;	
		current_nsec += exec_list[current_exec].dnsec;
		exec_list[current_exec].ctx = current_context;
		current_exec++;
		last_tsc = tsc;
		if (par1 > 16) {
		  current_context = par1;

		  for (i=0;i<context_total;i++)
			if (par1 == context_list[i].ctx) break;
		  if (i == context_total) {
			context_list[context_total].ctx = par1;
			context_total++;
		  }
		}

		exec_list[current_exec].tsc_start = tsc;
		exec_list[current_exec].nsec_start = current_nsec;
		state = 1;
		break;

	/* Change ctx */
	case 4:

		exec_list[current_exec].dtsc = tsc - last_tsc;
		exec_list[current_exec].dnsec = exec_list[current_exec].dtsc * 1000000 / clk_per_msec;
		current_nsec += exec_list[current_exec].dnsec;
		exec_list[current_exec].ctx = current_context;
		current_exec++;
		last_tsc = tsc;
		current_context = par1;

                for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) break;
                if (i == context_total) {
                        context_list[context_total].ctx = par1;
                        context_total++;
                }

		exec_list[current_exec].tsc_start = tsc;
		exec_list[current_exec].nsec_start = current_nsec;
		state = 1;
		break;

	/* Task create */
	case 5:

		for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) {
				context_list[i].pid = par2;
				break;
			}
                if (i == context_total) {
                        context_list[context_total].ctx = par1;
			context_list[context_total].pid = par2;
                        context_total++;
                }

		break;
	
	/* Task kill */
	case 7:

		for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) break;
		if (i == context_total) Error(5,k);
		  else {

			kill_delta += 1000;

			for (k=0;k<current_endcycle;k++)
				if (endcycle_list[k].ctx == par1)
					endcycle_list[k].ctx += kill_delta;
			for (k=0;k<current_exec;k++)
				if (exec_list[k].ctx == par1)
					exec_list[k].ctx += kill_delta;
			context_list[context_total].ctx = context_list[i].ctx + kill_delta;
                        context_list[context_total].pid = context_list[i].pid;
                        context_total++;

			if (current_context == par1) current_context += kill_delta;

		  }

		break;

	/* Task endcycle */
	case 8:

		for (i=0;i<context_total;i++)
			if (par1 == context_list[i].ctx) break;
		if (i == context_total) Error(4,k);
		
		endcycle_list[current_endcycle].ctx = par1;
		endcycle_list[current_endcycle].tsc = tsc;
		endcycle_list[current_endcycle].nsec = current_nsec + (tsc-last_tsc) * 1000000 / clk_per_msec;
		current_endcycle++;

		break;

	/* Task activate */
        case 6:
                                                                                                                             
                act_list[act_total].ctx = par1;
                act_list[act_total].tsc = tsc;
                act_list[act_total].nsec = current_nsec + (tsc-last_tsc) * 1000000 / clk_per_msec;
                act_total++;
                                                                                                                             
                break;
	/* Server Create */
        case 16:
	        for (i=0;i<ctx_server_total;i++)
                        if (par1 == ctx_server_list[i].ctx) {
				ctx_server_list[i].server_id = par2;
				break;
			}
                if (i == ctx_server_total) {
                        ctx_server_list[ctx_server_total].ctx = par1;
			ctx_server_list[ctx_server_total].server_id = par2;
                        ctx_server_total++;
                }
                printf("(%d, %d, %d)", ctx_server_total, par1, par2);
		break;
	        
	/* Deadline miss */
        case 20:
                                                                                                                             
                for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) break;
                //if (i == context_total) Error(4,k);
                                                                                                                             
                deadline_miss_list[deadline_miss].ctx = par1;
                deadline_miss_list[deadline_miss].tsc = tsc;
                deadline_miss_list[deadline_miss].nsec = current_nsec + (tsc-last_tsc) * 1000000 / clk_per_msec;
                deadline_miss++;
                                                                                                                             
                break;

	/* Wcet miss */
        case 21:
                                                                                                                             
                for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) break;
                //if (i == context_total) Error(4,k);
                                                                                                                             
                wcet_miss_list[wcet_miss].ctx = par1;
                wcet_miss_list[wcet_miss].tsc = tsc;
                wcet_miss_list[wcet_miss].nsec = current_nsec + (tsc-last_tsc) * 1000000 / clk_per_msec;
                wcet_miss++;
                                                                                                                             
                break;

	/* Task id */
        case 9:
                                                                                                                             
                for (i=0;i<context_total;i++)
                        if (par1 == context_list[i].ctx) {
				context_list[i].pid = par2;
				break;
			}	
                if (i == context_total) {
                        context_list[context_total].ctx = par1;
                        context_list[context_total].pid = par2;
                        context_total++;
                }
                                                                                                                             
                break;

	case 10:

		exec_list[current_exec].dtsc = tsc - last_tsc;
                exec_list[current_exec].dnsec = exec_list[current_exec].dtsc * 1000000 / clk_per_msec;
                current_nsec += exec_list[current_exec].dnsec;
                exec_list[current_exec].ctx = current_context;
                current_exec++;
                last_tsc = tsc;
                                                                                                                             
                exec_list[current_exec].tsc_start = tsc;
                exec_list[current_exec].nsec_start = current_nsec;

		if (!skip_clk_per_msec) clk_per_msec = par1;

		break;

    }

    if (current_exec == MAXJOB-1) {
	printf("Too many execs...\n");
	exit(3);
    }

    if (current_endcycle == MAXJOB-1 || act_total == MAXJOB-1
	|| deadline_miss == MAXJOB-1 || wcet_miss == MAXJOB-1) {
        printf("Too many jobs...\n");
        exit(4);
    }

    if (state == 10) break;

  }

  endcycle_total = current_endcycle;
  exec_total = current_exec;

  return k;

}

int task_events(int num, int *act, int *dlinemiss, int *wcetmiss) {

  unsigned long long i;
  int tmp = 0;

  *act = 0;
  *dlinemiss = 0;
  *wcetmiss = 0;

  tmp = 0;
  for (i=0;i<act_total;i++)
    if (act_list[i].ctx == context_list[num].ctx) tmp++;
  *act = tmp;
  tmp = 0;
  for (i=0;i<deadline_miss;i++)
    if (deadline_miss_list[i].ctx == context_list[num].ctx) tmp++;
  *dlinemiss = tmp;
  tmp = 0;
  for (i=0;i<wcet_miss;i++)
    if (wcet_miss_list[i].ctx == context_list[num].ctx) tmp++;
  *wcetmiss = tmp;

  return 0;

}

int create_job_list() {

  int current_job = 0, h, i, k;
  int temp_ctx;
  unsigned long long temp_nsec, endcycle_start_nsec;
  unsigned long long temp_tsc, endcycle_end_nsec;

  job_list = malloc(sizeof(struct ctx_exec) * MAXJOB);

  for (k=0;k<context_total;k++) {

    temp_ctx = context_list[k].ctx;
    endcycle_start_nsec = 0;

    for (h=0;h<endcycle_total;h++) {

      if (endcycle_list[h].ctx == temp_ctx) {

	if (endcycle_start_nsec == 0)
	  endcycle_start_nsec = 0;

	endcycle_end_nsec = endcycle_list[h].nsec;
	temp_nsec = 0;
	temp_tsc = 0;

  	job_list[current_job].nsec_start = 0;

	for(i=0;i<exec_total;i++)
		if (exec_list[i].ctx == temp_ctx) {
			if (exec_list[i].nsec_start < endcycle_end_nsec &&
				exec_list[i].nsec_start >= endcycle_start_nsec) { 
				if (job_list[current_job].nsec_start == 0)
				  job_list[current_job].nsec_start = exec_list[i].nsec_start;
				temp_nsec += exec_list[i].dnsec;
				temp_tsc += exec_list[i].dtsc;
			}
		}

	job_list[current_job].dtsc = temp_tsc;
	job_list[current_job].dnsec = temp_nsec;
	job_list[current_job].ctx = temp_ctx;
	current_job++;

	endcycle_start_nsec = endcycle_end_nsec;

      }

    }

  }	

  job_total = current_job;

  return 0;

}

int elaborate_statistics(int num, int task_type) {

  char pidstr[10];
  char serverstr[10];
  unsigned long long tot_nsec,mean_nsec,max_nsec,min_nsec,first_nsec;
  int number,act,dlinemiss,wcetmiss;
  int i=0;

  switch (context_list[num].pid) {
      case PID_NO_DEF:
        sprintf(pidstr,"NODEF");
        break;
      case INT_PID:
        sprintf(pidstr,"  INT");
        break;
      default:
        sprintf(pidstr,"%5d",context_list[num].pid);
        break;
  }

  for (i=0; i<ctx_server_total; i++) {
    if (ctx_server_list[i].ctx==context_list[num].ctx) {
      sprintf(serverstr,"%5d", ctx_server_list[i].server_id);
      break;
    }
  }
  
  if (i==ctx_server_total)
    sprintf(serverstr, "NODEF");


  if (task_type == BACKGROUND) {

    printf("Background Task CTX [%5d] PID [%s] SERVER [%s]\n",context_list[num].ctx,pidstr, serverstr);

    stats_from_execs(num,&tot_nsec,&min_nsec,&mean_nsec,&max_nsec,&first_nsec,&number);    

    if (number > 0) {

      printf("  Total Execution        [%12llu ns]\n",tot_nsec);
      printf("  Mean  CPU Bandwidth    [%11f%c   ]\n",(double)(tot_nsec)/(double)(total_nsec)*100.0,'%');
      printf("    after first exec     [%11f%c   ]\n",(double)(tot_nsec)/(double)(total_nsec-first_nsec)*100.0,'%');
      printf("  Execs Number           [%12d   ]\n",number);
      printf("  Min  Exec              [%12llu ns]\n",min_nsec);
      printf("  Mean Exec              [%12llu ns]\n",mean_nsec);
      printf("  Max  Exec              [%12llu ns]\n\n",max_nsec);

      plot_exec_demand_function(num,pidstr);

    } else {
                                                                                                                             
      printf("  Total Execution        [%12llu ns]\n\n",tot_nsec);
                                                                                                                             
    }

  }

  if (task_type == INTERRUPT) {

    printf("Interrupts\n");

    stats_from_execs(num,&tot_nsec,&min_nsec,&mean_nsec,&max_nsec,&first_nsec,&number);

    if (number > 0) {

      printf("  Total Execution        [%12llu ns]\n",tot_nsec);
      printf("  Mean  CPU Bandwidth    [%11f%c   ]\n",(double)(tot_nsec)/(double)(total_nsec)*100.0,'%');
      printf("    after first int      [%11f%c   ]\n",(double)(tot_nsec)/(double)(total_nsec-first_nsec)*100.0,'%');
      printf("  Interrupts Number      [%12d   ]\n",number);
      printf("  Min  Interrupt         [%12llu ns]\n",min_nsec);
      printf("  Mean Interrupt         [%12llu ns]\n",mean_nsec);
      printf("  Max  Interrupt         [%12llu ns]\n\n",max_nsec);

      plot_exec_demand_function(num,pidstr);

      plot_exec_c_distrib(num,max_nsec,pidstr);

      arr_stats_from_execs(num,&min_nsec,&mean_nsec,&max_nsec);

      if (max_nsec > 0) {

        printf("  Min  Arr. Delta        [%12llu ns]\n",min_nsec);
        printf("  Mean Arr. Delta        [%12llu ns]\n",mean_nsec);
        printf("  Max  Arr. Delta        [%12llu ns]\n\n",max_nsec);

        plot_exec_arr_distrib(num,max_nsec,pidstr);

      }

    } else {

      printf("  Total Execution        [%12llu ns]\n\n",tot_nsec);

    }

  }

  if (task_type == PERIODICAL) {

    printf("Periodical Task CTX [%5d] PID [%s] SERVER [%s]\n",context_list[num].ctx,pidstr,serverstr);    

    stats_from_execs(num,&tot_nsec,&min_nsec,&mean_nsec,&max_nsec,&first_nsec,&number);

    if (number > 0) {

      printf("  Total Execution        [%12llu ns]\n",tot_nsec);
      printf("  Mean  CPU Bandwidth    [%11f%c   ]\n",(double)(tot_nsec)/(double)(total_nsec)*100.0,'%');
      printf("    after first exec     [%11f%c   ]\n",(double)(tot_nsec)/(double)(total_nsec-first_nsec)*100.0,'%');
      printf("  Execs Number           [%12d   ]\n",number);
      printf("  Min  Exec              [%12llu ns]\n",min_nsec);
      printf("  Mean Exec              [%12llu ns]\n",mean_nsec);
      printf("  Max  Exec              [%12llu ns]\n\n",max_nsec);    

      plot_exec_demand_function(num,pidstr);

      stats_from_jobs(num,&tot_nsec,&min_nsec,&mean_nsec,&max_nsec,&first_nsec,&number);   

      printf("  Total Job Exec         [%12llu ns]\n",tot_nsec);
      printf("  Jobs Number            [%12d   ]\n",number);
      printf("  Min  Job               [%12llu ns]\n",min_nsec);
      printf("  Mean Job               [%12llu ns]\n",mean_nsec);
      printf("  Max  Job               [%12llu ns]\n\n",max_nsec);

      task_events(num,&act,&dlinemiss,&wcetmiss);

      printf("  Activations            [%12d   ]\n",act);
      printf("  Deadline Miss          [%12d   ]\n",dlinemiss);
      printf("  Wcet Miss              [%12d   ]\n\n",wcetmiss);

      plot_job_c_distrib(num,max_nsec,pidstr);

      arr_stats_from_jobs(num,&min_nsec,&mean_nsec,&max_nsec);

      if (max_nsec > 0) {

        printf("  Min  Arr. Delta        [%12llu ns]\n",min_nsec);
        printf("  Mean Arr. Delta        [%12llu ns]\n",mean_nsec);
        printf("  Max  Arr. Delta        [%12llu ns]\n\n",max_nsec);
  
        plot_job_arr_distrib(num,max_nsec,pidstr);

      }

    } else {

      printf("  Total Execution       [%12llu ns]\n\n",tot_nsec);

    }

  }

  return 0;

}

int main(int argc, char *argv[]) {

  int events_total,k,i;
  int task_type;

  unsigned long long temp_nsec;

  srand(getpid());

  if (argc < 2) {
    printf("%s: Enter the input file name \"%s filename.pwc [clk_per_msec]\"\n",argv[0],argv[0]);
    exit(1);
  }

  printf("\n");

  if (argc == 3) {
    skip_clk_per_msec = 1;
    clk_per_msec = atoi(argv[2]);
    printf("Clk/msec = %u\n\n",clk_per_msec);
  }
  
  events_total = create_lists(argv[1]);

  total_tsc = log_end_tsc - log_start_tsc;

  printf("\nTotal dTSC [%12llu] ns [%12llu]\n", total_tsc, total_nsec);
  printf("Events     [%12d]\n",events_total);
  printf("Execs      [%12d]\n",exec_total);
  printf("EndCycles  [%12d]\n",endcycle_total);
  printf("Dline miss [%12d]\n",deadline_miss);
  printf("WCET miss  [%12d]\n",wcet_miss);

  printf("\nPreemption Removing.... \n");

  /* Remove preemption from the computation time */
  create_job_list();

  temp_nsec = 0;
  for (i=0;i<job_total;i++)
    temp_nsec += job_list[i].dnsec;

  printf("Total nsec of jobs [%12llu]\n",temp_nsec);

  temp_nsec = 0;
  for (i=0;i<exec_total;i++)
    temp_nsec += exec_list[i].dnsec;
                                                                                                                             
  printf("Total nsec of exec [%12llu]\n",temp_nsec);
  printf("Total nsec considering last clk/msec [%12llu]\n",total_tsc*1000000/clk_per_msec);

  printf("\nCompute Task Statistics.... \n\n");

  for (i=0;i<context_total;i++) {

	task_type = BACKGROUND;

	if (context_list[i].ctx == INT_CTX) task_type = INTERRUPT;

	for (k=0;k<job_total;k++)
		if (job_list[k].ctx == context_list[i].ctx) {
		  task_type = PERIODICAL;
		  break;
		}

	elaborate_statistics(i,task_type);

  }

  return 0;

}

