/*
 * Project: HARTIK (HA-rd R-eal TI-me K-ernel)
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *               Gerardo Lamastra <gerardo@sssup.it>
 *
 * Authors     : Paolo Gai <pj@hartik.sssup.it>
 * (see authors.txt for full list of hartik's authors)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it
 */

/**
 ------------
 CVS :        $Id: const.h,v 1.7 2005/01/08 14:50:39 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.7 $
 Last update: $Date: 2005/01/08 14:50:39 $
 ------------

 System constants:

   - Customizable constants

   - Non-Customizable constants

 IMPORTANT!!!
 This file is used also at assembly level,to have unique
 defintion and avoid mess!

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

#ifndef __KERNEL_CONST_H__
#define __KERNEL_CONST_H__

#include <ll/i386/tss-ctx.h>

/* -------------------------------------------------------------------- */
/* These constant can be freely changed to customize the kernel         */
/* -------------------------------------------------------------------- */

/* Tasks, functions, interrupts, exceptions */
#define MAX_PROC          TSSMax-1      /*+ Maximum number of tasks
                                            Change the value in
                                            ll/i386/tss-ctx.h!!!
                                            -1 because there is the
                                            global context!!!            +*/
#define MAX_RUNLEVEL_FUNC 40            /*+ Runlevel functions +*/
#define STACK_SIZE      4096U           /*+ Stack size in bytes          +*/
#define JET_TABLE_DIM     20U           /*+ Max n. of Jet entries        +*/
#define MAX_CANCPOINTS    20U           /*+ maximum number of cancellation
                                            points (look at kill.c) +*/
#define MAX_SIGINTPOINTS  20U           /*+ maximum number of signal
                                            interruptable points         +*/

/* Levels and modules */
#define MAX_SCHED_LEVEL   30U           /*+ Max Schedule levels          +*/
#define MAX_RES_LEVEL     8U            /*+ Max Resource levels          +*/
#define MAX_LEVELNAME     20U           /*+ Level Name Max Lenght        +*/
#define MAX_MODULENAME    20U           /*+ Resource Module Name Max L.  +*/
#define MAX_TASKNAME      20U           /*+ Task Name Max Length         +*/

/* Resource and communication */
#define MAX_CPRINT_BUF 512U           /*+ Length of cprintf() buffer   +*/


/* -------------------------------------------------------------------- */
/* End of user customazable constants                                   */
/* -------------------------------------------------------------------- */


/*+ this is the bandwidth used in the on-line guarantee algorithm
    the value means x/MAX_BANDWIDTH free bandwidth... +*/
#define MAX_BANDWIDTH MAX_DWORD



/*+ STATUS attributes are divided in 2 sets:
    - kernel status set (FREE, EXE, SLEEP, IDLE)
    - scheduling modules status set (the remainings status! READY...)
      the values are in the range [MODULE_STATUS_BASE, APER_STATUS_BASE-1]
    - aperiodic servers status set
      the minimum value is APER_STATUS_BASE
      The statuses must be separated because an aperiodic server may,
      for example put an event that checks a task state;
+*/

#define FREE            0
#define EXE             1
#define SLEEP           2
#define WAIT_JOIN       3
#define WAIT_COND       4
#define WAIT_SIG        5
#define WAIT_SEM        6
#define WAIT_NANOSLEEP  7
#define WAIT_SIGSUSPEND 8
#define WAIT_MQSEND     9
#define WAIT_MQRECEIVE 10

/*+ base status for scheduling levels +*/
#define MODULE_STATUS_BASE  128

/*+ base status for aperiodic servers +*/
#define APER_STATUS_BASE   1024

/*+ base status for libraries +*/
#define LIB_STATUS_BASE    2048

/*+ return value for killed tasks; it is equal to PTHREAD_CANCELED!!! +*/
#define TASK_CANCELED  (void *)1

/*+ Set the cancellation state of the task +*/
#define TASK_CANCEL_ENABLE  KILL_ENABLED
#define TASK_CANCEL_DISABLE 0

/*+ Set the cancellation type of the task +*/
#define TASK_CANCEL_DEFERRED     KILL_DEFERRED
#define TASK_CANCEL_ASYNCHRONOUS 0

/* C stuff */


#ifndef NIL
#define NIL     -1                      /*+ integer unvalid value   +*/
#endif

/*+ Blocking attribute for semaphore, port & keyboard operations +*/
#define BLOCK           1
#define NON_BLOCK       0

#define DEFAULT (DWORD)-1

/*+ sys_atrunlevel status bit values: used to signal that the function
    has to be called +*/
#define RUNLEVEL_STARTUP     0 /*+ startup in real mode                  +*/
#define RUNLEVEL_INIT        1 /*+ init time                             +*/
#define RUNLEVEL_RUNNING     2 /*+ system is running!!!                  +*/
#define RUNLEVEL_SHUTDOWN    3 /*+ shutting down the system              +*/
#define RUNLEVEL_BEFORE_EXIT 4 /*+ before the kernel has been terminated +*/
#define RUNLEVEL_AFTER_EXIT  5 /*+ after the kernel has been terminated  +*/
#define RUNLEVEL_MASK        0x7 /*+ a mask used into kernel/init.c      +*/
#define NO_AT_ABORT          8 /*+ only when _exit is called           +*/

#endif /* __CONST_H__ */

