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


#ifndef __FSF_SERVICE_TASK_H__
#define __FSF_SERVICE_TASK_H__


typedef struct {
        fsf_server_id_t  server;
        TIME             actual_period;
        TIME             actual_budget;
        int              Qs;    // quality of service
        int              Is;    // importance of service
        bandwidth_t      U;     // actual bandwidth
        bandwidth_t      Umin;  // min bandwidth
        bandwidth_t      Umax;  // max bandwidth
        TIME             Cmax;
        TIME             Cmin;
        TIME             Tmin;
        TIME             Tmax;
        TIME             deadline;
        bool             d_equals_t;
} server_elem;


extern server_elem server_list[FSF_MAX_N_SERVERS];
extern int current_server;
extern bandwidth_t fsf_max_bw;

TASK service_task(void);

int negotiate_contract
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t                 *server);

int renegotiate_contract
  (const fsf_contract_parameters_t *contract,
   fsf_server_id_t                 server);

int adjust_SERVER_budget
   (int budget, const TIME budget_actual,
    const TIME period_actual, const TIME dline_actual);

int set_SERVER_budget_from_contract
  (const fsf_contract_parameters_t *contract,
   int *budget);

int relink_contract_to_server(const fsf_contract_parameters_t *contract,
                              fsf_server_id_t server);

int link_contract_to_server(const fsf_contract_parameters_t *contract,
                            fsf_server_id_t server);

int add_contract(const fsf_contract_parameters_t *contract);

#endif
