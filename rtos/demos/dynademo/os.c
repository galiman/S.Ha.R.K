//////////////////////////////////////////////////////////////
//  os.c 	-Lex Nahumury 2006
//
// This is the OpenSource SHARK OS/kernel part.
// It will dynamicly link the main application ELF object
// through 'Dynalink'.
//
//////////////////////////////////////////////////////////////
#include "kernel/kern.h"
#include <kernel/func.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ll/i386/cons.h"

#include "edf/edf/edf.h"
#include "cbs/cbs/cbs.h"
#include "hardcbs/hardcbs/hardcbs.h"
#include "rr/rr/rr.h"
#include "dummy/dummy/dummy.h"
#include "intdrive/intdrive/intdrive.h"

#include "sem/sem/sem.h"
#include "hartport/hartport/hartport.h"
#include "cabs/cabs/cabs.h"

#include <drivers/shark_linuxc26.h>
#include <drivers/shark_pci26.h>
#include <drivers/shark_input26.h>
#include <drivers/shark_keyb26.h>

// DYNALINK
#include <dynalink.h>

// some shark pci forward declaration stuff
extern int pci20to26_find_class(unsigned int class_code, int index, BYTE *bus, BYTE *dev);
extern int pci20to26_read_config_byte(unsigned int bus, unsigned int dev, int where, BYTE *val);
extern int pci20to26_read_config_word(unsigned int bus, unsigned int dev, int where, WORD *val);
extern int pci20to26_read_config_dword(unsigned int bus, unsigned int dev, int where, DWORD *val);
extern int pci20to26_write_config_byte(unsigned int bus, unsigned int dev, int where, BYTE val);
extern int pci20to26_write_config_word(unsigned int bus, unsigned int dev, int where, WORD val);
extern int pci20to26_write_config_dword(unsigned int bus, unsigned int dev, int where, DWORD val);


#define TICK 		1000	//  1ms
#define RRTICK 		100000	//  100ms

/*+ IntDrive Server +*/
#define INTDRIVE_Q 1000
#define INTDRIVE_U 0.1*MAX_BANDWIDTH
#define INTDRIVE_FLAG 0

#define NSEC_PER_SEC (1000000000L)
#define SHUTDOWN_TIMEOUT_SEC 	0.5		// seconds

// prototypes
void call_shutdown_task(void *arg);
int device_drivers_init();
int device_drivers_close();
void set_shutdown_task();
TASK shutdown_task_body(void *arg);

// user application function pointer prototype
int (*main_app_entry)(void* arg);


// vars
static struct multiboot_info *mb = 0;
static struct dynalink_module_list	dml;
PID shutdown_task_PID = -1;


// user app calls this to retrieve tick
DWORD get_tick()
{
	return TICK;
};

TASK shutdown_task_body(void *arg)
{
	device_drivers_close();
	sys_shutdown_message("-- OS Closed --\n");
	return NULL;
};

void set_shutdown_task()
{
	NRT_TASK_MODEL nrt;
	nrt_task_default_model(nrt);
	nrt_task_def_system(nrt);

	shutdown_task_PID = task_create("Shutdown Task", shutdown_task_body, &nrt, NULL);
	if (shutdown_task_PID == NIL)
	{
		sys_shutdown_message("Error: Cannot create shutdown task\n");
		exit(1);
	}
};


void call_shutdown_task(void *arg)
{
	struct timespec t;
	sys_gettime(&t);
	t.tv_nsec += NSEC_PER_SEC * SHUTDOWN_TIMEOUT_SEC; 
	kern_event_post(&t,(void *)((void *)sys_abort_shutdown),(void *)0);
	task_activate(shutdown_task_PID);
};


TIME __kernel_register_levels__(void *arg)
{
	mb = (struct multiboot_info *)arg;
	LEVEL EDF_level;

	INTDRIVE_register_level(INTDRIVE_Q, INTDRIVE_Q, INTDRIVE_U, INTDRIVE_FLAG);
  EDF_level = EDF_register_level(EDF_ENABLE_ALL);
	CBS_register_level(CBS_ENABLE_ALL, EDF_level);
	RR_register_level(RRTICK, RR_MAIN_YES, mb);
	dummy_register_level();
	
	SEM_register_module();
	CABS_register_module();	
	
  return TICK;
};

TASK __init__(void *arg)
{
	mb = (struct multiboot_info *)arg;
 
	//HARTPORT_init();
	set_shutdown_task();
	device_drivers_init();	

	sys_atrunlevel(	call_shutdown_task,
					NULL,
					RUNLEVEL_SHUTDOWN);	
	
	__call_main__(mb);

  return (void *)0;
};


int main(int argc, char **argv)
{
	// Any modules passed to kernel by GRUB?
	if(!mb->mods_count)
	{
		printk("No modules passed at all! Bye..\n");
		exit(1);
	}
	
	// Process the modules through 'dynalink'
	dynalink_modules(mb, &dml, "_module_entry");
	if(dml.num_apps == 0)
	{
		printk("No Application modules found! Bye..\n");
		exit(1);
	}

	// Run first found user application function
	// and pass the dynalink_module_list..	
	DWORD dynadr = dml.app[0].dyn_entry;
	if(dynadr)
	{
		main_app_entry = (void*)dynadr;
		main_app_entry(&dml);
	}
	else
	{
		printk("No Application modules found! Bye..\n");
		exit(1);
	}			
return 0;
};


int device_drivers_init()
{	
	LINUXC26_register_module(TRUE);
	//PCI26_init();
	INPUT26_init();
	
	KEYB_PARMS  kparms = BASE_KEYB;	
	keyb_def_ctrlC(kparms, NULL);	
	KEYB26_init(&kparms);
	
	return 0;
};

int device_drivers_close()
{	
	KEYB26_close();
	INPUT26_close();
	return 0;                                                                   
};


