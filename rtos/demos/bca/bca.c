/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/* CVS :        $Id: bca.c,v 1.10 2006/07/03 15:30:11 tullio Exp $ */

/*
 * Copyright (C) 2000 Fabio Calabrese <fabiocalabrese77@yahoo.it>
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

/****************************************************
 *                                                  *
 *  file:        bca.c                              *
 *  header file: bca.h                              *
 *  data:        15/09/2002                         *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ******************************************************
 *                                                    *
 *  descrizione: e' il file in cui è descritto il task*
 *               main del programma S.Ha.R.K.         *
 *               "(B)ase(C)ontr(A)rea".               *
 *                                                    *
 ******************************************************/


// *** Librerie S.Ha.R.K ***
#include <kernel/kern.h>
#include <ll/i386/cons.h>

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>

// *** Standard C library ***
#include <stdlib.h>

// *** FAB library ***
#include "fab_lib/fab_msg.h"
#include "fab_lib/fab_show.h"
#include "fab_lib/fab_tool.h"
#include "fab_lib/fab_grx.h"

// *** BCA library ***
#include "bca.h"

void end_fun(KEY_EVT* k) { 
	sys_shutdown_message("Ending...\n");
	exit(0);
}

void inc_planes(KEY_EVT* k) {
	if (aereo_count < AEREO_N_MAX) aereo_count++;
}

void dec_planes(KEY_EVT* k) {
	if (aereo_count > AEREO_N_MIN) aereo_count--;
}

void inc_cannon(KEY_EVT* k) {
	if (cannone_count < CANNONE_N_MAX) cannone_count++;
}

void dec_cannon(KEY_EVT* k) {
	if (cannone_count > CANNONE_N_MIN) cannone_count--;
}

int main(int argc, char **argv) {
	PID pid_dummy_radar, pid_aereo_creator, pid_cannone_creator, pid_control;

	KEY_EVT k;

	k.flag = CNTL_BIT;
	k.scan = KEY_C;
	k.ascii = 'c';
	k.status = KEY_PRESSED;
	keyb_hook(k, end_fun, FALSE);

	k.flag = CNTR_BIT;
	k.status = KEY_PRESSED;
	keyb_hook(k, end_fun, FALSE);
	
	k.flag  = 0;
	k.scan  = KEY_1;
	k.ascii = '1';
	k.status = KEY_PRESSED;
	keyb_hook(k, inc_planes, FALSE);
	
	k.flag  = 0;
	k.scan  = KEY_2;
	k.ascii = '2';
	k.status = KEY_PRESSED;
	keyb_hook(k, dec_planes, FALSE);
	
	k.flag  = 0;
	k.scan  = KEY_3;
	k.ascii = '3';
	k.status = KEY_PRESSED;
	keyb_hook(k, inc_cannon, FALSE);
	
	k.flag  = 0;
	k.scan  = KEY_4;
	k.ascii = '4';
	k.status = KEY_PRESSED;
	keyb_hook(k, dec_cannon, FALSE);

	/* inizializza il mutex grafico */
	app_mutex_init(&grx_mutex);

	scenario();

	//---init---
	srand(sys_gettime(NULL));
	aereo_count   = AEREO_N_MIN;
	cannone_count = CANNONE_N_MIN;

	pid_dummy_radar = crea_soft_dummy_radar();
	if (pid_dummy_radar == -1) {
		sys_shutdown_message("bca.c(main): could not create task <dummy_radar>");
		exit(1);
	}
	task_activate(pid_dummy_radar);

	pid_aereo_creator = crea_soft_aereo_creator();
	if (pid_aereo_creator == -1) {
		sys_shutdown_message("bca.c(main): could not create task <aereo_creator>");
		exit(1);
	}
	task_activate(pid_aereo_creator);

	pid_cannone_creator = crea_soft_cannone_creator();
	if (pid_cannone_creator == -1) {
		sys_shutdown_message("bca.c(main): could not create task <cannone_creator>");
		exit(1);
	}
	task_activate(pid_cannone_creator);

	pid_control = crea_soft_control();
	if (pid_control == -1) {
		sys_shutdown_message("bca.c(main): could not create task <control>");
		exit(1);
	}
	task_activate(pid_control);
	//----------

	return 0;
}


