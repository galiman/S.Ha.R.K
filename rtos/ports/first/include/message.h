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


#ifndef __MESSAGE_H__
#define __MESSAGE_H__

typedef enum {
 NO_TYPE=-1,
 NEGOTIATE_CONTRACT,
 RENEGOTIATE_CONTRACT,
 REQUEST_RENEGOTIATE_CONTRACT,
 CHANGE_PARAMETER,
 WAIT_NEGOTIATE,
 NEGOTIATE_GROUP,
 NUM_MESSAGE,
 } message_type;

typedef struct qual_imp {
  int importance;
  int quality;
} qual_imp;


typedef struct mess {
  message_type    type;
  PID             process;
  int             sig_notify;
  union sigval    sig_value;
  fsf_server_id_t server;
  union {
    fsf_contract_parameters_t contract;
    qual_imp qi;
  };
} mess;


#endif
