/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   Michael Trimarchi   <trimarchi@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * Copyright (C) 2002 Paolo Gai
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


#ifndef __GRUBSTAR_H__
#define __GRUBSTAR_H__

#include <kernel/kern.h>

#define BUDGET_PCLASS 0x0600				
			
typedef struct {
  TASK_MODEL t;
  int b;
} BUDGET_TASK_MODEL;

#define budget_task_default_model(m,buf)                          \
                        task_default_model((m).t, BUDGET_PCLASS), \
                        (m).b    = (buf);



/* some constants for registering the Module in the right place */
#define GRUBSTAR_LEVELNAME       "GRUBSTAR"
#define GRUBSTAR_LEVEL_CODE       106
#define GRUBSTAR_LEVEL_VERSION    1

LEVEL GRUBSTAR_register_level(int n, LEVEL master);

/* Allocates a budget to be used for an application.
   Input parameters:
     Q The budget
     T The period of the budget
   Return value:
     0..N The ID of the budget
     -1   no more free budgets
     -2   The budgets allocated locally to this module have bandwidth > 1
     -3   wrong LEVEL id 
*/
int GRUBSTAR_setbudget(LEVEL l, TIME Q, TIME T, TIME D, LEVEL local_scheduler_level, int scheduler_id);

int GRUBSTAR_removebudget(LEVEL l, int budget);

int GRUBSTAR_adjust_budget(LEVEL l, TIME Q, TIME T, TIME D, int budget);

int GRUBSTAR_getbudgetinfo(LEVEL l, TIME *Q, TIME *T, TIME *D, int budget);

int GRUBSTAR_was_budget_overran(LEVEL l, int budget);

int GRUBSTAR_is_active(LEVEL l, int budget);

int GRUBSTAR_get_local_scheduler_level_from_budget(LEVEL l, int budget);

int GRUBSTAR_get_local_scheduler_level_from_pid(LEVEL l, PID p);

int GRUBSTAR_get_local_scheduler_id_from_budget(LEVEL l, int budget);

int GRUBSTAR_get_local_scheduler_id_from_pid(LEVEL l, PID p);

int GRUBSTAR_get_last_reclaiming(LEVEL l, PID p);

int GRUBSTAR_get_remain_capacity(LEVEL l, int budget);

bandwidth_t GRUBSTAR_return_bandwidth(LEVEL l);

void GRUBSTAR_disable_server(LEVEL l, int budget);

int GRUBSTAR_get_renegotiation_status(LEVEL l, int budget);

void  GRUBSTAR_getdeadline(LEVEL l, int budget, struct timespec *t);

#endif

