/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: init.c,v 1.5 2005/01/08 14:48:24 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.5 $
 Last update: $Date: 2005/01/08 14:48:24 $
 ------------

 - Kernel module registration functions
 - miscellaneous functions related to module registration, system init and end

**/

/*
 * Copyright (C) 2000 Paolo Gai
 *
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

#include <stdarg.h>
#include <ll/ll.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <ll/string.h>
#include <kernel/config.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>

/***********************************************************************
 * Runlevel management
 ***********************************************************************/

/*+ List of function to call at each rnlevel;
    they are posted with sys_atrunlevel +*/
static struct exit_func {
    void (*f)(void *);
    void *arg;
    int no_at_abort;
    int next;
} runlevel_list[MAX_RUNLEVEL_FUNC];

static int runlevel_init_list;
static int runlevel_init_tail;
static int runlevel_shutdown_list;
static int runlevel_before_list;
static int runlevel_after_list;
static int runlevel_free;


void runlevel_init()
{
  int i;

  for (i = 0; i < MAX_RUNLEVEL_FUNC-1; i++)
    runlevel_list[i].next = i+1;
  runlevel_list[MAX_RUNLEVEL_FUNC-1].next = -1;

  runlevel_init_list     = -1;
  runlevel_init_tail     = -1;
  runlevel_shutdown_list = -1;
  runlevel_before_list   = -1;
  runlevel_after_list    = -1;
  runlevel_free          = 0;
}

/*+ flags may be RUNLEVEL_XXX...
    aborting may be 0 or NO_AT_ABORT +*/
void call_runlevel_func(int runlevel, int aborting)
{
  int i, j;

  switch (runlevel) {
    case RUNLEVEL_INIT:
      i = runlevel_init_list;
      runlevel_init_list = -1;
      break;
    case RUNLEVEL_SHUTDOWN:
      i = runlevel_shutdown_list;
      runlevel_shutdown_list = -1;
      break;
    case RUNLEVEL_BEFORE_EXIT:
      i = runlevel_before_list;
      runlevel_before_list = -1;
      break;
    case RUNLEVEL_AFTER_EXIT:
      i = runlevel_after_list;
      runlevel_after_list = -1;
      break;
    default:
      return;
  }

  // the task_activate must differ!!! look at activate.c
  calling_runlevel_func = 1;

  while (i != -1) {
    if (!(aborting && runlevel_list[i].no_at_abort))
      runlevel_list[i].f(runlevel_list[i].arg);

    j = i;
    i = runlevel_list[i].next;

    runlevel_list[j].next = runlevel_free;
    runlevel_free = j;
  }

  calling_runlevel_func = 0;

}

/*+ Use this function to post your own runlevel operations
    (when uses some defines contained in const.h) +*/
int sys_atrunlevel(void (*func_code)(void *),void *parm, BYTE when)
{
    register int i = 0;
    SYS_FLAGS f;

    f = kern_fsave();
    if (runlevel_free == -1) {
	errno = ETOOMUCH_EXITFUNC;
	kern_frestore(f);
	return -1;
    }

    i = runlevel_free;
    runlevel_free = runlevel_list[runlevel_free].next;

    runlevel_list[i].f = func_code;
    runlevel_list[i].arg = parm;
    runlevel_list[i].no_at_abort = when & NO_AT_ABORT;

    switch (when & RUNLEVEL_MASK) {
      case RUNLEVEL_INIT:
        /* the init functions are called in the order they are posted
           so, we insert at the queue tail */
        runlevel_list[i].next = -1;
        if (runlevel_init_list == -1)
          runlevel_init_list = i;
        else
          runlevel_list[runlevel_init_tail].next = i;
        runlevel_init_tail = i;
        break;
      case RUNLEVEL_SHUTDOWN:
        runlevel_list[i].next = runlevel_shutdown_list;
        runlevel_shutdown_list = i;
        break;
      case RUNLEVEL_BEFORE_EXIT:
        runlevel_list[i].next = runlevel_before_list;
        runlevel_before_list = i;
        break;
      default: // RUNLEVEL_AFTER_EXIT
        runlevel_list[i].next = runlevel_after_list;
        runlevel_after_list = i;
    }

    kern_frestore(f);
    return 0;
}

/***********************************************************************
 * Level Default Descriptor
 ***********************************************************************/

static void level_excfunc(LEVEL l)
{ 
  printk(KERN_EMERG "unreg scheduling function called, level=%d!\n", l); 
  kern_raise(XINVALID_TASK, exec_shadow);
}

static int level_return1(void) { return 1; }
static int level_returnminus1(void) { return -1; }
static void level_nothing(void) { }
static int level_return0(void) { return 0; }

static level_des level_default_descriptor =
{
  (void (*)(LEVEL,PID,TASK_MODEL *))level_excfunc, /* private_insert   */
  (void (*)(LEVEL,PID))             level_excfunc, /* private_extract  */
  (int  (*)(LEVEL,PID))             level_return0, /* private_eligible */
  (void (*)(LEVEL,PID, int))        level_excfunc, /* private_dispatch */
  (void (*)(LEVEL,PID))             level_excfunc, /* private_epilogue */
  
  (PID  (*)(LEVEL))                 level_returnminus1, /* pubvlic_scheduler */
  (int  (*)(LEVEL,bandwidth_t *))   level_return1, /* public_guarantee */
  (int  (*)(LEVEL,PID,TASK_MODEL *))level_returnminus1, /* public_create */
  (void (*)(LEVEL,PID))             level_nothing, /* public_detach */
  (void (*)(LEVEL,PID))             level_excfunc, /* public_end       */
  (int  (*)(LEVEL,PID))             level_return0, /* public_eligible */
  (void (*)(LEVEL,PID, int))        level_excfunc, /* public_dispatch  */
  (void (*)(LEVEL,PID))             level_excfunc, /* public_epilogue  */
  (void (*)(LEVEL,PID,struct timespec *))level_excfunc, /* public_activate  */
  (void (*)(LEVEL,PID))             level_excfunc, /* public_unblock   */
  (void (*)(LEVEL,PID))             level_excfunc, /* public_block     */
  (int  (*)(LEVEL,PID,void *))      level_excfunc, /* public_message   */
};


/***********************************************************************
 * Module registration
 ***********************************************************************/

/* this function initializes all the data structures used by the level
   registration functions */
void levels_init(void)
{
  int l;
  for (l=0; l<MAX_SCHED_LEVEL; l++) {
    level_table[l] = &level_default_descriptor;
    level_used[l] = 0;
    level_next[l] = l+1;
    level_prev[l] = l-1;
  }

  level_next[MAX_SCHED_LEVEL-1l] = -1;
  level_prev[0] = -1;
  
  level_first = -1;
  level_last = -1;
  level_free = 0;
}

/*+ This function returns a level_des **. the value returned shall be
    used to register a level module. 

    The function is usually called at module registration time.  The
    function can also be called when the system is already started, to
    allow the implementation of dynamic module registration.  

    The argument must be the size of the data block that have to be allocated

    The function returns the number of the descriptor allocated for the module
    or -1 in case there are no free descriptors.

    The function also reserves a descriptor with size s, initialized
    with default function pointers.

+*/
LEVEL level_alloc_descriptor(size_t s)
{
  LEVEL l;
  /* try to find a free descriptor */
  if (level_free == -1)
    return -1;

  /* alloc it */
  l = level_free;
  level_free = level_next[l];

  level_used[l] = 1;

  /* insert the module as the last in the scheduling module's list */
  if (level_last == -1) {
    level_first = l;
    level_prev[l] = -1;
  }
  else {
    level_next[level_last] = l;
    level_prev[l] = level_last;
  }
  level_last = l;
  level_next[l] = -1;

  /* allocate the descriptor! */
  if (s < sizeof(level_des)) 
    s = sizeof(level_des);

  level_table[l] = (level_des *)kern_alloc(s);

  *(level_table[l]) = level_default_descriptor;

  level_size[l] = s;

  /* return the descriptor index */
  return l;
}


/*+ This function release a level descriptor previously allocated using 
  level_alloc_descriptor(). 

  The function returns 0 if the level has been freed, or -1 if someone is
  using it, -2 if the level has never been registered.

+*/
int level_free_descriptor(LEVEL l)
{
  if (level_used[l] == 0) 
    return -2;
  else if (level_used[l] > 1)
    return -1;

  /* we can free the descriptor */
  level_used[l] = 0;

  /* remove it from the "first" queue */
  if (level_prev[l] == -1)
    level_first = level_next[l];
  else
    level_next[level_prev[l]] = level_next[l];
 
  if (level_next[l] == -1)
    level_last = level_prev[l];
  else
    level_prev[level_next[l]] = level_prev[l];
      
  /* ... and put it in the free queue */
  level_prev[level_free] = l;
  level_next[l] = level_free;
  level_free = l;

  /* finally, free the memory allocated to it */
  kern_free(level_table[l], level_size[l]);

  return 0;
}

/* Call this if you want to say that your module is using module l 
   (e.g., for calling its private functions) */
int level_use_descriptor(LEVEL l)
{
  return ++level_used[l];
}

/* Call this when you no more need the module l */
int level_unuse_descriptor(LEVEL l)
{
  return --level_used[l];
}


/*+ This function returns a resource_des **. the value returned shall be
    used to register a resource module. The function shall be called only at
    module registration time. +*/
RLEVEL resource_alloc_descriptor()
{
  if (res_levels == MAX_RES_LEVEL)
  {
    printk("Too many resource levels!!!\n");
    exit(1);
  }

  return res_levels++;
}


/***********************************************************************
 * Parameter parsing (argc, argv)
 ***********************************************************************/

/*+ This function compute the command line parameters from the multiboot_info
    NOTE: this function modify the multiboot struct, so this function and
    __call_main__ are mutually exclusives!!! +*/
void __compute_args__(struct multiboot_info *mbi, int *_argc, char **_argv)
{
  register int i = 0;
  char *cmdline = (char *)(mbi->cmdline);

  /* creates the command line... */
  *_argc = 0;
  if (mbi->flags & MB_INFO_CMDLINE) {
    while (cmdline[i] != 0) {
      _argv[*_argc] = &(cmdline[i]);
      while (cmdline[i] != ' ' && cmdline[i] != 0) i++;
      if (cmdline[i] == ' ') {
  	cmdline[i] = 0; i++; (*_argc)++;
      }
    }
    (*_argc)++;
  }
}

/* note that the prototype is not public... so the user can
    also use a int main(void), void main(void)... and so on... */
int main(int argc, char **argv);

/*+ This function calls the standard C main() function, with a
    parameter list up to 100 parameters                        +*/
int __call_main__(struct multiboot_info *mbi)
{
  int _argc;
  char *_argv[100];
  __compute_args__(mbi, &_argc, _argv);
  return main(_argc,_argv);
}



