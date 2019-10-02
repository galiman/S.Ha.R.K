///////////////////////////////////////////////////
// app.c		-Lex Nahumury 2006
// 
// This would be the Closed Source User Application part.
// The ELF object itself contains no GPL code.
// The references to GPL code are resolved
// by the Dynamic Linker after the kernel has loaded.
//
//////////////////////////////////////////////////

/*
Usage example:

# For booting SHARK image from HD
title  S.H.A.R.K + Boot Modules from HD 0,0
kernel (hd0,0)/boot/os   mount root=/dev/hda1
module (hd0,0)/boot/sh_app.bin
module (hd0,0)/boot/data.bin

*/

#include "kernel/kern.h"
#include <kernel/func.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ll/i386/cons.h"

#include <drivers/shark_keyb26.h>
#include "../../ports/dynalink/dynalink.h"

extern void call_shutdown_task(void *arg);
extern DWORD get_tick();

struct params
{
	int task_nr;
	int row;
	int count;
	int cycles;
};


TASK periodic_task(void *arg)
{
	int ani=0;
	char c[2];
	struct params* p = (struct params*)(arg);
	int row = p->row;
	char txt[4];
	p->count=0;

	while(p->count < p->cycles)
	{
		switch(ani)
		{
		case 0:
			ani=1;	sprintf(c,"%s", "\\");
			break;
		case 1:
			ani=2;	sprintf(c,"/");
			break;			
		case 2:
			ani=0;	sprintf(c,"-");
			break;					
		}		
		puts_xy(12,row,YELLOW, c );

		++p->count;
		sprintf(txt,"%02d", p->count);
		puts_xy(8,row,YELLOW, txt);	
		
		task_endcycle();
	}

	cprintf("Task #%d end.\n", p->task_nr);
	if(p->task_nr==4) call_shutdown_task(0);
	return 0;
};


// This is the main user application entry point.
int main_module_entry(void* arg)
{	
	struct dynalink_module_list* dml = (struct dynalink_module_list*)arg;	
	
	// make char pointer to text data module,..
	char* txt = (char*)(dml->dat[0].start);
	
	// ... and print out it's data content
	cprintf("%s", txt);	

	keyb_getch(BLOCK);
	clear();	

	SOFT_TASK_MODEL msoft;	
	PID p1,p2,p3,p4;

	int yrow = 1;
	puts_xy(0,yrow,  7,"Task#1:[  ]");
	puts_xy(0,yrow+1,7,"Task#2:[  ]");
	puts_xy(0,yrow+2,7,"Task#3:[  ]");
	puts_xy(0,yrow+3,7,"Task#4:[  ]");	

	place(0,7);

	// Init shared soft task model
	soft_task_default_model(msoft);
	soft_task_def_met(msoft,10000);	
	soft_task_def_group(msoft, 1);  
	soft_task_def_periodic(msoft);
	soft_task_def_level(msoft, 2);
	
	int cycles = 4;
	
	// init Task 1 
	float task_periode = 1.0; 	// 1sec	
	int tick = get_tick();		// 1000 usec = 1 ms
	int per = (int)( task_periode *1000.0 * tick);
	struct params pp1;
	pp1.task_nr = 1;
	pp1.row		= yrow;
	pp1.cycles	= cycles;
	soft_task_def_period(msoft, per);			// set period
	soft_task_def_arg(msoft, (void*)(&pp1) ); 	// set arguments
	p1 = task_create("save", periodic_task, &msoft, NULL);
	if (p1 == NIL)
	{ 
	  sys_shutdown_message("Can't create task1 ...\n"); 
	  exit(1);
	}

	// init Task 2
	task_periode = task_periode*0.5; 	// twice as fast as task1
	tick = get_tick();		// 1000 usec = 1 ms
	per = (int)( task_periode *1000.0 * tick);
	struct params pp2;
	pp2.task_nr	= 2;
	pp2.row		= yrow+1;
	pp2.cycles	= cycles*2;
	soft_task_def_period(msoft, per);
	soft_task_def_arg(msoft, (void*)(&pp2) );
	p2 = task_create("skip", periodic_task, &msoft, NULL);
	if (p2 == NIL)
	{ 
	  sys_shutdown_message("Can't create task2...\n"); 
	  exit(1);
	}

	// init Task 3
	task_periode = task_periode*0.5; 	// twice as fast as previous task
	tick = get_tick();		// 1000 usec = 1 ms
	per = (int)( task_periode *1000.0 * tick);
	struct params pp3;
	pp3.task_nr	= 3;
	pp3.row		= yrow+2;
	pp3.cycles	= cycles*4;
	soft_task_def_period(msoft, per);
	soft_task_def_arg(msoft, (void*)(&pp3) );
	p3 = task_create("skip", periodic_task, &msoft, NULL);
	if(p3 == NIL)
	{ 
	  sys_shutdown_message("Can't create task3...\n"); 
	  exit(1);
	}	

	// init Task 4
	task_periode = task_periode*0.5; 	// twice as fast as previous task
	tick = get_tick();		// 1000 usec = 1 ms
	per = (int)( task_periode *1000.0 * tick);
	struct params pp4;
	pp4.task_nr	= 4;
	pp4.row		= yrow+3;
	pp4.cycles	= cycles*8;
	soft_task_def_period(msoft, per);
	soft_task_def_arg(msoft, (void*)(&pp4) );
	p4 = task_create("skip", periodic_task, &msoft, NULL);
	if(p4 == NIL)
	{ 
	  sys_shutdown_message("Can't create task4...\n"); 
	  exit(1);
	}
	
	group_activate(1);

  return 0;
};
